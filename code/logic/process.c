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
        case FOSSIL_PIXEL_FORMAT_GRAY8:        return 1;
        case FOSSIL_PIXEL_FORMAT_GRAY16:       return 2;
        case FOSSIL_PIXEL_FORMAT_RGB24:        return 3;
        case FOSSIL_PIXEL_FORMAT_RGB48:        return 6;
        case FOSSIL_PIXEL_FORMAT_RGBA32:       return 4;
        case FOSSIL_PIXEL_FORMAT_RGBA64:       return 8;
        case FOSSIL_PIXEL_FORMAT_FLOAT32:      return 4;
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:  return 12;
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA: return 16;
        case FOSSIL_PIXEL_FORMAT_INDEXED8:     return 1;
        case FOSSIL_PIXEL_FORMAT_YUV24:        return 3;
        default:                               return 0;
    }
}

fossil_image_t *fossil_image_process_create(
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

    // Set channels based on format
    switch (format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
            img->channels = 1;
            break;
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            img->channels = 3;
            break;
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            img->channels = 4;
            break;
        default:
            img->channels = 0;
            break;
    }

    // Calculate buffer size in bytes
    size_t bytes_per_pixel;
    switch (format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
            bytes_per_pixel = 1;
            break;
        case FOSSIL_PIXEL_FORMAT_GRAY16:
            bytes_per_pixel = 2;
            break;
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            bytes_per_pixel = 3;
            break;
        case FOSSIL_PIXEL_FORMAT_RGB48:
            bytes_per_pixel = 6;
            break;
        case FOSSIL_PIXEL_FORMAT_RGBA32:
            bytes_per_pixel = 4;
            break;
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            bytes_per_pixel = 8;
            break;
        case FOSSIL_PIXEL_FORMAT_FLOAT32:
            bytes_per_pixel = 4;
            break;
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
            bytes_per_pixel = 12;
            break;
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            bytes_per_pixel = 16;
            break;
        default:
            bytes_per_pixel = 0;
            break;
    }

    img->size = (size_t)width * (size_t)height * bytes_per_pixel;
    img->owns_data = true;
    img->dpi_x = 96.0;
    img->dpi_y = 96.0;
    img->exposure = 0.0;
    img->channels_mask = 0;
    img->userdata = NULL;
    img->is_ai_generated = false;
    img->name[0] = '\0';
    img->author[0] = '\0';
    img->creation_os[0] = '\0';
    img->software[0] = '\0';
    img->creation_date[0] = '\0';
    strncpy(img->name, "unnamed", sizeof(img->name) - 1);
    img->name[sizeof(img->name) - 1] = '\0';

    // Check for integer overflow in size calculation
    if (width == 0 || height == 0 || bytes_per_pixel == 0 ||
        img->size / bytes_per_pixel / width != height) {
        free(img);
        return NULL;
    }

    // Allocate buffer
    if (format == FOSSIL_PIXEL_FORMAT_FLOAT32 ||
        format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGB ||
        format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA) {
        img->fdata = (float *)calloc(img->size / sizeof(float), sizeof(float));
        if (!img->fdata) {
            free(img);
            return NULL;
        }
    } else {
        img->data = (uint8_t *)calloc(img->size, sizeof(uint8_t));
        if (!img->data) {
            free(img);
            return NULL;
        }
    }
    return img;
}

