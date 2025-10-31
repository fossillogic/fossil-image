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
    // Set format according to new enum
    if (info.biBitCount == 24)
        out_image->format = FOSSIL_PIXEL_FORMAT_RGB24;
    else if (info.biBitCount == 32)
        out_image->format = FOSSIL_PIXEL_FORMAT_RGBA32;
    else
        out_image->format = FOSSIL_PIXEL_FORMAT_NONE;

    size_t size = (size_t)out_image->width * out_image->height * out_image->channels;
    out_image->data = (uint8_t *)malloc(size);
    out_image->size = size;
    out_image->owns_data = true;

    fseek(f, hdr.bfOffBits, SEEK_SET);

    for (int y = info.biHeight - 1; y >= 0; --y) {
        fread(out_image->data + y * out_image->width * out_image->channels, out_image->channels, out_image->width, f);
        fseek(f, (4 - (out_image->width * out_image->channels) % 4) % 4, SEEK_CUR);
    }

    // Set extended metadata to defaults
    out_image->name[0] = '\0';
    out_image->author[0] = '\0';
    out_image->dpi_x = 0.0;
    out_image->dpi_y = 0.0;
    out_image->exposure = 0.0;
    out_image->channels_mask = 0;
    out_image->userdata = NULL;
    out_image->is_ai_generated = false;
    out_image->creation_os[0] = '\0';
    out_image->software[0] = '\0';
    out_image->creation_date[0] = '\0';

    fclose(f);
    return true;
}

