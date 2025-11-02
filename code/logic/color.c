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
#include "fossil/image/color.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// ======================================================
// Fossil Image — Color Sub-Library Implementation
// ======================================================

static inline uint8_t clamp8(float v) {
    if (v < 0.0f) return 0;
    if (v > 255.0f) return 255;
    return (uint8_t)v;
}

bool fossil_image_color_brightness(fossil_image_t *image, int offset) {
    if (!image)
        return false;

    size_t pixels = image->width * image->height * image->channels;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
        {
            if (!image->data) return false;
            for (size_t i = 0; i < pixels; ++i) {
                int val = (int)image->data[i] + offset;
                image->data[i] = (uint8_t)(val < 0 ? 0 : (val > 255 ? 255 : val));
            }
            break;
        }
        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
        {
            if (!image->data) return false;
            uint16_t *data16 = (uint16_t *)image->data;
            for (size_t i = 0; i < pixels; ++i) {
                int val = (int)data16[i] + offset;
                data16[i] = (uint16_t)(val < 0 ? 0 : (val > 65535 ? 65535 : val));
            }
            break;
        }
        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
        {
            if (!image->fdata) return false;
            for (size_t i = 0; i < pixels; ++i) {
                image->fdata[i] += (float)offset;
            }
            break;
        }
        default:
            return false;
    }
    return true;
}

bool fossil_image_color_contrast(fossil_image_t *image, float factor) {
    if (!image)
        return false;

    size_t pixels = image->width * image->height * image->channels;
    float midpoint = 128.0f;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
        {
            if (!image->data) return false;
            for (size_t i = 0; i < pixels; ++i) {
                float val = ((float)image->data[i] - midpoint) * factor + midpoint;
                image->data[i] = clamp8(val);
            }
            break;
        }
        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
        {
            if (!image->data) return false;
            uint16_t *data16 = (uint16_t *)image->data;
            float midpoint16 = 32768.0f;
            for (size_t i = 0; i < pixels; ++i) {
                float val = ((float)data16[i] - midpoint16) * factor + midpoint16;
                data16[i] = (uint16_t)(val < 0 ? 0 : (val > 65535 ? 65535 : val));
            }
            break;
        }
        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
        {
            if (!image->fdata) return false;
            float midpointf = 0.5f;
            for (size_t i = 0; i < pixels; ++i) {
                image->fdata[i] = ((image->fdata[i] - midpointf) * factor) + midpointf;
            }
            break;
        }
        default:
            return false;
    }
    return true;
}

bool fossil_image_color_gamma(fossil_image_t *image, float gamma) {
    if (!image || gamma <= 0.0f)
        return false;

    size_t pixels = image->width * image->height * image->channels;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
        {
            if (!image->data) return false;
            uint8_t gamma_lut[256];
            float inv_gamma = 1.0f / gamma;
            for (int i = 0; i < 256; ++i)
                gamma_lut[i] = clamp8(powf(i / 255.0f, inv_gamma) * 255.0f);

            for (size_t i = 0; i < pixels; ++i)
                image->data[i] = gamma_lut[image->data[i]];
            break;
        }
        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
        {
            if (!image->data) return false;
            uint16_t *data16 = (uint16_t *)image->data;
            float inv_gamma = 1.0f / gamma;
            for (size_t i = 0; i < pixels; ++i) {
                float norm = data16[i] / 65535.0f;
                data16[i] = (uint16_t)(clamp8(powf(norm, inv_gamma) * 65535.0f));
            }
            break;
        }
        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
        {
            if (!image->fdata) return false;
            float inv_gamma = 1.0f / gamma;
            for (size_t i = 0; i < pixels; ++i) {
                image->fdata[i] = powf(image->fdata[i], inv_gamma);
            }
            break;
        }
        default:
            return false;
    }
    return true;
}

