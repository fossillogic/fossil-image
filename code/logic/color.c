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
    if (!image || !image->data)
        return false;

    size_t pixels = image->width * image->height * image->channels;
    for (size_t i = 0; i < pixels; ++i) {
        int val = (int)image->data[i] + offset;
        image->data[i] = (uint8_t)(val < 0 ? 0 : (val > 255 ? 255 : val));
    }
    return true;
}

bool fossil_image_color_contrast(fossil_image_t *image, float factor) {
    if (!image || !image->data)
        return false;

    float midpoint = 128.0f;
    size_t pixels = image->width * image->height * image->channels;
    for (size_t i = 0; i < pixels; ++i) {
        float val = ((float)image->data[i] - midpoint) * factor + midpoint;
        image->data[i] = clamp8(val);
    }
    return true;
}

bool fossil_image_color_gamma(fossil_image_t *image, float gamma) {
    if (!image || !image->data || gamma <= 0.0f)
        return false;

    uint8_t gamma_lut[256];
    float inv_gamma = 1.0f / gamma;
    for (int i = 0; i < 256; ++i)
        gamma_lut[i] = clamp8(powf(i / 255.0f, inv_gamma) * 255.0f);

    size_t pixels = image->width * image->height * image->channels;
    for (size_t i = 0; i < pixels; ++i)
        image->data[i] = gamma_lut[image->data[i]];

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
    if (!image || !image->data || image->channels < 3)
        return false;

    size_t npixels = (size_t)image->width * image->height;
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

    return true;
}

bool fossil_image_color_channel_swap(
    fossil_image_t *image,
    uint32_t ch_a,
    uint32_t ch_b
) {
    if (!image || !image->data)
        return false;

    if (ch_a >= image->channels || ch_b >= image->channels)
        return false;

    size_t npixels = (size_t)image->width * image->height;
    for (size_t i = 0; i < npixels; ++i) {
        size_t idx = i * image->channels;
        uint8_t tmp = image->data[idx + ch_a];
        image->data[idx + ch_a] = image->data[idx + ch_b];
        image->data[idx + ch_b] = tmp;
    }
    return true;
}

bool fossil_image_color_to_grayscale(fossil_image_t *image) {
    if (!image || !image->data)
        return false;

    if (image->channels < 3)
        return true;

    size_t npixels = (size_t)image->width * image->height;
    uint8_t *new_data = (uint8_t *)calloc(npixels, 1);
    if (!new_data)
        return false;

    for (size_t i = 0; i < npixels; ++i) {
        uint8_t r = image->data[i * image->channels + 0];
        uint8_t g = image->data[i * image->channels + 1];
        uint8_t b = image->data[i * image->channels + 2];
        new_data[i] = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
    }

    free(image->data);
    image->data = new_data;
    image->channels = 1;
    image->format = FOSSIL_PIXEL_FORMAT_GRAY8;
    image->size = npixels;
    return true;
}