static bool save_bmp(const char *filename, const fossil_image_t *image) {
    if (!image || !image->data) return false;

    // Only support 8-bit RGB (24) and RGBA (32) for BMP
    if (image->format != FOSSIL_PIXEL_FORMAT_RGB24 && image->format != FOSSIL_PIXEL_FORMAT_RGBA32)
        return false;

    FILE *f = fopen(filename, "wb");
    if (!f) return false;

    uint16_t bit_count = (image->format == FOSSIL_PIXEL_FORMAT_RGB24) ? 24 : 32;
    uint32_t channels = (image->format == FOSSIL_PIXEL_FORMAT_RGB24) ? 3 : 4;

    bmp_header_t hdr = {0};
    hdr.bfType = 0x4D42;
    hdr.bfOffBits = sizeof(bmp_header_t) + sizeof(bmp_info_t);

    // Calculate row size with padding
    uint32_t row_bytes = image->width * channels;
    uint32_t row_padded = (row_bytes + 3) & ~3;
    hdr.bfSize = hdr.bfOffBits + row_padded * image->height;

    bmp_info_t info = {0};
    info.biSize = sizeof(bmp_info_t);
    info.biWidth = image->width;
    info.biHeight = image->height;
    info.biPlanes = 1;
    info.biBitCount = bit_count;
    info.biCompression = 0;
    info.biSizeImage = row_padded * image->height;

    fwrite(&hdr, sizeof(hdr), 1, f);
    fwrite(&info, sizeof(info), 1, f);

    uint8_t pad[3] = {0};
    for (int y = image->height - 1; y >= 0; --y) {
        fwrite(image->data + y * image->width * channels, channels, image->width, f);
        fwrite(pad, 1, row_padded - row_bytes, f);
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
    if (strcmp(header, "P6") == 0) { // 8-bit RGB
        fscanf(f, "%d %d %d%*c", &w, &h, &maxv);
        out_image->width = (uint32_t)w;
        out_image->height = (uint32_t)h;
        out_image->channels = 3;
        out_image->size = w * h * 3;
        out_image->owns_data = true;

        if (maxv == 255) {
            out_image->format = FOSSIL_PIXEL_FORMAT_RGB24;
            out_image->data = (uint8_t *)malloc(out_image->size);
            fread(out_image->data, 1, out_image->size, f);
        } else if (maxv == 65535) {
            out_image->format = FOSSIL_PIXEL_FORMAT_RGB48;
            out_image->data = (uint8_t *)malloc(out_image->size * 2);
            fread(out_image->data, 2, out_image->size, f);
        } else {
            fclose(f);
            return false;
        }
    } else if (strcmp(header, "P5") == 0) { // 8/16-bit grayscale
        fscanf(f, "%d %d %d%*c", &w, &h, &maxv);
        out_image->width = (uint32_t)w;
        out_image->height = (uint32_t)h;
        out_image->channels = 1;
        out_image->size = w * h;
        out_image->owns_data = true;

        if (maxv == 255) {
            out_image->format = FOSSIL_PIXEL_FORMAT_GRAY8;
            out_image->data = (uint8_t *)malloc(out_image->size);
            fread(out_image->data, 1, out_image->size, f);
        } else if (maxv == 65535) {
            out_image->format = FOSSIL_PIXEL_FORMAT_GRAY16;
            out_image->data = (uint8_t *)malloc(out_image->size * 2);
            fread(out_image->data, 2, out_image->size, f);
        } else {
            fclose(f);
            return false;
        }
    } else {
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}

static bool save_ppm(const char *filename, const fossil_image_t *image) {
    if (!image || !image->data) return false;

    FILE *f = fopen(filename, "wb");
    if (!f) return false;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_RGB24:
            fprintf(f, "P6\n%d %d\n255\n", image->width, image->height);
            fwrite(image->data, 1, image->width * image->height * 3, f);
            break;
        case FOSSIL_PIXEL_FORMAT_RGB48:
            fprintf(f, "P6\n%d %d\n65535\n", image->width, image->height);
            fwrite(image->data, 2, image->width * image->height * 3, f);
            break;
        case FOSSIL_PIXEL_FORMAT_RGBA32:
            // Save as P6, strip alpha
            fprintf(f, "P6\n%d %d\n255\n", image->width, image->height);
            for (size_t i = 0; i < image->width * image->height; ++i)
                fwrite(&image->data[i * 4], 1, 3, f);
            break;
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            // Save as P6, strip alpha, 16-bit
            fprintf(f, "P6\n%d %d\n65535\n", image->width, image->height);
            for (size_t i = 0; i < image->width * image->height; ++i)
                fwrite(&image->data[i * 8], 2, 3, f); // 2 bytes per channel
            break;
        case FOSSIL_PIXEL_FORMAT_GRAY8:
            fprintf(f, "P5\n%d %d\n255\n", image->width, image->height);
            fwrite(image->data, 1, image->width * image->height, f);
            break;
        case FOSSIL_PIXEL_FORMAT_GRAY16:
            fprintf(f, "P5\n%d %d\n65535\n", image->width, image->height);
            fwrite(image->data, 2, image->width * image->height, f);
            break;
        default:
            fclose(f);
            return false;
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
    if (!filename || !out_image) return false;
    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    raw_header_t hdr;
    fread(&hdr, sizeof(hdr), 1, f);

    out_image->width = hdr.width;
    out_image->height = hdr.height;
    out_image->channels = hdr.channels;

    // Determine format based on channels and bytes per pixel
    // For this example, assume 8-bit per channel unless file extension or extra header info is added
    switch (hdr.channels) {
        case 1:
            out_image->format = FOSSIL_PIXEL_FORMAT_GRAY8;
            break;
        case 3:
            out_image->format = FOSSIL_PIXEL_FORMAT_RGB24;
            break;
        case 4:
            out_image->format = FOSSIL_PIXEL_FORMAT_RGBA32;
            break;
        default:
            out_image->format = FOSSIL_PIXEL_FORMAT_NONE;
            fclose(f);
            return false;
    }

    out_image->size = (size_t)hdr.width * hdr.height * hdr.channels;
    out_image->data = (uint8_t *)malloc(out_image->size);
    out_image->owns_data = true;

    if (!out_image->data) {
        fclose(f);
        return false;
    }

    fread(out_image->data, 1, out_image->size, f);

    // Set extended metadata to defaults
    out_image->name[0] = '\0';
    out_image->author[0] = '\0';
    out_image->dpi_x = 0.0;
    out_image->dpi_y = 0.0;
    out_image->exposure = 0.0;
    out_image->channels_mask = 0;
    out_image->userdata = NULL;
    out_image->is_ai_generated = false;
    out_image->creation_os[0] = '\0';
    out_image->software[0] = '\0';
    out_image->creation_date[0] = '\0';

    fclose(f);
    return true;
}

static bool save_raw(const char *filename, const fossil_image_t *image) {
    if (!filename || !image || !image->data) return false;
    FILE *f = fopen(filename, "wb");
    if (!f) return false;

    // Write header: width, height, channels, format
    raw_header_t hdr = {image->width, image->height, image->channels};
    fwrite(&hdr, sizeof(hdr), 1, f);
    fwrite(&image->format, sizeof(image->format), 1, f);

    // Write pixel data according to format
    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            fwrite(image->data, 1, image->size, f);
            break;
        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            fwrite(image->data, 1, image->size, f); // 2 bytes per channel, size should be correct
            break;
        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            fwrite(image->fdata, 1, image->size, f); // size in bytes
            break;
        default:
            fclose(f);
            return false;
    }

    fclose(f);
    return true;
}

// ======================================================
// Format-specific Load/Save functions for each pixel format
// ======================================================

// --- GRAY8 ---
static bool load_gray8(const char *filename, fossil_image_t *out_image) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;
    uint32_t w, h;
    if (fread(&w, sizeof(w), 1, f) != 1 || fread(&h, sizeof(h), 1, f) != 1) { fclose(f); return false; }
    out_image->width = w;
    out_image->height = h;
    out_image->channels = 1;
    out_image->format = FOSSIL_PIXEL_FORMAT_GRAY8;
    out_image->size = w * h;
    out_image->data = (uint8_t *)malloc(out_image->size);
    out_image->owns_data = true;
    if (!out_image->data) { fclose(f); return false; }
    fread(out_image->data, 1, out_image->size, f);
    fclose(f);
    return true;
}

