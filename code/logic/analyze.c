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
    if (!image || !out_hist)
        return false;

    size_t bins = 256 * image->channels;
    memset(out_hist, 0, bins * sizeof(uint32_t));

    size_t pixels = (size_t)image->width * image->height;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            if (!image->data)
                return false;
            for (size_t i = 0; i < pixels; ++i) {
                for (uint32_t c = 0; c < image->channels; ++c) {
                    uint8_t val = image->data[i * image->channels + c];
                    out_hist[c * 256 + val]++;
                }
            }
            break;

        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            if (!image->data)
                return false;
            for (size_t i = 0; i < pixels; ++i) {
                for (uint32_t c = 0; c < image->channels; ++c) {
                    uint16_t *idata = (uint16_t *)image->data;
                    uint16_t val = idata[i * image->channels + c];
                    uint8_t bin = (uint8_t)(val >> 8); // Map 16-bit to 8-bit bin
                    out_hist[c * 256 + bin]++;
                }
            }
            break;

        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            if (!image->fdata)
                return false;
            for (size_t i = 0; i < pixels; ++i) {
                for (uint32_t c = 0; c < image->channels; ++c) {
                    float val = image->fdata[i * image->channels + c];
                    uint8_t bin = (uint8_t)(fmaxf(0.0f, fminf(val, 1.0f)) * 255.0f);
                    out_hist[c * 256 + bin]++;
                }
            }
            break;

        default:
            return false;
    }

    return true;
}

bool fossil_image_analyze_mean_stddev(const fossil_image_t *image, double *mean, double *stddev) {
    if (!image || !mean || !stddev)
        return false;

    size_t npixels = (size_t)image->width * image->height;
    uint32_t ch = image->channels;

    // Handle different pixel formats
    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            if (!image->data)
                return false;
            for (uint32_t c = 0; c < ch; ++c) {
                double sum = 0.0, sum_sq = 0.0;
                for (size_t i = 0; i < npixels; ++i) {
                    double v = (double)image->data[i * ch + c];
                    sum += v;
                    sum_sq += v * v;
                }
                mean[c] = sum / npixels;
                double variance = (sum_sq / npixels) - (mean[c] * mean[c]);
                stddev[c] = sqrt(fmax(variance, 0.0));
            }
            break;

        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            if (!image->data)
                return false;
            for (uint32_t c = 0; c < ch; ++c) {
                double sum = 0.0, sum_sq = 0.0;
                uint16_t *idata = (uint16_t *)image->data;
                for (size_t i = 0; i < npixels; ++i) {
                    double v = (double)idata[i * ch + c];
                    sum += v;
                    sum_sq += v * v;
                }
                mean[c] = sum / npixels;
                double variance = (sum_sq / npixels) - (mean[c] * mean[c]);
                stddev[c] = sqrt(fmax(variance, 0.0));
            }
            break;

        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            if (!image->fdata)
                return false;
            for (uint32_t c = 0; c < ch; ++c) {
                double sum = 0.0, sum_sq = 0.0;
                for (size_t i = 0; i < npixels; ++i) {
                    double v = (double)image->fdata[i * ch + c];
                    sum += v;
                    sum_sq += v * v;
                }
                mean[c] = sum / npixels;
                double variance = (sum_sq / npixels) - (mean[c] * mean[c]);
                stddev[c] = sqrt(fmax(variance, 0.0));
            }
            break;

        default:
            return false;
    }

    return true;
}