void fossil_image_process_destroy(fossil_image_t *image) {
    if (!image) return;

    if (image->owns_data) {
        switch (image->format) {
            case FOSSIL_PIXEL_FORMAT_FLOAT32:
            case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
            case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
                if (image->fdata) {
                    free(image->fdata);
                    image->fdata = NULL;
                }
                break;
            default:
                if (image->data) {
                    free(image->data);
                    image->data = NULL;
                }
                break;
        }
    }

    // Reset all pointers and fields to avoid dangling references
    image->data = NULL;
    image->fdata = NULL;
    image->userdata = NULL;
    image->size = 0;
    image->width = 0;
    image->height = 0;
    image->channels = 0;
    image->channels_mask = 0;
    image->dpi_x = 0.0;
    image->dpi_y = 0.0;
    image->exposure = 0.0;
    image->owns_data = false;
    image->is_ai_generated = false;
    image->name[0] = '\0';
    image->author[0] = '\0';
    image->creation_os[0] = '\0';
    image->software[0] = '\0';
    image->creation_date[0] = '\0';

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
    if (!image)
        return false;

    // Check for invalid resize dimensions
    if (new_w == 0 || new_h == 0)
        return false;

    size_t channels = image->channels;
    size_t bytes_per_pixel = fossil_image_bytes_per_pixel(image->format);
    size_t new_size = (size_t)new_w * (size_t)new_h * bytes_per_pixel;

    // Handle float formats
    bool is_float = (
        image->format == FOSSIL_PIXEL_FORMAT_FLOAT32 ||
        image->format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGB ||
        image->format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA
    );

    void *new_buffer = NULL;
    if (is_float) {
        new_buffer = calloc(new_size / sizeof(float), sizeof(float));
    } else {
        new_buffer = calloc(new_size, 1);
    }
    if (!new_buffer)
        return false;

    switch (mode) {
        case FOSSIL_INTERP_NEAREST:
            if (is_float) {
                float *src = image->fdata;
                float *dst = (float *)new_buffer;
                for (uint32_t y = 0; y < new_h; ++y) {
                    for (uint32_t x = 0; x < new_w; ++x) {
                        uint32_t src_x = (uint32_t)((float)x * image->width / new_w);
                        uint32_t src_y = (uint32_t)((float)y * image->height / new_h);
                        size_t src_i = (src_y * image->width + src_x) * channels;
                        size_t dst_i = (y * new_w + x) * channels;
                        for (size_t c = 0; c < channels; ++c) {
                            dst[dst_i + c] = src[src_i + c];
                        }
                    }
                }
            } else {
                uint8_t *src = image->data;
                uint8_t *dst = (uint8_t *)new_buffer;
                for (uint32_t y = 0; y < new_h; ++y) {
                    for (uint32_t x = 0; x < new_w; ++x) {
                        uint32_t src_x = (uint32_t)((float)x * image->width / new_w);
                        uint32_t src_y = (uint32_t)((float)y * image->height / new_h);
                        size_t src_i = (src_y * image->width + src_x) * channels;
                        size_t dst_i = (y * new_w + x) * channels;
                        memcpy(&dst[dst_i], &src[src_i], channels);
                    }
                }
            }
            break;
        case FOSSIL_INTERP_LINEAR:
            if (is_float) {
                float *src = image->fdata;
                float *dst = (float *)new_buffer;
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
                            float p00 = src[(y0 * image->width + x0) * channels + c];
                            float p01 = src[(y0 * image->width + x1) * channels + c];
                            float p10 = src[(y1 * image->width + x0) * channels + c];
                            float p11 = src[(y1 * image->width + x1) * channels + c];
                            float val = (1 - wx) * (1 - wy) * p00 +
                                        wx * (1 - wy) * p01 +
                                        (1 - wx) * wy * p10 +
                                        wx * wy * p11;
                            dst[dst_i + c] = val;
                        }
                    }
                }
            } else {
                uint8_t *src = image->data;
                uint8_t *dst = (uint8_t *)new_buffer;
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
                            uint8_t p00 = src[(y0 * image->width + x0) * channels + c];
                            uint8_t p01 = src[(y0 * image->width + x1) * channels + c];
                            uint8_t p10 = src[(y1 * image->width + x0) * channels + c];
                            uint8_t p11 = src[(y1 * image->width + x1) * channels + c];
                            float val = (1 - wx) * (1 - wy) * p00 +
                                        wx * (1 - wy) * p01 +
                                        (1 - wx) * wy * p10 +
                                        wx * wy * p11;
                            dst[dst_i + c] = (uint8_t)(val + 0.5f);
                        }
                    }
                }
            }
            break;
        case FOSSIL_INTERP_CUBIC:
        case FOSSIL_INTERP_LANCZOS:
        case FOSSIL_INTERP_BICUBIC:
        case FOSSIL_INTERP_MITCHELL:
        case FOSSIL_INTERP_BSPLINE:
            // Not implemented, fallback to nearest
            if (is_float) {
                float *src = image->fdata;
                float *dst = (float *)new_buffer;
                for (uint32_t y = 0; y < new_h; ++y) {
                    for (uint32_t x = 0; x < new_w; ++x) {
                        uint32_t src_x = (uint32_t)((float)x * image->width / new_w);
                        uint32_t src_y = (uint32_t)((float)y * image->height / new_h);
                        size_t src_i = (src_y * image->width + src_x) * channels;
                        size_t dst_i = (y * new_w + x) * channels;
                        for (size_t c = 0; c < channels; ++c) {
                            dst[dst_i + c] = src[src_i + c];
                        }
                    }
                }
            } else {
                uint8_t *src = image->data;
                uint8_t *dst = (uint8_t *)new_buffer;
                for (uint32_t y = 0; y < new_h; ++y) {
                    for (uint32_t x = 0; x < new_w; ++x) {
                        uint32_t src_x = (uint32_t)((float)x * image->width / new_w);
                        uint32_t src_y = (uint32_t)((float)y * image->height / new_h);
                        size_t src_i = (src_y * image->width + src_x) * channels;
                        size_t dst_i = (y * new_w + x) * channels;
                        memcpy(&dst[dst_i], &src[src_i], channels);
                    }
                }
            }
            break;
        default:
            free(new_buffer);
            return false;
    }

    // Free old buffer and update image
    if (is_float) {
        free(image->fdata);
        image->fdata = (float *)new_buffer;
    } else {
        free(image->data);
        image->data = (uint8_t *)new_buffer;
    }
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
    if (!image)
        return false;

    size_t bytes_per_pixel = fossil_image_bytes_per_pixel(image->format);
    size_t channels = image->channels;
    size_t new_size = (size_t)w * (size_t)h * bytes_per_pixel;

    bool is_float = (
        image->format == FOSSIL_PIXEL_FORMAT_FLOAT32 ||
        image->format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGB ||
        image->format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA
    );

    if (x + w > image->width || y + h > image->height)
        return false;

    if (is_float) {
        float *src = image->fdata;
        float *new_fdata = (float *)calloc(new_size / sizeof(float), sizeof(float));
        if (!new_fdata)
            return false;

        for (uint32_t j = 0; j < h; ++j) {
            size_t src_idx = ((y + j) * image->width + x) * channels;
            size_t dst_idx = j * w * channels;
            if (src_idx + w * channels <= image->width * image->height * channels &&
                dst_idx + w * channels <= w * h * channels) {
                memcpy(&new_fdata[dst_idx], &src[src_idx], w * channels * sizeof(float));
            }
        }
        free(image->fdata);
        image->fdata = new_fdata;
    } else {
        uint8_t *src = image->data;
        uint8_t *new_data = (uint8_t *)calloc(new_size, sizeof(uint8_t));
        if (!new_data)
            return false;

        for (uint32_t j = 0; j < h; ++j) {
            size_t src_idx = ((y + j) * image->width + x) * channels;
            size_t dst_idx = j * w * channels;
            if (src_idx + w * channels <= image->width * image->height * channels &&
                dst_idx + w * channels <= w * h * channels) {
                memcpy(&new_data[dst_idx], &src[src_idx], w * channels * sizeof(uint8_t));
            }
        }
    }

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
    if (!image)
        return false;

    uint32_t w = image->width;
    uint32_t h = image->height;
    size_t c = image->channels;

    bool is_float = (
        image->format == FOSSIL_PIXEL_FORMAT_FLOAT32 ||
        image->format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGB ||
        image->format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA
    );

    if (is_float) {
        if (!image->fdata)
            return false;
        float *temp = (float *)calloc(image->size / sizeof(float), sizeof(float));
        if (!temp)
            return false;

        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                uint32_t sx = horizontal ? (w - 1 - x) : x;
                uint32_t sy = vertical ? (h - 1 - y) : y;
                size_t src_idx = (sy * w + sx) * c;
                size_t dst_idx = (y * w + x) * c;
                if (src_idx + c <= image->size / sizeof(float) && dst_idx + c <= image->size / sizeof(float)) {
                    memcpy(&temp[dst_idx], &image->fdata[src_idx], c * sizeof(float));
                }
            }
        }
        memcpy(image->fdata, temp, image->size);
        free(temp);
    } else {
        if (!image->data)
            return false;
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
    }
    return true;
}