static bool save_gray8(const char *filename, const fossil_image_t *image) {
    if (!image || !image->data) return false;
    FILE *f = fopen(filename, "wb");
    if (!f) return false;
    fwrite(&image->width, sizeof(image->width), 1, f);
    fwrite(&image->height, sizeof(image->height), 1, f);
    fwrite(image->data, 1, image->width * image->height, f);
    fclose(f);
    return true;
}

// --- GRAY16 ---
static bool load_gray16(const char *filename, fossil_image_t *out_image) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;
    uint32_t w, h;
    if (fread(&w, sizeof(w), 1, f) != 1 || fread(&h, sizeof(h), 1, f) != 1) { fclose(f); return false; }
    out_image->width = w;
    out_image->height = h;
    out_image->channels = 1;
    out_image->format = FOSSIL_PIXEL_FORMAT_GRAY16;
    out_image->size = w * h * 2;
    out_image->data = (uint8_t *)malloc(out_image->size);
    out_image->owns_data = true;
    if (!out_image->data) { fclose(f); return false; }
    fread(out_image->data, 2, w * h, f);
    fclose(f);
    return true;
}

static bool save_gray16(const char *filename, const fossil_image_t *image) {
    if (!image || !image->data) return false;
    FILE *f = fopen(filename, "wb");
    if (!f) return false;
    fwrite(&image->width, sizeof(image->width), 1, f);
    fwrite(&image->height, sizeof(image->height), 1, f);
    fwrite(image->data, 2, image->width * image->height, f);
    fclose(f);
    return true;
}

// --- RGB24 ---
// Already implemented as BMP/PPM

// --- RGB48 ---
static bool load_rgb48(const char *filename, fossil_image_t *out_image) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;
    uint32_t w, h;
    if (fread(&w, sizeof(w), 1, f) != 1 || fread(&h, sizeof(h), 1, f) != 1) { fclose(f); return false; }
    out_image->width = w;
    out_image->height = h;
    out_image->channels = 3;
    out_image->format = FOSSIL_PIXEL_FORMAT_RGB48;
    out_image->size = w * h * 3 * 2;
    out_image->data = (uint8_t *)malloc(out_image->size);
    out_image->owns_data = true;
    if (!out_image->data) { fclose(f); return false; }
    fread(out_image->data, 2, w * h * 3, f);
    fclose(f);
    return true;
}

static bool save_rgb48(const char *filename, const fossil_image_t *image) {
    if (!image || !image->data) return false;
    FILE *f = fopen(filename, "wb");
    if (!f) return false;
    fwrite(&image->width, sizeof(image->width), 1, f);
    fwrite(&image->height, sizeof(image->height), 1, f);
    fwrite(image->data, 2, image->width * image->height * 3, f);
    fclose(f);
    return true;
}

// --- RGBA32 ---
// Already implemented as BMP/PPM