static void rgb_to_hsv(float r, float g, float b, float *h, float *s, float *v) {
    float max = fmaxf(r, fmaxf(g, b));
    float min = fminf(r, fminf(g, b));
    float delta = max - min;

    *v = max;
    *s = (max == 0) ? 0 : (delta / max);

    if (delta == 0) {
        *h = 0;
    } else if (max == r) {
        *h = 60.0f * fmodf(((g - b) / delta), 6.0f);
    } else if (max == g) {
        *h = 60.0f * (((b - r) / delta) + 2.0f);
    } else {
        *h = 60.0f * (((r - g) / delta) + 4.0f);
    }

    if (*h < 0) *h += 360.0f;
}

static void hsv_to_rgb(float h, float s, float v, float *r, float *g, float *b) {
    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r1, g1, b1;
    if      (h < 60)  { r1 = c; g1 = x; b1 = 0; }
    else if (h < 120) { r1 = x; g1 = c; b1 = 0; }
    else if (h < 180) { r1 = 0; g1 = c; b1 = x; }
    else if (h < 240) { r1 = 0; g1 = x; b1 = c; }
    else if (h < 300) { r1 = x; g1 = 0; b1 = c; }
    else              { r1 = c; g1 = 0; b1 = x; }

    *r = (r1 + m);
    *g = (g1 + m);
    *b = (b1 + m);
}

bool fossil_image_color_hsv_adjust(
    fossil_image_t *image,
    float hue_shift,
    float sat_mult,
    float val_mult
) {
    if (!image)
        return false;

    size_t npixels = (size_t)image->width * image->height;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_YUV24:
        {
            if (!image->data || image->channels < 3)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                float r = image->data[i * image->channels + 0] / 255.0f;
                float g = image->data[i * image->channels + 1] / 255.0f;
                float b = image->data[i * image->channels + 2] / 255.0f;

                float h, s, v;
                rgb_to_hsv(r, g, b, &h, &s, &v);
                h += hue_shift;
                if (h < 0) h += 360.0f;
                if (h >= 360.0f) h -= 360.0f;

                s = fminf(fmaxf(s * sat_mult, 0.0f), 1.0f);
                v = fminf(fmaxf(v * val_mult, 0.0f), 1.0f);

                hsv_to_rgb(h, s, v, &r, &g, &b);

                image->data[i * image->channels + 0] = clamp8(r * 255.0f);
                image->data[i * image->channels + 1] = clamp8(g * 255.0f);
                image->data[i * image->channels + 2] = clamp8(b * 255.0f);
            }
            break;
        }
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
        {
            if (!image->data || image->channels < 3)
                return false;
            uint16_t *data16 = (uint16_t *)image->data;
            for (size_t i = 0; i < npixels; ++i) {
                float r = data16[i * image->channels + 0] / 65535.0f;
                float g = data16[i * image->channels + 1] / 65535.0f;
                float b = data16[i * image->channels + 2] / 65535.0f;

                float h, s, v;
                rgb_to_hsv(r, g, b, &h, &s, &v);
                h += hue_shift;
                if (h < 0) h += 360.0f;
                if (h >= 360.0f) h -= 360.0f;

                s = fminf(fmaxf(s * sat_mult, 0.0f), 1.0f);
                v = fminf(fmaxf(v * val_mult, 0.0f), 1.0f);

                hsv_to_rgb(h, s, v, &r, &g, &b);

                data16[i * image->channels + 0] = (uint16_t)(fminf(fmaxf(r * 65535.0f, 0.0f), 65535.0f));
                data16[i * image->channels + 1] = (uint16_t)(fminf(fmaxf(g * 65535.0f, 0.0f), 65535.0f));
                data16[i * image->channels + 2] = (uint16_t)(fminf(fmaxf(b * 65535.0f, 0.0f), 65535.0f));
            }
            break;
        }
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
        {
            if (!image->fdata || image->channels < 3)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                float r = image->fdata[i * image->channels + 0];
                float g = image->fdata[i * image->channels + 1];
                float b = image->fdata[i * image->channels + 2];

                float h, s, v;
                rgb_to_hsv(r, g, b, &h, &s, &v);
                h += hue_shift;
                if (h < 0) h += 360.0f;
                if (h >= 360.0f) h -= 360.0f;

                s = fminf(fmaxf(s * sat_mult, 0.0f), 1.0f);
                v = fminf(fmaxf(v * val_mult, 0.0f), 1.0f);

                hsv_to_rgb(h, s, v, &r, &g, &b);

                image->fdata[i * image->channels + 0] = r;
                image->fdata[i * image->channels + 1] = g;
                image->fdata[i * image->channels + 2] = b;
            }
            break;
        }
        default:
            return false;
    }

    return true;
}

