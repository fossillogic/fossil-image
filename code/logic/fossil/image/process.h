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
#ifndef FOSSIL_IMAGE_PROFESSES_H
#define FOSSIL_IMAGE_PROFESSES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// Fossil Image — Process Sub-Library
// ======================================================
//
// Provides pixel-level and high-level image transformations.
// Functions are designed to work with fossil_image_t objects.
//
// ======================================================

// ------------------------------------------
// ENUMS AND DEFINITIONS
// ------------------------------------------

typedef enum fossil_image_filter_e {
    FOSSIL_IMAGE_FILTER_NONE = 0,
    FOSSIL_IMAGE_FILTER_GRAYSCALE,
    FOSSIL_IMAGE_FILTER_SEPIA,
    FOSSIL_IMAGE_FILTER_NEGATIVE,
    FOSSIL_IMAGE_FILTER_BLUR,
    FOSSIL_IMAGE_FILTER_SHARPEN,
    FOSSIL_IMAGE_FILTER_EDGE,
    FOSSIL_IMAGE_FILTER_EMBOSS,
    FOSSIL_IMAGE_FILTER_CUSTOM
} fossil_image_filter_e;

typedef enum fossil_image_scale_e {
    FOSSIL_IMAGE_SCALE_NEAREST = 0,
    FOSSIL_IMAGE_SCALE_LINEAR,
    FOSSIL_IMAGE_SCALE_CUBIC
} fossil_image_scale_e;

// ------------------------------------------
// CORE OPERATIONS
// ------------------------------------------

/**
 * @brief Applies a filter to the image.
 */
bool fossil_image_process_filter(fossil_image_t *image, fossil_image_filter_e filter);

/**
 * @brief Applies a custom convolution kernel.
 */
bool fossil_image_process_convolve(fossil_image_t *image, const float *kernel, size_t width, size_t height);

/**
 * @brief Adjusts brightness, contrast, and saturation.
 */
bool fossil_image_process_adjust(fossil_image_t *image, float brightness, float contrast, float saturation);

/**
 * @brief Flips the image horizontally or vertically.
 */
bool fossil_image_process_flip(fossil_image_t *image, bool horizontal, bool vertical);

/**
 * @brief Rotates the image by degrees (clockwise).
 */
bool fossil_image_process_rotate(fossil_image_t *image, float degrees);

/**
 * @brief Scales the image to a new width and height.
 */
bool fossil_image_process_resize(fossil_image_t *image, uint32_t new_width, uint32_t new_height, fossil_image_scale_e method);

/**
 * @brief Crops an image region.
 */
bool fossil_image_process_crop(fossil_image_t *image, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

/**
 * @brief Blends two images together with alpha.
 */
bool fossil_image_process_blend(fossil_image_t *dest, const fossil_image_t *src, float alpha);

/**
 * @brief Applies gamma correction.
 */
bool fossil_image_process_gamma(fossil_image_t *image, float gamma_value);

/**
 * @brief Converts between color spaces (RGB, RGBA, Grayscale, etc.)
 */
bool fossil_image_process_colorspace(fossil_image_t *image, fossil_color_space_e target_space);

// ------------------------------------------
// ADVANCED OPS
// ------------------------------------------

/**
 * @brief Performs histogram equalization for contrast improvement.
 */
bool fossil_image_process_equalize(fossil_image_t *image);

/**
 * @brief Detects edges using Sobel or similar operator.
 */
bool fossil_image_process_edge_detect(fossil_image_t *image, bool normalize);

/**
 * @brief Applies Gaussian blur with radius parameter.
 */
bool fossil_image_process_gaussian(fossil_image_t *image, float radius);

#ifdef __cplusplus
}


namespace fossil {

    namespace image {



    } // namespace image

} // namespace fossil

#endif

#endif /* FOSSIL_IMAGE_H */