// --- RGBA64 ---
static bool load_rgba64(const char *filename, fossil_image_t *out_image) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;
    uint32_t w, h;
    if (fread(&w, sizeof(w), 1, f) != 1 || fread(&h, sizeof(h), 1, f) != 1) { fclose(f); return false; }
    out_image->width = w;
    out_image->height = h;
    out_image->channels = 4;
    out_image->format = FOSSIL_PIXEL_FORMAT_RGBA64;
    out_image->size = w * h * 4 * 2;
    out_image->data = (uint8_t *)malloc(out_image->size);
    out_image->owns_data = true;
    if (!out_image->data) { fclose(f); return false; }
    fread(out_image->data, 2, w * h * 4, f);
    fclose(f);
    return true;
}

static bool save_rgba64(const char *filename, const fossil_image_t *image) {
    if (!image || !image->data) return false;
    FILE *f = fopen(filename, "wb");
    if (!f) return false;
    fwrite(&image->width, sizeof(image->width), 1, f);
    fwrite(&image->height, sizeof(image->height), 1, f);
    fwrite(image->data, 2, image->width * image->height * 4, f);
    fclose(f);
    return true;
}

// --- FLOAT32 ---
static bool load_float32(const char *filename, fossil_image_t *out_image) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;
    uint32_t w, h;
    if (fread(&w, sizeof(w), 1, f) != 1 || fread(&h, sizeof(h), 1, f) != 1) { fclose(f); return false; }
    out_image->width = w;
    out_image->height = h;
    out_image->channels = 1;
    out_image->format = FOSSIL_PIXEL_FORMAT_FLOAT32;
    out_image->size = w * h * sizeof(float);
    out_image->fdata = (float *)malloc(out_image->size);
    out_image->owns_data = true;
    if (!out_image->fdata) { fclose(f); return false; }
    fread(out_image->fdata, sizeof(float), w * h, f);
    fclose(f);
    return true;
}

static bool save_float32(const char *filename, const fossil_image_t *image) {
    if (!image || !image->fdata) return false;
    FILE *f = fopen(filename, "wb");
    if (!f) return false;
    fwrite(&image->width, sizeof(image->width), 1, f);
    fwrite(&image->height, sizeof(image->height), 1, f);
    fwrite(image->fdata, sizeof(float), image->width * image->height, f);
    fclose(f);
    return true;
}

// --- FLOAT32_RGB ---
static bool load_float32_rgb(const char *filename, fossil_image_t *out_image) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;
    uint32_t w, h;
    if (fread(&w, sizeof(w), 1, f) != 1 || fread(&h, sizeof(h), 1, f) != 1) { fclose(f); return false; }
    out_image->width = w;
    out_image->height = h;
    out_image->channels = 3;
    out_image->format = FOSSIL_PIXEL_FORMAT_FLOAT32_RGB;
    out_image->size = w * h * 3 * sizeof(float);
    out_image->fdata = (float *)malloc(out_image->size);
    out_image->owns_data = true;
    if (!out_image->fdata) { fclose(f); return false; }
    fread(out_image->fdata, sizeof(float), w * h * 3, f);
    fclose(f);
    return true;
}

static bool save_float32_rgb(const char *filename, const fossil_image_t *image) {
    if (!image || !image->fdata) return false;
    FILE *f = fopen(filename, "wb");
    if (!f) return false;
    fwrite(&image->width, sizeof(image->width), 1, f);
    fwrite(&image->height, sizeof(image->height), 1, f);
    fwrite(image->fdata, sizeof(float), image->width * image->height * 3, f);
    fclose(f);
    return true;
}

// --- FLOAT32_RGBA ---
static bool load_float32_rgba(const char *filename, fossil_image_t *out_image) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;
    uint32_t w, h;
    if (fread(&w, sizeof(w), 1, f) != 1 || fread(&h, sizeof(h), 1, f) != 1) { fclose(f); return false; }
    out_image->width = w;
    out_image->height = h;
    out_image->channels = 4;
    out_image->format = FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA;
    out_image->size = w * h * 4 * sizeof(float);
    out_image->fdata = (float *)malloc(out_image->size);
    out_image->owns_data = true;
    if (!out_image->fdata) { fclose(f); return false; }
    fread(out_image->fdata, sizeof(float), w * h * 4, f);
    fclose(f);
    return true;
}