bool fossil_image_process_rotate(
    fossil_image_t *image,
    float degrees
) {
    if (!image)
        return false;

    bool is_float = (
        image->format == FOSSIL_PIXEL_FORMAT_FLOAT32 ||
        image->format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGB ||
        image->format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA
    );

    uint32_t w = image->width;
    uint32_t h = image->height;
    size_t c = image->channels;

    float radians = degrees * (float)M_PI / 180.0f;
    float cos_a = cosf(radians);
    float sin_a = sinf(radians);

    int cx = (int)w / 2;
    int cy = (int)h / 2;

    if (is_float) {
        float *new_fdata = (float *)calloc(image->size / sizeof(float), sizeof(float));
        if (!new_fdata)
            return false;

        for (int y = 0; y < (int)h; ++y) {
            for (int x = 0; x < (int)w; ++x) {
                float fx = (float)x - cx;
                float fy = (float)y - cy;
                float rx = cos_a * fx - sin_a * fy + cx;
                float ry = sin_a * fx + cos_a * fy + cy;
                int irx = (int)(rx + 0.5f);
                int iry = (int)(ry + 0.5f);
                if (irx >= 0 && irx < (int)w && iry >= 0 && iry < (int)h) {
                    size_t src_idx = (iry * w + irx) * c;
                    size_t dst_idx = (y * w + x) * c;
                    memcpy(&new_fdata[dst_idx], &image->fdata[src_idx], c * sizeof(float));
                }
            }
        }
        free(image->fdata);
        image->fdata = new_fdata;
    } else {
        uint8_t *new_data = (uint8_t *)calloc(image->size, 1);
        if (!new_data)
            return false;

        for (int y = 0; y < (int)h; ++y) {
            for (int x = 0; x < (int)w; ++x) {
                float fx = (float)x - cx;
                float fy = (float)y - cy;
                float rx = cos_a * fx - sin_a * fy + cx;
                float ry = sin_a * fx + cos_a * fy + cy;
                int irx = (int)(rx + 0.5f);
                int iry = (int)(ry + 0.5f);
                if (irx >= 0 && irx < (int)w && iry >= 0 && iry < (int)h) {
                    size_t src_idx = (iry * w + irx) * c;
                    size_t dst_idx = (y * w + x) * c;
                    memcpy(&new_data[dst_idx], &image->data[src_idx], c);
                }
            }
        }
        free(image->data);
        image->data = new_data;
    }
    // image->width, image->height, and image->size remain unchanged for in-place rotation
    return true;
}

