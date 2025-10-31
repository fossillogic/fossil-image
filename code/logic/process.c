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
#include "fossil/image/process.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ======================================================
// Fossil Image — Core Utilities
// ======================================================

size_t fossil_image_bytes_per_pixel(fossil_pixel_format_t format) {
    switch (format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:   return 1;
        case FOSSIL_PIXEL_FORMAT_RGB24:   return 3;
        case FOSSIL_PIXEL_FORMAT_RGBA32:  return 4;
        case FOSSIL_PIXEL_FORMAT_FLOAT32: return 4;
        default:                          return 0;
    }
}

fossil_image_t *fossil_image_create(
    uint32_t width,
    uint32_t height,
    fossil_pixel_format_t format
) {
    fossil_image_t *img = (fossil_image_t *)calloc(1, sizeof(fossil_image_t));
    if (!img)
        return NULL;

    img->width = width;
    img->height = height;
    img->format = format;
    img->channels = (uint32_t)fossil_image_bytes_per_pixel(format);
    img->size = (size_t)width * (size_t)height * img->channels;
    img->owns_data = true;
    img->dpi_x = 96.0;
    img->dpi_y = 96.0;
    strncpy(img->name, "unnamed", sizeof(img->name) - 1);
    img->name[sizeof(img->name) - 1] = '\0';

    // Check for integer overflow in size calculation
    if (width == 0 || height == 0 || img->channels == 0 ||
        img->size / img->channels / width != height) {
        free(img);
        return NULL;
    }

    img->data = (uint8_t *)calloc(img->size, sizeof(uint8_t));
    if (!img->data) {
        free(img);
        return NULL;
    }
    return img;
}

void fossil_image_destroy(fossil_image_t *image) {
    if (!image) return;
    if (image->owns_data && image->data)
        free(image->data);
    free(image);
}

// ======================================================
// Fossil Image — Process Sub-Library
// ======================================================

bool fossil_image_process_resize(
    fossil_image_t *image,
    uint32_t new_w,
    uint32_t new_h,
    fossil_interp_t mode
) {
    if (!image || !image->data)
        return false;

    size_t channels = image->channels;
    size_t new_size = (size_t)new_w * (size_t)new_h * channels;
    uint8_t *new_data = (uint8_t *)calloc(new_size, sizeof(uint8_t));
    if (!new_data)
        return false;

    switch (mode) {
        case FOSSIL_INTERP_NEAREST:
            // Nearest-neighbor scaling
            for (uint32_t y = 0; y < new_h; ++y) {
                for (uint32_t x = 0; x < new_w; ++x) {
                    uint32_t src_x = (uint32_t)((float)x * image->width / new_w);
                    uint32_t src_y = (uint32_t)((float)y * image->height / new_h);
                    size_t src_i = (src_y * image->width + src_x) * channels;
                    size_t dst_i = (y * new_w + x) * channels;
                    if (src_i + channels <= image->size && dst_i + channels <= new_size) {
                        memcpy(&new_data[dst_i], &image->data[src_i], channels);
                    }
                }
            }
            break;
        case FOSSIL_INTERP_LINEAR:
            // Bilinear interpolation (simple implementation)
            for (uint32_t y = 0; y < new_h; ++y) {
                float src_yf = (float)y * image->height / new_h;
                uint32_t y0 = (uint32_t)src_yf;
                uint32_t y1 = (y0 + 1 < image->height) ? y0 + 1 : y0;
                float wy = src_yf - y0;
                for (uint32_t x = 0; x < new_w; ++x) {
                    float src_xf = (float)x * image->width / new_w;
                    uint32_t x0 = (uint32_t)src_xf;
                    uint32_t x1 = (x0 + 1 < image->width) ? x0 + 1 : x0;
                    float wx = src_xf - x0;
                    size_t dst_i = (y * new_w + x) * channels;
                    for (size_t c = 0; c < channels; ++c) {
                        uint8_t p00 = image->data[(y0 * image->width + x0) * channels + c];
                        uint8_t p01 = image->data[(y0 * image->width + x1) * channels + c];
                        uint8_t p10 = image->data[(y1 * image->width + x0) * channels + c];
                        uint8_t p11 = image->data[(y1 * image->width + x1) * channels + c];
                        float val = (1 - wx) * (1 - wy) * p00 +
                                    wx * (1 - wy) * p01 +
                                    (1 - wx) * wy * p10 +
                                    wx * wy * p11;
                        new_data[dst_i + c] = (uint8_t)(val + 0.5f);
                    }
                }
            }
            break;
        case FOSSIL_INTERP_CUBIC:
        case FOSSIL_INTERP_LANCZOS:
            // Not implemented, fallback to nearest
            for (uint32_t y = 0; y < new_h; ++y) {
                for (uint32_t x = 0; x < new_w; ++x) {
                    uint32_t src_x = (uint32_t)((float)x * image->width / new_w);
                    uint32_t src_y = (uint32_t)((float)y * image->height / new_h);
                    size_t src_i = (src_y * image->width + src_x) * channels;
                    size_t dst_i = (y * new_w + x) * channels;
                    if (src_i + channels <= image->size && dst_i + channels <= new_size) {
                        memcpy(&new_data[dst_i], &image->data[src_i], channels);
                    }
                }
            }
            break;
        default:
            free(new_data);
            return false;
    }

    free(image->data);
    image->data = new_data;
    image->width = new_w;
    image->height = new_h;
    image->size = new_size;
    return true;
}