static bool save_float32_rgba(const char *filename, const fossil_image_t *image) {
    if (!image || !image->fdata) return false;
    FILE *f = fopen(filename, "wb");
    if (!f) return false;
    fwrite(&image->width, sizeof(image->width), 1, f);
    fwrite(&image->height, sizeof(image->height), 1, f);
    fwrite(image->fdata, sizeof(float), image->width * image->height * 4, f);
    fclose(f);
    return true;
}

// --- INDEXED8 ---
static bool load_indexed8(const char *filename, fossil_image_t *out_image) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;
    uint32_t w, h;
    if (fread(&w, sizeof(w), 1, f) != 1 || fread(&h, sizeof(h), 1, f) != 1) { fclose(f); return false; }
    out_image->width = w;
    out_image->height = h;
    out_image->channels = 1;
    out_image->format = FOSSIL_PIXEL_FORMAT_INDEXED8;
    out_image->size = w * h;
    out_image->data = (uint8_t *)malloc(out_image->size);
    out_image->owns_data = true;
    if (!out_image->data) { fclose(f); return false; }
    fread(out_image->data, 1, out_image->size, f);
    fclose(f);
    return true;
}

static bool save_indexed8(const char *filename, const fossil_image_t *image) {
    if (!image || !image->data) return false;
    FILE *f = fopen(filename, "wb");
    if (!f) return false;
    fwrite(&image->width, sizeof(image->width), 1, f);
    fwrite(&image->height, sizeof(image->height), 1, f);
    fwrite(image->data, 1, image->width * image->height, f);
    fclose(f);
    return true;
}

// --- YUV24 ---
static bool load_yuv24(const char *filename, fossil_image_t *out_image) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;
    uint32_t w, h;
    if (fread(&w, sizeof(w), 1, f) != 1 || fread(&h, sizeof(h), 1, f) != 1) { fclose(f); return false; }
    out_image->width = w;
    out_image->height = h;
    out_image->channels = 3;
    out_image->format = FOSSIL_PIXEL_FORMAT_YUV24;
    out_image->size = w * h * 3;
    out_image->data = (uint8_t *)malloc(out_image->size);
    out_image->owns_data = true;
    if (!out_image->data) { fclose(f); return false; }
    fread(out_image->data, 1, out_image->size, f);
    fclose(f);
    return true;
}

static bool save_yuv24(const char *filename, const fossil_image_t *image) {
    if (!image || !image->data) return false;
    FILE *f = fopen(filename, "wb");
    if (!f) return false;
    fwrite(&image->width, sizeof(image->width), 1, f);
    fwrite(&image->height, sizeof(image->height), 1, f);
    fwrite(image->data, 1, image->width * image->height * 3, f);
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
    if (strcmp(format_id, "raw") == 0) return load_raw(filename, out_image);
    if (strcmp(format_id, "gray8") == 0) return load_gray8(filename, out_image);
    if (strcmp(format_id, "gray16") == 0) return load_gray16(filename, out_image);
    if (strcmp(format_id, "rgb48") == 0) return load_rgb48(filename, out_image);
    if (strcmp(format_id, "rgba64") == 0) return load_rgba64(filename, out_image);
    if (strcmp(format_id, "float32") == 0) return load_float32(filename, out_image);
    if (strcmp(format_id, "float32_rgb") == 0) return load_float32_rgb(filename, out_image);
    if (strcmp(format_id, "float32_rgba") == 0) return load_float32_rgba(filename, out_image);
    if (strcmp(format_id, "indexed8") == 0) return load_indexed8(filename, out_image);
    if (strcmp(format_id, "yuv24") == 0) return load_yuv24(filename, out_image);

    return false;
}

bool fossil_image_io_save(const char *filename, const char *format_id, const fossil_image_t *image) {
    if (!format_id || !image) return false;

    if (strcmp(format_id, "bmp") == 0) return save_bmp(filename, image);
    if (strcmp(format_id, "ppm") == 0) return save_ppm(filename, image);
    if (strcmp(format_id, "raw") == 0) return save_raw(filename, image);
    if (strcmp(format_id, "gray8") == 0) return save_gray8(filename, image);
    if (strcmp(format_id, "gray16") == 0) return save_gray16(filename, image);
    if (strcmp(format_id, "rgb48") == 0) return save_rgb48(filename, image);
    if (strcmp(format_id, "rgba64") == 0) return save_rgba64(filename, image);
    if (strcmp(format_id, "float32") == 0) return save_float32(filename, image);
    if (strcmp(format_id, "float32_rgb") == 0) return save_float32_rgb(filename, image);
    if (strcmp(format_id, "float32_rgba") == 0) return save_float32_rgba(filename, image);
    if (strcmp(format_id, "indexed8") == 0) return save_indexed8(filename, image);
    if (strcmp(format_id, "yuv24") == 0) return save_yuv24(filename, image);

    return false;
}

