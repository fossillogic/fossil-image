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
#ifndef FOSSIL_IMAGE_COLOR_H
#define FOSSIL_IMAGE_COLOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// Fossil Image — Color Sub-Library
// ======================================================
//
// Handles color space conversions, channel manipulation,
// tone adjustments, and color-based utilities.
//
// ======================================================

// ------------------------------------------
// ENUMS AND DEFINITIONS
// ------------------------------------------

typedef enum fossil_color_channel_e {
    FOSSIL_COLOR_CHANNEL_RED = 0,
    FOSSIL_COLOR_CHANNEL_GREEN,
    FOSSIL_COLOR_CHANNEL_BLUE,
    FOSSIL_COLOR_CHANNEL_ALPHA,
    FOSSIL_COLOR_CHANNEL_LUMINANCE
} fossil_color_channel_e;

typedef enum fossil_color_space_e {
    FOSSIL_COLOR_SPACE_RGB = 0,
    FOSSIL_COLOR_SPACE_RGBA,
    FOSSIL_COLOR_SPACE_HSV,
    FOSSIL_COLOR_SPACE_HSL,
    FOSSIL_COLOR_SPACE_YUV,
    FOSSIL_COLOR_SPACE_GRAY
} fossil_color_space_e;

// ------------------------------------------
// COLOR SPACE CONVERSIONS
// ------------------------------------------

/**
 * @brief Converts an image from one color space to another.
 */
bool fossil_image_color_convert(fossil_image_t *image, fossil_color_space_e target_space);

/**
 * @brief Converts a single pixel color between color spaces.
 */
fossil_color_t fossil_image_color_convert_pixel(fossil_color_t color, fossil_color_space_e from, fossil_color_space_e to);

// ------------------------------------------
// CHANNEL OPERATIONS
// ------------------------------------------

/**
 * @brief Extracts a single color channel from an image.
 */
bool fossil_image_color_extract_channel(const fossil_image_t *image, fossil_color_channel_e channel, fossil_image_t *out_channel);

/**
 * @brief Merges individual color channels into a single image.
 */
bool fossil_image_color_merge_channels(fossil_image_t *dest, const fossil_image_t *r, const fossil_image_t *g, const fossil_image_t *b, const fossil_image_t *a);

/**
 * @brief Swaps two color channels in place.
 */
bool fossil_image_color_swap_channels(fossil_image_t *image, fossil_color_channel_e a, fossil_color_channel_e b);

/**
 * @brief Sets a single channel to a constant value (0.0–1.0 range).
 */
bool fossil_image_color_set_channel(fossil_image_t *image, fossil_color_channel_e channel, float value);

// ------------------------------------------
// COLOR ADJUSTMENT AND MANIPULATION
// ------------------------------------------

/**
 * @brief Applies a color tint overlay.
 */
bool fossil_image_color_tint(fossil_image_t *image, fossil_color_t tint_color, float intensity);

/**
 * @brief Adjusts hue, saturation, and lightness values.
 */
bool fossil_image_color_hsl_adjust(fossil_image_t *image, float hue_shift, float sat_scale, float light_scale);

/**
 * @brief Adjusts RGB levels individually.
 */
bool fossil_image_color_levels(fossil_image_t *image, float r_scale, float g_scale, float b_scale);

/**
 * @brief Inverts specific channels or all channels.
 */
bool fossil_image_color_invert(fossil_image_t *image, bool invert_alpha);

/**
 * @brief Applies a color balance correction across temperature and tint.
 */
bool fossil_image_color_balance(fossil_image_t *image, float temperature, float tint);

/**
 * @brief Converts the image to grayscale.
 */
bool fossil_image_color_grayscale(fossil_image_t *image);

// ------------------------------------------
// COLOR UTILITY FUNCTIONS
// ------------------------------------------

/**
 * @brief Computes the average color of the entire image.
 */
fossil_color_t fossil_image_color_average(const fossil_image_t *image);

/**
 * @brief Computes the dominant (mode) color of the image.
 */
fossil_color_t fossil_image_color_dominant(const fossil_image_t *image);

/**
 * @brief Performs color key transparency (chroma key effect).
 */
bool fossil_image_color_key(fossil_image_t *image, fossil_color_t key_color, float tolerance);

#ifdef __cplusplus
}


namespace fossil {

    namespace image {



    } // namespace image

} // namespace fossil

#endif

#endif /* FOSSIL_IMAGE_H */