bool fossil_image_analyze_brightness(const fossil_image_t *image, double *out_brightness) {
    if (!image || !out_brightness)
        return false;

    size_t npixels = (size_t)image->width * image->height;
    double total = 0.0;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            if (!image->data)
                return false;
            for (size_t i = 0; i < npixels; ++i)
                total += image->data[i];
            *out_brightness = total / (npixels * 255.0);
            break;

        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
            if (!image->data)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                uint8_t r = image->data[i * image->channels + 0];
                uint8_t g = image->data[i * image->channels + 1];
                uint8_t b = image->data[i * image->channels + 2];
                total += 0.299 * r + 0.587 * g + 0.114 * b;
            }
            *out_brightness = total / (npixels * 255.0);
            break;

        case FOSSIL_PIXEL_FORMAT_GRAY16:
            if (!image->data)
                return false;
            {
                uint16_t *idata = (uint16_t *)image->data;
                for (size_t i = 0; i < npixels; ++i)
                    total += idata[i];
                *out_brightness = total / (npixels * 65535.0);
            }
            break;

        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            if (!image->data)
                return false;
            {
                uint16_t *idata = (uint16_t *)image->data;
                for (size_t i = 0; i < npixels; ++i) {
                    uint16_t r = idata[i * image->channels + 0];
                    uint16_t g = idata[i * image->channels + 1];
                    uint16_t b = idata[i * image->channels + 2];
                    total += 0.299 * r + 0.587 * g + 0.114 * b;
                }
                *out_brightness = total / (npixels * 65535.0);
            }
            break;

        case FOSSIL_PIXEL_FORMAT_FLOAT32:
            if (!image->fdata)
                return false;
            for (size_t i = 0; i < npixels; ++i)
                total += image->fdata[i];
            *out_brightness = total / npixels;
            break;

        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            if (!image->fdata)
                return false;
            for (size_t i = 0; i < npixels; ++i) {
                float r = image->fdata[i * image->channels + 0];
                float g = image->fdata[i * image->channels + 1];
                float b = image->fdata[i * image->channels + 2];
                total += 0.299f * r + 0.587f * g + 0.114f * b;
            }
            *out_brightness = total / npixels;
            break;

        default:
            return false;
    }

    return true;
}

bool fossil_image_analyze_contrast(const fossil_image_t *image, double *out_contrast) {
    if (!image || !out_contrast)
        return false;

    double mean[4] = {0}, stddev[4] = {0};
    bool ok = false;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            if (!image->data)
                return false;
            ok = fossil_image_analyze_mean_stddev(image, mean, stddev);
            if (!ok) return false;
            {
                double total = 0.0;
                for (uint32_t c = 0; c < image->channels; ++c)
                    total += stddev[c];
                *out_contrast = (total / image->channels) / 255.0;
            }
            break;

        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            if (!image->data)
                return false;
            ok = fossil_image_analyze_mean_stddev(image, mean, stddev);
            if (!ok) return false;
            {
                double total = 0.0;
                for (uint32_t c = 0; c < image->channels; ++c)
                    total += stddev[c];
                *out_contrast = (total / image->channels) / 65535.0;
            }
            break;

        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            if (!image->fdata)
                return false;
            ok = fossil_image_analyze_mean_stddev(image, mean, stddev);
            if (!ok) return false;
            {
                double total = 0.0;
                for (uint32_t c = 0; c < image->channels; ++c)
                    total += stddev[c];
                *out_contrast = total / image->channels;
            }
            break;

        default:
            return false;
    }

    return true;
}