bool fossil_image_process_blend(
    fossil_image_t *dst,
    const fossil_image_t *src,
    float ratio
) {
    if (!dst || !src)
        return false;

    // Check matching dimensions, channels, and format
    if (dst->width != src->width || dst->height != src->height ||
        dst->channels != src->channels || dst->format != src->format)
        return false;

    ratio = fmaxf(0.0f, fminf(1.0f, ratio));

    bool is_float = (
        dst->format == FOSSIL_PIXEL_FORMAT_FLOAT32 ||
        dst->format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGB ||
        dst->format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA
    );

    size_t npixels = (size_t)dst->width * dst->height * dst->channels;

    if (is_float) {
        if (!dst->fdata || !src->fdata)
            return false;
        for (size_t i = 0; i < npixels; ++i) {
            dst->fdata[i] = (1.0f - ratio) * dst->fdata[i] + ratio * src->fdata[i];
        }
    } else {
        if (!dst->data || !src->data)
            return false;
        size_t bytes_per_pixel = fossil_image_bytes_per_pixel(dst->format);
        // For 16-bit formats, blend as uint16_t
        if (dst->format == FOSSIL_PIXEL_FORMAT_GRAY16 ||
            dst->format == FOSSIL_PIXEL_FORMAT_RGB48 ||
            dst->format == FOSSIL_PIXEL_FORMAT_RGBA64) {
            uint16_t *d16 = (uint16_t *)dst->data;
            uint16_t *s16 = (uint16_t *)src->data;
            size_t n16 = npixels * (bytes_per_pixel / 2);
            for (size_t i = 0; i < n16; ++i) {
                float blended = (1.0f - ratio) * d16[i] + ratio * s16[i];
                if (blended < 0.0f) blended = 0.0f;
                if (blended > 65535.0f) blended = 65535.0f;
                d16[i] = (uint16_t)(blended + 0.5f);
            }
        } else {
            // 8-bit formats
            for (size_t i = 0; i < npixels; ++i) {
                float blended = (1.0f - ratio) * dst->data[i] + ratio * src->data[i];
                if (blended < 0.0f) blended = 0.0f;
                if (blended > 255.0f) blended = 255.0f;
                dst->data[i] = (uint8_t)(blended + 0.5f);
            }
        }
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
    if (!dst || !overlay)
        return false;

    // Ensure channel counts and formats match
    if (dst->channels != overlay->channels || dst->format != overlay->format)
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

    bool is_float = (
        dst->format == FOSSIL_PIXEL_FORMAT_FLOAT32 ||
        dst->format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGB ||
        dst->format == FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA
    );

    if (is_float) {
        if (!dst->fdata || !overlay->fdata)
            return false;
        for (uint32_t j = 0; j < h; ++j) {
            for (uint32_t i = 0; i < w; ++i) {
                uint32_t dx = x + i;
                uint32_t dy = y + j;
                size_t di = (dy * dst->width + dx) * c;
                size_t si = (j * overlay->width + i) * c;
                if (di + c > dst->size / sizeof(float) || si + c > overlay->size / sizeof(float))
                    continue;
                for (size_t k = 0; k < c; ++k) {
                    dst->fdata[di + k] = (1.0f - alpha) * dst->fdata[di + k] + alpha * overlay->fdata[si + k];
                }
            }
        }
    } else if (
        dst->format == FOSSIL_PIXEL_FORMAT_GRAY16 ||
        dst->format == FOSSIL_PIXEL_FORMAT_RGB48 ||
        dst->format == FOSSIL_PIXEL_FORMAT_RGBA64
    ) {
        if (!dst->data || !overlay->data)
            return false;
        uint16_t *d16 = (uint16_t *)dst->data;
        uint16_t *s16 = (uint16_t *)overlay->data;
        for (uint32_t j = 0; j < h; ++j) {
            for (uint32_t i = 0; i < w; ++i) {
                uint32_t dx = x + i;
                uint32_t dy = y + j;
                size_t di = (dy * dst->width + dx) * c;
                size_t si = (j * overlay->width + i) * c;
                if (di + c > dst->size / 2 || si + c > overlay->size / 2)
                    continue;
                for (size_t k = 0; k < c; ++k) {
                    float blended = (1.0f - alpha) * d16[di + k] + alpha * s16[si + k];
                    if (blended < 0.0f) blended = 0.0f;
                    if (blended > 65535.0f) blended = 65535.0f;
                    d16[di + k] = (uint16_t)(blended + 0.5f);
                }
            }
        }
    } else {
        if (!dst->data || !overlay->data)
            return false;
        for (uint32_t j = 0; j < h; ++j) {
            for (uint32_t i = 0; i < w; ++i) {
                uint32_t dx = x + i;
                uint32_t dy = y + j;
                size_t di = (dy * dst->width + dx) * c;
                size_t si = (j * overlay->width + i) * c;
                if (di + c > dst->size || si + c > overlay->size)
                    continue;
                for (size_t k = 0; k < c; ++k) {
                    float blended = (1.0f - alpha) * dst->data[di + k] + alpha * overlay->data[si + k];
                    if (blended < 0.0f) blended = 0.0f;
                    if (blended > 255.0f) blended = 255.0f;
                    dst->data[di + k] = (uint8_t)(blended + 0.5f);
                }
            }
        }
    }
    return true;
}

bool fossil_image_process_grayscale(fossil_image_t *image) {
    if (!image)
        return false;

    size_t npixels = (size_t)image->width * image->height;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_RGB24: {
            if (!image->data)
                return false;
            uint8_t *new_data = (uint8_t *)calloc(npixels, 1);
            if (!new_data)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                uint8_t r = image->data[i * 3 + 0];
                uint8_t g = image->data[i * 3 + 1];
                uint8_t b = image->data[i * 3 + 2];
                new_data[i] = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
            }
            free(image->data);
            image->data = new_data;
            image->channels = 1;
            image->format = FOSSIL_PIXEL_FORMAT_GRAY8;
            image->size = npixels;
            break;
        }
        case FOSSIL_PIXEL_FORMAT_RGBA32: {
            if (!image->data)
                return false;
            uint8_t *new_data = (uint8_t *)calloc(npixels, 1);
            if (!new_data)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                uint8_t r = image->data[i * 4 + 0];
                uint8_t g = image->data[i * 4 + 1];
                uint8_t b = image->data[i * 4 + 2];
                new_data[i] = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
            }
            free(image->data);
            image->data = new_data;
            image->channels = 1;
            image->format = FOSSIL_PIXEL_FORMAT_GRAY8;
            image->size = npixels;
            break;
        }
        case FOSSIL_PIXEL_FORMAT_RGB48: {
            if (!image->data)
                return false;
            uint16_t *src = (uint16_t *)image->data;
            uint16_t *new_data = (uint16_t *)calloc(npixels, sizeof(uint16_t));
            if (!new_data)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                uint16_t r = src[i * 3 + 0];
                uint16_t g = src[i * 3 + 1];
                uint16_t b = src[i * 3 + 2];
                new_data[i] = (uint16_t)(0.299 * r + 0.587 * g + 0.114 * b);
            }
            free(image->data);
            image->data = (uint8_t *)new_data;
            image->channels = 1;
            image->format = FOSSIL_PIXEL_FORMAT_GRAY16;
            image->size = npixels * sizeof(uint16_t);
            break;
        }
        case FOSSIL_PIXEL_FORMAT_RGBA64: {
            if (!image->data)
                return false;
            uint16_t *src = (uint16_t *)image->data;
            uint16_t *new_data = (uint16_t *)calloc(npixels, sizeof(uint16_t));
            if (!new_data)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                uint16_t r = src[i * 4 + 0];
                uint16_t g = src[i * 4 + 1];
                uint16_t b = src[i * 4 + 2];
                new_data[i] = (uint16_t)(0.299 * r + 0.587 * g + 0.114 * b);
            }
            free(image->data);
            image->data = (uint8_t *)new_data;
            image->channels = 1;
            image->format = FOSSIL_PIXEL_FORMAT_GRAY16;
            image->size = npixels * sizeof(uint16_t);
            break;
        }
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB: {
            if (!image->fdata)
                return false;
            float *new_fdata = (float *)calloc(npixels, sizeof(float));
            if (!new_fdata)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                float r = image->fdata[i * 3 + 0];
                float g = image->fdata[i * 3 + 1];
                float b = image->fdata[i * 3 + 2];
                new_fdata[i] = 0.299f * r + 0.587f * g + 0.114f * b;
            }
            free(image->fdata);
            image->fdata = new_fdata;
            image->channels = 1;
            image->format = FOSSIL_PIXEL_FORMAT_FLOAT32;
            image->size = npixels * sizeof(float);
            break;
        }
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA: {
            if (!image->fdata)
                return false;
            float *new_fdata = (float *)calloc(npixels, sizeof(float));
            if (!new_fdata)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                float r = image->fdata[i * 4 + 0];
                float g = image->fdata[i * 4 + 1];
                float b = image->fdata[i * 4 + 2];
                new_fdata[i] = 0.299f * r + 0.587f * g + 0.114f * b;
            }
            free(image->fdata);
            image->fdata = new_fdata;
            image->channels = 1;
            image->format = FOSSIL_PIXEL_FORMAT_FLOAT32;
            image->size = npixels * sizeof(float);
            break;
        }
        case FOSSIL_PIXEL_FORMAT_YUV24: {
            if (!image->data)
                return false;
            uint8_t *new_data = (uint8_t *)calloc(npixels, 1);
            if (!new_data)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                uint8_t y = image->data[i * 3 + 0];
                new_data[i] = y;
            }
            free(image->data);
            image->data = new_data;
            image->channels = 1;
            image->format = FOSSIL_PIXEL_FORMAT_GRAY8;
            image->size = npixels;
            break;
        }
        default:
            // Already grayscale or unsupported format
            return false;
    }
    return true;
}