bool fossil_image_process_crop(
    fossil_image_t *image,
    uint32_t x,
    uint32_t y,
    uint32_t w,
    uint32_t h
) {
    if (!image || !image->data)
        return false;

    if (x + w > image->width || y + h > image->height)
        return false;

    size_t channels = image->channels;
    size_t new_size = (size_t)w * (size_t)h * channels;
    uint8_t *new_data = (uint8_t *)calloc(new_size, sizeof(uint8_t));
    if (!new_data)
        return false;

    for (uint32_t j = 0; j < h; ++j) {
        size_t src_idx = ((y + j) * image->width + x) * channels;
        size_t dst_idx = j * w * channels;
        // Ensure we do not read beyond the source buffer
        if (src_idx + w * channels <= image->size && dst_idx + w * channels <= new_size) {
            memcpy(&new_data[dst_idx], &image->data[src_idx], w * channels);
        }
    }

    free(image->data);
    image->data = new_data;
    image->width = w;
    image->height = h;
    image->size = new_size;
    return true;
}

bool fossil_image_process_flip(
    fossil_image_t *image,
    bool horizontal,
    bool vertical
) {
    if (!image || !image->data)
        return false;

    uint32_t w = image->width;
    uint32_t h = image->height;
    size_t c = image->channels;
    uint8_t *temp = (uint8_t *)calloc(image->size, sizeof(uint8_t));
    if (!temp)
        return false;

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            uint32_t sx = horizontal ? (w - 1 - x) : x;
            uint32_t sy = vertical ? (h - 1 - y) : y;
            size_t src_idx = (sy * w + sx) * c;
            size_t dst_idx = (y * w + x) * c;
            if (src_idx + c <= image->size && dst_idx + c <= image->size) {
                memcpy(&temp[dst_idx], &image->data[src_idx], c);
            }
        }
    }

    memcpy(image->data, temp, image->size);
    free(temp);
    return true;
}

bool fossil_image_process_rotate(
    fossil_image_t *image,
    float degrees
) {
    if (!image || !image->data)
        return false;

    float radians = degrees * (float)M_PI / 180.0f;
    float cos_a = cosf(radians);
    float sin_a = sinf(radians);

    uint32_t w = image->width;
    uint32_t h = image->height;
    size_t c = image->channels;

    uint8_t *new_data = (uint8_t *)calloc(image->size, 1);
    if (!new_data)
        return false;

    int cx = (int)w / 2;
    int cy = (int)h / 2;

    for (int y = 0; y < (int)h; ++y) {
        for (int x = 0; x < (int)w; ++x) {
            int rx = (int)(cos_a * (x - cx) - sin_a * (y - cy) + cx);
            int ry = (int)(sin_a * (x - cx) + cos_a * (y - cy) + cy);
            if (rx >= 0 && rx < (int)w && ry >= 0 && ry < (int)h) {
                memcpy(&new_data[(y * w + x) * c],
                       &image->data[(ry * w + rx) * c],
                       c);
            }
        }
    }

    free(image->data);
    image->data = new_data;
    // image->width, image->height, and image->size remain unchanged for in-place rotation
    return true;
}