bool fossil_image_color_channel_swap(
    fossil_image_t *image,
    uint32_t ch_a,
    uint32_t ch_b
) {
    if (!image)
        return false;

    if (ch_a >= image->channels || ch_b >= image->channels)
        return false;

    size_t npixels = (size_t)image->width * image->height;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
        {
            if (!image->data)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                size_t idx = i * image->channels;
                uint8_t tmp = image->data[idx + ch_a];
                image->data[idx + ch_a] = image->data[idx + ch_b];
                image->data[idx + ch_b] = tmp;
            }
            break;
        }
        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
        {
            if (!image->data)
                return false;
            uint16_t *data16 = (uint16_t *)image->data;
            for (size_t i = 0; i < npixels; ++i) {
                size_t idx = i * image->channels;
                uint16_t tmp = data16[idx + ch_a];
                data16[idx + ch_a] = data16[idx + ch_b];
                data16[idx + ch_b] = tmp;
            }
            break;
        }
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        {
            if (!image->fdata)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                size_t idx = i * image->channels;
                float tmp = image->fdata[idx + ch_a];
                image->fdata[idx + ch_a] = image->fdata[idx + ch_b];
                image->fdata[idx + ch_b] = tmp;
            }
            break;
        }
        default:
            return false;
    }
    return true;
}

bool fossil_image_color_to_grayscale(fossil_image_t *image) {
    if (!image)
        return false;

    size_t npixels = (size_t)image->width * image->height;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_YUV24:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        {
            if (!image->data || image->channels < 3)
                return false;
            uint8_t *new_data = (uint8_t *)calloc(npixels, 1);
            if (!new_data)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                uint8_t r = image->data[i * image->channels + 0];
                uint8_t g = image->data[i * image->channels + 1];
                uint8_t b = image->data[i * image->channels + 2];
                new_data[i] = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
            }
            if (image->owns_data)
                free(image->data);
            image->data = new_data;
            image->channels = 1;
            image->format = FOSSIL_PIXEL_FORMAT_GRAY8;
            image->size = npixels;
            image->owns_data = true;
            return true;
        }
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
        {
            if (!image->data || image->channels < 3)
                return false;
            uint16_t *data16 = (uint16_t *)image->data;
            uint16_t *new_data = (uint16_t *)calloc(npixels, sizeof(uint16_t));
            if (!new_data)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                uint16_t r = data16[i * image->channels + 0];
                uint16_t g = data16[i * image->channels + 1];
                uint16_t b = data16[i * image->channels + 2];
                new_data[i] = (uint16_t)(0.299f * r + 0.587f * g + 0.114f * b);
            }
            if (image->owns_data)
                free(image->data);
            image->data = (uint8_t *)new_data;
            image->channels = 1;
            image->format = FOSSIL_PIXEL_FORMAT_GRAY16;
            image->size = npixels * sizeof(uint16_t);
            image->owns_data = true;
            return true;
        }
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
        {
            if (!image->fdata || image->channels < 3)
                return false;
            float *new_data = (float *)calloc(npixels, sizeof(float));
            if (!new_data)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                float r = image->fdata[i * image->channels + 0];
                float g = image->fdata[i * image->channels + 1];
                float b = image->fdata[i * image->channels + 2];
                new_data[i] = 0.299f * r + 0.587f * g + 0.114f * b;
            }
            if (image->owns_data)
                free(image->fdata);
            image->fdata = new_data;
            image->channels = 1;
            image->format = FOSSIL_PIXEL_FORMAT_FLOAT32;
            image->size = npixels * sizeof(float);
            image->owns_data = true;
            return true;
        }
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_FLOAT32:
            // Already grayscale
            return true;
        default:
            return false;
    }
}