bool fossil_image_process_threshold(fossil_image_t *image, uint8_t threshold) {
    if (!image)
        return false;

    size_t npixels = (size_t)image->width * image->height * image->channels;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            if (!image->data)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                image->data[i] = (image->data[i] >= threshold) ? 255 : 0;
            }
            break;
        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            if (!image->data)
                return false;
            {
                uint16_t *data16 = (uint16_t *)image->data;
                for (size_t i = 0; i < npixels; ++i) {
                    data16[i] = (data16[i] >= (uint16_t)threshold * 257) ? 65535 : 0;
                }
            }
            break;
        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            if (!image->fdata)
                return false;
            {
                float t = (float)threshold / 255.0f;
                for (size_t i = 0; i < npixels; ++i) {
                    image->fdata[i] = (image->fdata[i] >= t) ? 1.0f : 0.0f;
                }
            }
            break;
        default:
            return false;
    }
    return true;
}

bool fossil_image_process_invert(fossil_image_t *image) {
    if (!image)
        return false;

    size_t npixels = (size_t)image->width * image->height * image->channels;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            if (!image->data)
                return false;
            for (size_t i = 0; i < npixels; ++i)
                image->data[i] = (uint8_t)(255U - image->data[i]);
            break;
        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            if (!image->data)
                return false;
            {
                uint16_t *data16 = (uint16_t *)image->data;
                for (size_t i = 0; i < npixels; ++i)
                    data16[i] = (uint16_t)(65535U - data16[i]);
            }
            break;
        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            if (!image->fdata)
                return false;
            for (size_t i = 0; i < npixels; ++i)
                image->fdata[i] = 1.0f - image->fdata[i];
            break;
        default:
            return false;
    }
    return true;
}

