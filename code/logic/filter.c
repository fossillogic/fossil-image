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
#include "fossil/image/filter.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ======================================================
// Fossil Image — Filter Sub-Library Implementation
// ======================================================

static inline uint8_t clamp8(float v) {
    if (v < 0.0f) return 0;
    if (v > 255.0f) return 255;
    return (uint8_t)v;
}

/**
 * @brief Core 3x3 convolution kernel operation.
 */
bool fossil_image_filter_convolve3x3(
    fossil_image_t *image,
    const float kernel[3][3],
    float scale,
    float bias
) {
    if (!image || image->channels == 0 || image->width < 3 || image->height < 3)
        return false;

    uint32_t w = image->width;
    uint32_t h = image->height;
    size_t c = image->channels;
    size_t pixel_count = w * h;

    // Determine pixel format and buffer pointers
    switch (image->format) {
    case FOSSIL_PIXEL_FORMAT_GRAY8:
    case FOSSIL_PIXEL_FORMAT_RGB24:
    case FOSSIL_PIXEL_FORMAT_RGBA32:
    case FOSSIL_PIXEL_FORMAT_INDEXED8:
    case FOSSIL_PIXEL_FORMAT_YUV24: {
        uint8_t *src = image->data;
        uint8_t *dst = (uint8_t *)calloc(image->size, 1);
        if (!dst)
            return false;

        for (uint32_t y = 1; y < h - 1; ++y) {
            for (uint32_t x = 1; x < w - 1; ++x) {
                for (size_t ch = 0; ch < c; ++ch) {
                    float sum = 0.0f;
                    for (int ky = -1; ky <= 1; ++ky) {
                        for (int kx = -1; kx <= 1; ++kx) {
                            size_t idx = ((y + ky) * w + (x + kx)) * c + ch;
                            sum += src[idx] * kernel[ky + 1][kx + 1];
                        }
                    }
                    sum = sum * scale + bias;
                    dst[(y * w + x) * c + ch] = clamp8(sum);
                }
            }
        }
        memcpy(image->data, dst, image->size);
        free(dst);
        break;
    }
    case FOSSIL_PIXEL_FORMAT_GRAY16:
    case FOSSIL_PIXEL_FORMAT_RGB48:
    case FOSSIL_PIXEL_FORMAT_RGBA64: {
        uint16_t *src = (uint16_t *)image->data;
        uint16_t *dst = (uint16_t *)calloc(image->size / 2, sizeof(uint16_t));
        if (!dst)
            return false;

        for (uint32_t y = 1; y < h - 1; ++y) {
            for (uint32_t x = 1; x < w - 1; ++x) {
                for (size_t ch = 0; ch < c; ++ch) {
                    float sum = 0.0f;
                    for (int ky = -1; ky <= 1; ++ky) {
                        for (int kx = -1; kx <= 1; ++kx) {
                            size_t idx = ((y + ky) * w + (x + kx)) * c + ch;
                            sum += src[idx] * kernel[ky + 1][kx + 1];
                        }
                    }
                    sum = sum * scale + bias;
                    if (sum < 0.0f) sum = 0.0f;
                    if (sum > 65535.0f) sum = 65535.0f;
                    dst[(y * w + x) * c + ch] = (uint16_t)sum;
                }
            }
        }
        memcpy(image->data, dst, image->size);
        free(dst);
        break;
    }
    case FOSSIL_PIXEL_FORMAT_FLOAT32:
    case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
    case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA: {
        float *src = image->fdata;
        float *dst = (float *)calloc(pixel_count * c, sizeof(float));
        if (!dst)
            return false;

        for (uint32_t y = 1; y < h - 1; ++y) {
            for (uint32_t x = 1; x < w - 1; ++x) {
                for (size_t ch = 0; ch < c; ++ch) {
                    float sum = 0.0f;
                    for (int ky = -1; ky <= 1; ++ky) {
                        for (int kx = -1; kx <= 1; ++kx) {
                            size_t idx = ((y + ky) * w + (x + kx)) * c + ch;
                            sum += src[idx] * kernel[ky + 1][kx + 1];
                        }
                    }
                    dst[(y * w + x) * c + ch] = sum * scale + bias;
                }
            }
        }
        memcpy(image->fdata, dst, pixel_count * c * sizeof(float));
        free(dst);
        break;
    }
    default:
        // Unsupported format
        return false;
    }

    return true;
}

// ------------------------------------------------------
// Predefined Filters
// ------------------------------------------------------