bool fossil_image_analyze_edge_sobel(const fossil_image_t *src, fossil_image_t *dst) {
    if (!src || !dst)
        return false;

    uint32_t w = src->width, h = src->height;
    if (w < 3 || h < 3)
        return false;

    dst->width = w;
    dst->height = h;
    dst->channels = 1;
    dst->format = FOSSIL_PIXEL_FORMAT_GRAY8;
    dst->size = (size_t)w * h;
    dst->data = (uint8_t *)calloc(dst->size, 1);
    dst->owns_data = true;

    if (!dst->data)
        return false;

    const int gx[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
    const int gy[3][3] = { {-1,-2,-1}, { 0, 0, 0}, { 1, 2, 1} };

    for (uint32_t y = 1; y < h - 1; ++y) {
        for (uint32_t x = 1; x < w - 1; ++x) {
            float sumx = 0, sumy = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    float pixel = 0.0f;
                    size_t idx = ((y + ky) * w + (x + kx));
                    switch (src->format) {
                        case FOSSIL_PIXEL_FORMAT_GRAY8:
                        case FOSSIL_PIXEL_FORMAT_INDEXED8:
                        case FOSSIL_PIXEL_FORMAT_YUV24:
                            pixel = (float)src->data[idx];
                            break;
                        case FOSSIL_PIXEL_FORMAT_RGB24:
                        case FOSSIL_PIXEL_FORMAT_RGBA32: {
                            uint8_t r = src->data[idx * src->channels + 0];
                            uint8_t g = src->data[idx * src->channels + 1];
                            uint8_t b = src->data[idx * src->channels + 2];
                            pixel = 0.299f * r + 0.587f * g + 0.114f * b;
                            break;
                        }
                        case FOSSIL_PIXEL_FORMAT_GRAY16: {
                            uint16_t *idata = (uint16_t *)src->data;
                            pixel = (float)(idata[idx]) / 65535.0f * 255.0f;
                            break;
                        }
                        case FOSSIL_PIXEL_FORMAT_RGB48:
                        case FOSSIL_PIXEL_FORMAT_RGBA64: {
                            uint16_t *idata = (uint16_t *)src->data;
                            uint16_t r = idata[idx * src->channels + 0];
                            uint16_t g = idata[idx * src->channels + 1];
                            uint16_t b = idata[idx * src->channels + 2];
                            pixel = (0.299f * r + 0.587f * g + 0.114f * b) / 65535.0f * 255.0f;
                            break;
                        }
                        case FOSSIL_PIXEL_FORMAT_FLOAT32: {
                            pixel = fmaxf(0.0f, fminf(src->fdata[idx], 1.0f)) * 255.0f;
                            break;
                        }
                        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
                        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA: {
                            float r = src->fdata[idx * src->channels + 0];
                            float g = src->fdata[idx * src->channels + 1];
                            float b = src->fdata[idx * src->channels + 2];
                            pixel = (0.299f * r + 0.587f * g + 0.114f * b) * 255.0f;
                            break;
                        }
                        default:
                            pixel = 0.0f;
                            break;
                    }
                    sumx += gx[ky + 1][kx + 1] * pixel;
                    sumy += gy[ky + 1][kx + 1] * pixel;
                }
            }
            float mag = sqrtf(sumx * sumx + sumy * sumy);
            dst->data[y * w + x] = (uint8_t)fminf(mag, 255.0f);
        }
    }

    return true;
}

bool fossil_image_analyze_entropy(const fossil_image_t *image, double *out_entropy) {
    if (!image || !out_entropy)
        return false;

    size_t npixels = (size_t)image->width * image->height;
    uint32_t bins = 256;
    uint32_t hist[1024] = {0}; // Support up to 4 channels

    // Select proper histogram source and bins
    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            if (!image->data)
                return false;
            bins = 256;
            for (size_t i = 0; i < npixels; ++i)
                hist[image->data[i]]++;
            break;

        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
            if (!image->data)
                return false;
            bins = 256 * image->channels;
            for (size_t i = 0; i < npixels; ++i)
                for (uint32_t c = 0; c < image->channels; ++c)
                    hist[c * 256 + image->data[i * image->channels + c]]++;
            break;

        case FOSSIL_PIXEL_FORMAT_GRAY16:
            if (!image->data)
                return false;
            bins = 256;
            {
                uint16_t *idata = (uint16_t *)image->data;
                for (size_t i = 0; i < npixels; ++i)
                    hist[idata[i] >> 8]++;
            }
            break;

        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            if (!image->data)
                return false;
            bins = 256 * image->channels;
            {
                uint16_t *idata = (uint16_t *)image->data;
                for (size_t i = 0; i < npixels; ++i)
                    for (uint32_t c = 0; c < image->channels; ++c)
                        hist[c * 256 + (idata[i * image->channels + c] >> 8)]++;
            }
            break;

        case FOSSIL_PIXEL_FORMAT_FLOAT32:
            if (!image->fdata)
                return false;
            bins = 256;
            for (size_t i = 0; i < npixels; ++i) {
                float val = fmaxf(0.0f, fminf(image->fdata[i], 1.0f));
                hist[(uint8_t)(val * 255.0f)]++;
            }
            break;

        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            if (!image->fdata)
                return false;
            bins = 256 * image->channels;
            for (size_t i = 0; i < npixels; ++i)
                for (uint32_t c = 0; c < image->channels; ++c) {
                    float val = fmaxf(0.0f, fminf(image->fdata[i * image->channels + c], 1.0f));
                    hist[c * 256 + (uint8_t)(val * 255.0f)]++;
                }
            break;

        default:
            return false;
    }

    double entropy = 0.0;
    size_t total = npixels * ((bins > 256) ? image->channels : 1);
    for (uint32_t i = 0; i < bins; ++i) {
        if (hist[i] == 0) continue;
        double p = (double)hist[i] / total;
        entropy -= p * log2(p);
    }

    *out_entropy = entropy;
    return true;
}