bool fossil_image_process_normalize(fossil_image_t *image) {
    if (!image)
        return false;

    size_t npixels = (size_t)image->width * image->height * image->channels;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24: {
            if (!image->data)
                return false;
            uint8_t min_val = 255, max_val = 0;
            for (size_t i = 0; i < npixels; ++i) {
                if (image->data[i] < min_val) min_val = image->data[i];
                if (image->data[i] > max_val) max_val = image->data[i];
            }
            if (max_val == min_val)
                return true;
            float scale = 255.0f / (float)(max_val - min_val);
            for (size_t i = 0; i < npixels; ++i) {
                float normalized = ((float)image->data[i] - (float)min_val) * scale;
                if (normalized < 0.0f) normalized = 0.0f;
                if (normalized > 255.0f) normalized = 255.0f;
                image->data[i] = (uint8_t)(normalized + 0.5f);
            }
            break;
        }
        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64: {
            if (!image->data)
                return false;
            uint16_t *data16 = (uint16_t *)image->data;
            uint16_t min_val = 65535, max_val = 0;
            for (size_t i = 0; i < npixels; ++i) {
                if (data16[i] < min_val) min_val = data16[i];
                if (data16[i] > max_val) max_val = data16[i];
            }
            if (max_val == min_val)
                return true;
            float scale = 65535.0f / (float)(max_val - min_val);
            for (size_t i = 0; i < npixels; ++i) {
                float normalized = ((float)data16[i] - (float)min_val) * scale;
                if (normalized < 0.0f) normalized = 0.0f;
                if (normalized > 65535.0f) normalized = 65535.0f;
                data16[i] = (uint16_t)(normalized + 0.5f);
            }
            break;
        }
        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA: {
            if (!image->fdata)
                return false;
            float min_val = image->fdata[0], max_val = image->fdata[0];
            for (size_t i = 1; i < npixels; ++i) {
                if (image->fdata[i] < min_val) min_val = image->fdata[i];
                if (image->fdata[i] > max_val) max_val = image->fdata[i];
            }
            if (max_val == min_val)
                return true;
            float scale = 1.0f / (max_val - min_val);
            for (size_t i = 0; i < npixels; ++i) {
                float normalized = (image->fdata[i] - min_val) * scale;
                if (normalized < 0.0f) normalized = 0.0f;
                if (normalized > 1.0f) normalized = 1.0f;
                image->fdata[i] = normalized;
            }
            break;
        }
        default:
            return false;
    }
    return true;
}