bool fossil_image_filter_blur(fossil_image_t *image, float radius) {
    if (!image || image->channels == 0 || image->width < 3 || image->height < 3)
        return false;

    // If radius <= 1, use single-pass 3x3 Gaussian blur.
    if (radius <= 1.0f) {
        static const float kernel[3][3] = {
            {1, 2, 1},
            {2, 4, 2},
            {1, 2, 1}
        };
        return fossil_image_filter_convolve3x3(image, kernel, 1.0f / 16.0f, 0.0f);
    }

    // Multi-pass blur: apply the 3x3 kernel multiple times.
    int passes = (int)radius;
    bool ok = true;
    for (int i = 0; i < passes && ok; ++i) {
        static const float kernel[3][3] = {
            {1, 2, 1},
            {2, 4, 2},
            {1, 2, 1}
        };
        ok = fossil_image_filter_convolve3x3(image, kernel, 1.0f / 16.0f, 0.0f);
    }
    return ok;
}

bool fossil_image_filter_sharpen(fossil_image_t *image) {
    if (!image || image->channels == 0 || image->width < 3 || image->height < 3)
        return false;

    static const float kernel[3][3] = {
        { 0, -1,  0},
        {-1,  5, -1},
        { 0, -1,  0}
    };

    switch (image->format) {
    case FOSSIL_PIXEL_FORMAT_GRAY8:
    case FOSSIL_PIXEL_FORMAT_RGB24:
    case FOSSIL_PIXEL_FORMAT_RGBA32:
    case FOSSIL_PIXEL_FORMAT_INDEXED8:
    case FOSSIL_PIXEL_FORMAT_YUV24:
        return fossil_image_filter_convolve3x3(image, kernel, 1.0f, 0.0f);

    case FOSSIL_PIXEL_FORMAT_GRAY16:
    case FOSSIL_PIXEL_FORMAT_RGB48:
    case FOSSIL_PIXEL_FORMAT_RGBA64:
        return fossil_image_filter_convolve3x3(image, kernel, 1.0f, 0.0f);

    case FOSSIL_PIXEL_FORMAT_FLOAT32:
    case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
    case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
        return fossil_image_filter_convolve3x3(image, kernel, 1.0f, 0.0f);

    default:
        // Unsupported format
        return false;
    }
}

bool fossil_image_filter_edge(fossil_image_t *image) {
    if (!image || image->channels == 0 || image->width < 3 || image->height < 3)
        return false;

    static const float kernel[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

    switch (image->format) {
    case FOSSIL_PIXEL_FORMAT_GRAY8:
    case FOSSIL_PIXEL_FORMAT_RGB24:
    case FOSSIL_PIXEL_FORMAT_RGBA32:
    case FOSSIL_PIXEL_FORMAT_INDEXED8:
    case FOSSIL_PIXEL_FORMAT_YUV24:
        return fossil_image_filter_convolve3x3(image, kernel, 1.0f, 0.0f);

    case FOSSIL_PIXEL_FORMAT_GRAY16:
    case FOSSIL_PIXEL_FORMAT_RGB48:
    case FOSSIL_PIXEL_FORMAT_RGBA64:
        return fossil_image_filter_convolve3x3(image, kernel, 1.0f, 0.0f);

    case FOSSIL_PIXEL_FORMAT_FLOAT32:
    case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
    case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
        return fossil_image_filter_convolve3x3(image, kernel, 1.0f, 0.0f);

    default:
        // Unsupported format
        return false;
    }
}

bool fossil_image_filter_emboss(fossil_image_t *image) {
    if (!image || image->channels == 0 || image->width < 3 || image->height < 3)
        return false;

    static const float kernel[3][3] = {
        {-2, -1,  0},
        {-1,  1,  1},
        { 0,  1,  2}
    };

    switch (image->format) {
    case FOSSIL_PIXEL_FORMAT_GRAY8:
    case FOSSIL_PIXEL_FORMAT_RGB24:
    case FOSSIL_PIXEL_FORMAT_RGBA32:
    case FOSSIL_PIXEL_FORMAT_INDEXED8:
    case FOSSIL_PIXEL_FORMAT_YUV24:
        return fossil_image_filter_convolve3x3(image, kernel, 1.0f, 128.0f);

    case FOSSIL_PIXEL_FORMAT_GRAY16:
    case FOSSIL_PIXEL_FORMAT_RGB48:
    case FOSSIL_PIXEL_FORMAT_RGBA64:
        return fossil_image_filter_convolve3x3(image, kernel, 1.0f, 32768.0f);

    case FOSSIL_PIXEL_FORMAT_FLOAT32:
    case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
    case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
        return fossil_image_filter_convolve3x3(image, kernel, 1.0f, 0.5f);

    default:
        // Unsupported format
        return false;
    }
}
