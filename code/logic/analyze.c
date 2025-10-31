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
#include "fossil/image/analyze.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// ======================================================
// Fossil Image — Analyze Sub-Library Implementation
// ======================================================

bool fossil_image_analyze_histogram(const fossil_image_t *image, uint32_t *out_hist) {
    if (!image || !image->data || !out_hist)
        return false;

    size_t bins = 256 * image->channels;
    memset(out_hist, 0, bins * sizeof(uint32_t));

    size_t pixels = (size_t)image->width * image->height;
    for (size_t i = 0; i < pixels; ++i) {
        for (uint32_t c = 0; c < image->channels; ++c) {
            uint8_t val = image->data[i * image->channels + c];
            out_hist[c * 256 + val]++;
        }
    }

    return true;
}

bool fossil_image_analyze_mean_stddev(const fossil_image_t *image, double *mean, double *stddev) {
    if (!image || !image->data || !mean || !stddev)
        return false;

    size_t npixels = (size_t)image->width * image->height;
    uint32_t ch = image->channels;

    for (uint32_t c = 0; c < ch; ++c) {
        double sum = 0.0;
        double sum_sq = 0.0;
        for (size_t i = 0; i < npixels; ++i) {
            double v = (double)image->data[i * ch + c];
            sum += v;
            sum_sq += v * v;
        }
        mean[c] = sum / npixels;
        double variance = (sum_sq / npixels) - (mean[c] * mean[c]);
        stddev[c] = sqrt(fmax(variance, 0.0));
    }
    return true;
}

bool fossil_image_analyze_brightness(const fossil_image_t *image, double *out_brightness) {
    if (!image || !image->data || !out_brightness)
        return false;

    size_t npixels = (size_t)image->width * image->height;
    double total = 0.0;

    if (image->channels == 1) {
        for (size_t i = 0; i < npixels; ++i)
            total += image->data[i];
    } else {
        for (size_t i = 0; i < npixels; ++i) {
            uint8_t r = image->data[i * image->channels + 0];
            uint8_t g = image->data[i * image->channels + 1];
            uint8_t b = image->data[i * image->channels + 2];
            total += 0.299 * r + 0.587 * g + 0.114 * b;
        }
    }

    *out_brightness = (total / (npixels * 255.0));
    return true;
}

bool fossil_image_analyze_contrast(const fossil_image_t *image, double *out_contrast) {
    if (!image || !image->data || !out_contrast)
        return false;

    double mean[4] = {0}, stddev[4] = {0};
    if (!fossil_image_analyze_mean_stddev(image, mean, stddev))
        return false;

    double total = 0.0;
    for (uint32_t c = 0; c < image->channels; ++c)
        total += stddev[c];

    *out_contrast = (total / image->channels) / 255.0;
    return true;
}

bool fossil_image_analyze_edge_sobel(const fossil_image_t *src, fossil_image_t *dst) {
    if (!src || !src->data || !dst)
        return false;

    if (src->channels < 1)
        return false;

    dst->width = src->width;
    dst->height = src->height;
    dst->channels = 1;
    dst->format = FOSSIL_PIXEL_FORMAT_GRAY8;
    dst->size = (size_t)dst->width * dst->height;
    dst->data = (uint8_t *)calloc(dst->size, 1);
    dst->owns_data = true;

    if (!dst->data)
        return false;

    const int gx[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
    const int gy[3][3] = { {-1,-2,-1}, { 0, 0, 0}, { 1, 2, 1} };

    for (uint32_t y = 1; y < src->height - 1; ++y) {
        for (uint32_t x = 1; x < src->width - 1; ++x) {
            float sumx = 0, sumy = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    uint8_t pixel;
                    if (src->channels == 1) {
                        pixel = src->data[(y + ky) * src->width + (x + kx)];
                    } else {
                        uint8_t r = src->data[((y + ky) * src->width + (x + kx)) * src->channels + 0];
                        uint8_t g = src->data[((y + ky) * src->width + (x + kx)) * src->channels + 1];
                        uint8_t b = src->data[((y + ky) * src->width + (x + kx)) * src->channels + 2];
                        pixel = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
                    }
                    sumx += gx[ky + 1][kx + 1] * pixel;
                    sumy += gy[ky + 1][kx + 1] * pixel;
                }
            }
            float mag = sqrtf(sumx * sumx + sumy * sumy);
            dst->data[y * dst->width + x] = (uint8_t)fminf(mag, 255.0f);
        }
    }

    return true;
}

bool fossil_image_analyze_entropy(const fossil_image_t *image, double *out_entropy) {
    if (!image || !image->data || !out_entropy)
        return false;

    uint32_t hist[256] = {0};
    if (!fossil_image_analyze_histogram(image, hist))
        return false;

    size_t npixels = (size_t)image->width * image->height;
    double entropy = 0.0;
    for (int i = 0; i < 256; ++i) {
        if (hist[i] == 0) continue;
        double p = (double)hist[i] / npixels;
        entropy -= p * log2(p);
    }

    *out_entropy = entropy;
    return true;
}
