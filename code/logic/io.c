/**
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop
 * high-performance, cross-platform applications and libraries. The code
 * contained herein is licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Author: Michael Gene Brockus (Dreamer)
 * Date: 04/05/2014
 *
 * Copyright (C) 2014-2025 Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#include "fossil/image/io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ======================================================
// BMP structures
// ======================================================
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} bmp_header_t;

typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} bmp_info_t;

// ======================================================
// BMP Load/Save (supports 24-bit RGB & 32-bit RGBA)
// ======================================================
static bool load_bmp(const char *filename, fossil_image_t *out_image) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    bmp_header_t hdr;
    fread(&hdr, sizeof(hdr), 1, f);
    if (hdr.bfType != 0x4D42) { fclose(f); return false; }

    bmp_info_t info;
    fread(&info, sizeof(info), 1, f);
    if ((info.biBitCount != 24 && info.biBitCount != 32) || info.biCompression != 0) { fclose(f); return false; }

    out_image->width = info.biWidth;
    out_image->height = info.biHeight;
    out_image->channels = (info.biBitCount == 24) ? 3 : 4;
    out_image->format = (out_image->channels == 3) ? FOSSIL_PIXEL_FORMAT_RGB24 : FOSSIL_PIXEL_FORMAT_RGBA32;
    size_t size = (size_t)out_image->width * out_image->height * out_image->channels;
    out_image->data = (uint8_t *)malloc(size);
    out_image->size = size;
    out_image->owns_data = true;

    fseek(f, hdr.bfOffBits, SEEK_SET);

    for (int y = info.biHeight - 1; y >= 0; --y) {
        fread(out_image->data + y * out_image->width * out_image->channels, out_image->channels, out_image->width, f);
        fseek(f, (4 - (out_image->width * out_image->channels) % 4) % 4, SEEK_CUR);
    }

    fclose(f);
    return true;
}

static bool save_bmp(const char *filename, const fossil_image_t *image) {
    if (!image || !image->data) return false;

    FILE *f = fopen(filename, "wb");
    if (!f) return false;

    bmp_header_t hdr = {0};
    hdr.bfType = 0x4D42;
    hdr.bfOffBits = sizeof(bmp_header_t) + sizeof(bmp_info_t);
    hdr.bfSize = hdr.bfOffBits + (uint32_t)(image->width * image->height * image->channels);

    bmp_info_t info = {0};
    info.biSize = sizeof(bmp_info_t);
    info.biWidth = image->width;
    info.biHeight = image->height;
    info.biPlanes = 1;
    info.biBitCount = (uint16_t)(image->channels * 8);
    info.biCompression = 0;
    info.biSizeImage = image->width * image->height * image->channels;

    fwrite(&hdr, sizeof(hdr), 1, f);
    fwrite(&info, sizeof(info), 1, f);

    for (int y = image->height - 1; y >= 0; --y) {
        fwrite(image->data + y * image->width * image->channels, image->channels, image->width, f);
        uint8_t pad[3] = {0};
        fwrite(pad, 1, (4 - (image->width * image->channels) % 4) % 4, f);
    }

    fclose(f);
    return true;
}

// ======================================================
// PPM Load/Save (P6 RGB & P7 RGBA extension)
// ======================================================
static bool load_ppm(const char *filename, fossil_image_t *out_image) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    char header[3];
    fscanf(f, "%2s", header);

    int w, h, maxv;
    if (strcmp(header, "P6") == 0) { // RGB
        fscanf(f, "%d %d %d%*c", &w, &h, &maxv);
        out_image->width = (uint32_t)w;
        out_image->height = (uint32_t)h;
        out_image->channels = 3;
        out_image->format = FOSSIL_PIXEL_FORMAT_RGB24;
        out_image->size = w * h * 3;
        out_image->data = (uint8_t *)malloc(out_image->size);
        out_image->owns_data = true;
        fread(out_image->data, 1, out_image->size, f);
    } else { fclose(f); return false; }

    fclose(f);
    return true;
}

static bool save_ppm(const char *filename, const fossil_image_t *image) {
    if (!image || !image->data) return false;

    FILE *f = fopen(filename, "wb");
    if (!f) return false;

    if (image->channels == 3) {
        fprintf(f, "P6\n%d %d\n255\n", image->width, image->height);
        fwrite(image->data, 1, image->width * image->height * 3, f);
    } else if (image->channels == 4) {
        // Use P6 with alpha stripped, or define P7 extension
        fprintf(f, "P6\n%d %d\n255\n", image->width, image->height);
        for (size_t i = 0; i < image->width * image->height; ++i) {
            fwrite(&image->data[i*4], 1, 3, f);
        }
    }

    fclose(f);
    return true;
}

// ======================================================
// RAW Load/Save with header (width, height, channels)
// ======================================================
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t channels;
} raw_header_t;

static bool load_raw(const char *filename, fossil_image_t *out_image) {
    if (!filename) return false;
    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    raw_header_t hdr;
    fread(&hdr, sizeof(hdr), 1, f);

    out_image->width = hdr.width;
    out_image->height = hdr.height;
    out_image->channels = hdr.channels;
    out_image->format = (hdr.channels == 1) ? FOSSIL_PIXEL_FORMAT_GRAY8 :
                        (hdr.channels == 3) ? FOSSIL_PIXEL_FORMAT_RGB24 :
                        FOSSIL_PIXEL_FORMAT_RGBA32;
    out_image->size = (size_t)hdr.width * hdr.height * hdr.channels;
    out_image->data = (uint8_t *)malloc(out_image->size);
    out_image->owns_data = true;

    fread(out_image->data, 1, out_image->size, f);
    fclose(f);
    return true;
}

static bool save_raw(const char *filename, const fossil_image_t *image) {
    if (!filename || !image || !image->data) return false;
    FILE *f = fopen(filename, "wb");
    if (!f) return false;

    raw_header_t hdr = {image->width, image->height, image->channels};
    fwrite(&hdr, sizeof(hdr), 1, f);
    fwrite(image->data, 1, image->size, f);

    fclose(f);
    return true;
}

// ======================================================
// Generic Load/Save
// ======================================================
bool fossil_image_io_load(const char *filename, const char *format_id, fossil_image_t *out_image) {
    if (!format_id || !out_image) return false;

    if (strcmp(format_id, "bmp") == 0) return load_bmp(filename, out_image);
    if (strcmp(format_id, "ppm") == 0) return load_ppm(filename, out_image);
    if (strcmp(format_id, "raw") == 0) return load_raw(filename);

    return false;
}

bool fossil_image_io_save(const char *filename, const char *format_id, const fossil_image_t *image) {
    if (!format_id || !image) return false;

    if (strcmp(format_id, "bmp") == 0) return save_bmp(filename, image);
    if (strcmp(format_id, "ppm") == 0) return save_ppm(filename, image);
    if (strcmp(format_id, "raw") == 0) return save_raw(filename, image);

    return false;
}

// ======================================================
// Image Generators
// ======================================================
bool fossil_image_io_generate(fossil_image_t *out_image, const char *type_id, uint32_t width, uint32_t height, uint32_t channels, const float *params) {
    if (!out_image || width == 0 || height == 0 || channels == 0 || channels > 4)
        return false;

    out_image->width = width;
    out_image->height = height;
    out_image->channels = channels;
    out_image->format = (channels == 1) ? FOSSIL_PIXEL_FORMAT_GRAY8 :
                        (channels == 3) ? FOSSIL_PIXEL_FORMAT_RGB24 :
                        FOSSIL_PIXEL_FORMAT_RGBA32;
    out_image->size = width * height * channels;
    out_image->data = (uint8_t *)malloc(out_image->size);
    out_image->owns_data = true;
    if (!out_image->data) return false;

    if (strcmp(type_id, "solid") == 0) {
        uint8_t c[4] = {0};
        if (params) for (uint32_t i = 0; i < channels; ++i)
            c[i] = (uint8_t)fminf(fmaxf(params[i], 0.0f), 255.0f);
        for (size_t i = 0; i < out_image->size; i += channels)
            for (uint32_t ch = 0; ch < channels; ++ch)
                out_image->data[i + ch] = c[ch];
        return true;
    }

    if (strcmp(type_id, "gradient") == 0) {
        uint8_t start[4] = {0}, end[4] = {255,255,255,255};
        if (params) {
            for (uint32_t i = 0; i < channels; ++i) start[i] = (uint8_t)params[i];
            for (uint32_t i = 0; i < channels; ++i) end[i] = (uint8_t)params[channels + i];
        }
        for (uint32_t y = 0; y < height; ++y) {
            float t = (float)y / (float)(height-1);
            for (uint32_t x = 0; x < width; ++x)
                for (uint32_t ch = 0; ch < channels; ++ch)
                    out_image->data[(y*width + x)*channels + ch] = (uint8_t)((1.0f-t)*start[ch] + t*end[ch]);
        }
        return true;
    }

    if (strcmp(type_id, "checker") == 0) {
        uint32_t tile = 8;
        if (params) tile = (uint32_t)params[0];
        uint8_t c1[4] = {0}, c2[4] = {255,255,255,255};
        if (params && params[1]) for (uint32_t i = 0; i < channels; ++i) c1[i] = (uint8_t)params[i+1];
        if (params && params[1+channels]) for (uint32_t i = 0; i < channels; ++i) c2[i] = (uint8_t)params[i+1+channels];
        for (uint32_t y = 0; y < height; ++y)
            for (uint32_t x = 0; x < width; ++x) {
                bool is_c1 = ((x/tile + y/tile) % 2 == 0);
                uint8_t *col = is_c1 ? c1 : c2;
                for (uint32_t ch = 0; ch < channels; ++ch)
                    out_image->data[(y*width + x)*channels + ch] = col[ch];
            }
        return true;
    }

    if (strcmp(type_id, "noise") == 0) {
        for (size_t i = 0; i < out_image->size; ++i)
            out_image->data[i] = (uint8_t)(rand() % 256);
        return true;
    }

    return false;
}