// ======================================================
// Image Generators
// ======================================================

bool fossil_image_io_generate(fossil_image_t *out_image, const char *type_id, uint32_t width, uint32_t height, fossil_pixel_format_t format, const float *params)
{
    if (!out_image || width == 0 || height == 0)
        return false;

    uint32_t channels = 0;
    size_t pixel_bytes = 0;
    bool is_float = false;

    switch (format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:      channels = 1; pixel_bytes = 1; break;
        case FOSSIL_PIXEL_FORMAT_GRAY16:     channels = 1; pixel_bytes = 2; break;
        case FOSSIL_PIXEL_FORMAT_RGB24:      channels = 3; pixel_bytes = 1; break;
        case FOSSIL_PIXEL_FORMAT_RGB48:      channels = 3; pixel_bytes = 2; break;
        case FOSSIL_PIXEL_FORMAT_RGBA32:     channels = 4; pixel_bytes = 1; break;
        case FOSSIL_PIXEL_FORMAT_RGBA64:     channels = 4; pixel_bytes = 2; break;
        case FOSSIL_PIXEL_FORMAT_FLOAT32:    channels = 1; pixel_bytes = 4; is_float = true; break;
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:channels = 3; pixel_bytes = 4; is_float = true; break;
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:channels = 4; pixel_bytes = 4; is_float = true; break;
        case FOSSIL_PIXEL_FORMAT_INDEXED8:   channels = 1; pixel_bytes = 1; break;
        case FOSSIL_PIXEL_FORMAT_YUV24:      channels = 3; pixel_bytes = 1; break;
        default: return false;
    }

    out_image->width = width;
    out_image->height = height;
    out_image->channels = channels;
    out_image->format = format;
    out_image->size = width * height * channels * pixel_bytes;
    out_image->owns_data = true;

    if (is_float)
        out_image->fdata = (float *)malloc(out_image->size);
    else
        out_image->data = (uint8_t *)malloc(out_image->size);

    if ((is_float && !out_image->fdata) || (!is_float && !out_image->data))
        return false;

    // Solid fill
    if (strcmp(type_id, "solid") == 0) {
        if (is_float) {
            float c[4] = {0.0f};
            if (params) for (uint32_t i = 0; i < channels; ++i)
                c[i] = params[i];
            for (size_t i = 0; i < width * height; ++i)
                for (uint32_t ch = 0; ch < channels; ++ch)
                    out_image->fdata[i * channels + ch] = c[ch];
        } else if (pixel_bytes == 2) {
            uint16_t c[4] = {0};
            if (params) for (uint32_t i = 0; i < channels; ++i)
                c[i] = (uint16_t)fminf(fmaxf(params[i], 0.0f), 65535.0f);
            for (size_t i = 0; i < width * height; ++i)
                for (uint32_t ch = 0; ch < channels; ++ch)
                    ((uint16_t *)out_image->data)[i * channels + ch] = c[ch];
        } else {
            uint8_t c[4] = {0};
            if (params) for (uint32_t i = 0; i < channels; ++i)
                c[i] = (uint8_t)fminf(fmaxf(params[i], 0.0f), 255.0f);
            for (size_t i = 0; i < width * height; ++i)
                for (uint32_t ch = 0; ch < channels; ++ch)
                    out_image->data[i * channels + ch] = c[ch];
        }
        return true;
    }

    // Gradient
    if (strcmp(type_id, "gradient") == 0) {
        if (is_float) {
            float start[4] = {0.0f}, end[4] = {1.0f,1.0f,1.0f,1.0f};
            if (params) {
                for (uint32_t i = 0; i < channels; ++i) start[i] = params[i];
                for (uint32_t i = 0; i < channels; ++i) end[i] = params[channels + i];
            }
            for (uint32_t y = 0; y < height; ++y) {
                float t = (float)y / (float)(height-1);
                for (uint32_t x = 0; x < width; ++x)
                    for (uint32_t ch = 0; ch < channels; ++ch)
                        out_image->fdata[(y*width + x)*channels + ch] = (1.0f-t)*start[ch] + t*end[ch];
            }
        } else if (pixel_bytes == 2) {
            uint16_t start[4] = {0}, end[4] = {65535,65535,65535,65535};
            if (params) {
                for (uint32_t i = 0; i < channels; ++i) start[i] = (uint16_t)params[i];
                for (uint32_t i = 0; i < channels; ++i) end[i] = (uint16_t)params[channels + i];
            }
            for (uint32_t y = 0; y < height; ++y) {
                float t = (float)y / (float)(height-1);
                for (uint32_t x = 0; x < width; ++x)
                    for (uint32_t ch = 0; ch < channels; ++ch)
                        ((uint16_t *)out_image->data)[(y*width + x)*channels + ch] =
                            (uint16_t)((1.0f-t)*start[ch] + t*end[ch]);
            }
        } else {
            uint8_t start[4] = {0}, end[4] = {255,255,255,255};
            if (params) {
                for (uint32_t i = 0; i < channels; ++i) start[i] = (uint8_t)params[i];
                for (uint32_t i = 0; i < channels; ++i) end[i] = (uint8_t)params[channels + i];
            }
            for (uint32_t y = 0; y < height; ++y) {
                float t = (float)y / (float)(height-1);
                for (uint32_t x = 0; x < width; ++x)
                    for (uint32_t ch = 0; ch < channels; ++ch)
                        out_image->data[(y*width + x)*channels + ch] =
                            (uint8_t)((1.0f-t)*start[ch] + t*end[ch]);
            }
        }
        return true;
    }

    // Checker
    if (strcmp(type_id, "checker") == 0) {
        uint32_t tile = 8;
        if (params) tile = (uint32_t)params[0];
        if (is_float) {
            float c1[4] = {0.0f}, c2[4] = {1.0f,1.0f,1.0f,1.0f};
            if (params && params[1]) for (uint32_t i = 0; i < channels; ++i) c1[i] = params[i+1];
            if (params && params[1+channels]) for (uint32_t i = 0; i < channels; ++i) c2[i] = params[i+1+channels];
            for (uint32_t y = 0; y < height; ++y)
                for (uint32_t x = 0; x < width; ++x) {
                    bool is_c1 = ((x/tile + y/tile) % 2 == 0);
                    float *col = is_c1 ? c1 : c2;
                    for (uint32_t ch = 0; ch < channels; ++ch)
                        out_image->fdata[(y*width + x)*channels + ch] = col[ch];
                }
        } else if (pixel_bytes == 2) {
            uint16_t c1[4] = {0}, c2[4] = {65535,65535,65535,65535};
            if (params && params[1]) for (uint32_t i = 0; i < channels; ++i) c1[i] = (uint16_t)params[i+1];
            if (params && params[1+channels]) for (uint32_t i = 0; i < channels; ++i) c2[i] = (uint16_t)params[i+1+channels];
            for (uint32_t y = 0; y < height; ++y)
                for (uint32_t x = 0; x < width; ++x) {
                    bool is_c1 = ((x/tile + y/tile) % 2 == 0);
                    uint16_t *col = is_c1 ? c1 : c2;
                    for (uint32_t ch = 0; ch < channels; ++ch)
                        ((uint16_t *)out_image->data)[(y*width + x)*channels + ch] = col[ch];
                }
        } else {
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
        }
        return true;
    }

    // Noise
    if (strcmp(type_id, "noise") == 0) {
        if (is_float) {
            for (size_t i = 0; i < width * height * channels; ++i)
                out_image->fdata[i] = (float)rand() / (float)RAND_MAX;
        } else if (pixel_bytes == 2) {
            for (size_t i = 0; i < width * height * channels; ++i)
                ((uint16_t *)out_image->data)[i] = (uint16_t)(rand() % 65536);
        } else {
            for (size_t i = 0; i < width * height * channels; ++i)
                out_image->data[i] = (uint8_t)(rand() % 256);
        }
        return true;
    }

    return false;
}