bool fossil_image_process_blend(
    fossil_image_t *dst,
    const fossil_image_t *src,
    float ratio
) {
    if (!dst || !src || !dst->data || !src->data)
        return false;

    if (dst->width != src->width || dst->height != src->height || dst->channels != src->channels)
        return false;

    size_t min_size = dst->size < src->size ? dst->size : src->size;
    ratio = fmaxf(0.0f, fminf(1.0f, ratio));
    for (size_t i = 0; i < min_size; ++i) {
        dst->data[i] = (uint8_t)(
            fmaxf(0.0f, fminf(255.0f, (1.0f - ratio) * dst->data[i] + ratio * src->data[i]))
        );
    }
    return true;
}

bool fossil_image_process_composite(
    fossil_image_t *dst,
    const fossil_image_t *overlay,
    uint32_t x,
    uint32_t y,
    float alpha
) {
    if (!dst || !overlay || !dst->data || !overlay->data)
        return false;

    // Ensure channel counts match
    if (dst->channels != overlay->channels)
        return false;

    // Ensure overlay fits within destination bounds
    if (x >= dst->width || y >= dst->height)
        return false;
    if (overlay->width > dst->width - x || overlay->height > dst->height - y)
        return false;

    alpha = fmaxf(0.0f, fminf(1.0f, alpha));
    uint32_t w = overlay->width;
    uint32_t h = overlay->height;
    size_t c = dst->channels;

    for (uint32_t j = 0; j < h; ++j) {
        for (uint32_t i = 0; i < w; ++i) {
            uint32_t dx = x + i;
            uint32_t dy = y + j;
            size_t di = (dy * dst->width + dx) * c;
            size_t si = (j * overlay->width + i) * c;
            // Prevent buffer overflow
            if (di + c > dst->size || si + c > overlay->size)
                continue;
            for (size_t k = 0; k < c; ++k) {
                dst->data[di + k] = (uint8_t)(
                    (1.0f - alpha) * dst->data[di + k] + alpha * overlay->data[si + k]
                );
            }
        }
    }
    return true;
}

bool fossil_image_process_grayscale(fossil_image_t *image) {
    if (!image || !image->data)
        return false;

    // Only convert if format is RGB24 or RGBA32
    if (image->format != FOSSIL_PIXEL_FORMAT_RGB24 && image->format != FOSSIL_PIXEL_FORMAT_RGBA32)
        return false;

    if (image->channels < 3)
        return true; // already grayscale

    size_t npixels = (size_t)image->width * image->height;
    size_t data_size = npixels * image->channels;
    if (image->size < data_size)
        return false; // Prevent buffer overflow

    uint8_t *new_data = (uint8_t *)calloc(npixels, 1);
    if (!new_data)
        return false;

    for (size_t i = 0; i < npixels; ++i) {
        uint8_t r = image->data[i * image->channels + 0];
        uint8_t g = image->data[i * image->channels + 1];
        uint8_t b = image->data[i * image->channels + 2];
        new_data[i] = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
    }

    free(image->data);
    image->data = new_data;
    image->channels = 1;
    image->format = FOSSIL_PIXEL_FORMAT_GRAY8;
    image->size = npixels;
    return true;
}

bool fossil_image_process_threshold(fossil_image_t *image, uint8_t threshold) {
    if (!image || !image->data)
        return false;

    // Ensure threshold is within valid range
    if (threshold > 255)
        threshold = 255;

    size_t pixels = image->width * image->height * image->channels;
    for (size_t i = 0; i < pixels; ++i) {
        image->data[i] = (image->data[i] >= threshold) ? 255 : 0;
    }
    return true;
}

bool fossil_image_process_invert(fossil_image_t *image) {
    if (!image || !image->data)
        return false;

    size_t pixels = image->width * image->height * image->channels;
    for (size_t i = 0; i < pixels; ++i)
        image->data[i] = (uint8_t)(255U - (uint8_t)image->data[i]);
    return true;
}

bool fossil_image_process_normalize(fossil_image_t *image) {
    if (!image || !image->data)
        return false;

    size_t pixels = image->width * image->height * image->channels;
    uint8_t min_val = 255, max_val = 0;
    for (size_t i = 0; i < pixels; ++i) {
        if (image->data[i] < min_val) min_val = image->data[i];
        if (image->data[i] > max_val) max_val = image->data[i];
    }

    if (max_val == min_val)
        return true;

    float scale = 255.0f / (float)(max_val - min_val);
    for (size_t i = 0; i < pixels; ++i) {
        float normalized = ((float)image->data[i] - (float)min_val) * scale;
        if (normalized < 0.0f) normalized = 0.0f;
        if (normalized > 255.0f) normalized = 255.0f;
        image->data[i] = (uint8_t)(normalized + 0.5f); // round to nearest
    }
    return true;
}
