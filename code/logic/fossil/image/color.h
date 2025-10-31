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

#include "process.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// Fossil Image — Color Sub-Library
// ======================================================

/**
 * @brief Adjust the brightness of an image by a specified offset.
 *
 * This function increases or decreases the brightness of all pixels in the image
 * by adding the given offset value to each color channel. The offset can range
 * from -255 (darken) to +255 (brighten). Values are clamped to valid channel ranges.
 *
 * @param image Pointer to the fossil_image_t structure representing the image to process.
 * @param offset Integer value to add to each color channel (-255 to +255).
 * @return true if the operation succeeds, false otherwise.
 */
bool fossil_image_color_brightness(
    fossil_image_t *image,
    int offset
);

/**
 * @brief Adjust the contrast of an image by a specified factor.
 *
 * This function modifies the contrast of the image by scaling the difference of
 * each pixel from the midpoint by the given factor. A factor of 1.0 leaves the
 * image unchanged, less than 1.0 reduces contrast, and greater than 1.0 increases it.
 * Valid range for factor is typically 0.0 to 4.0.
 *
 * @param image Pointer to the fossil_image_t structure representing the image to process.
 * @param factor Floating-point multiplier for contrast adjustment (0.0–4.0).
 * @return true if the operation succeeds, false otherwise.
 */
bool fossil_image_color_contrast(
    fossil_image_t *image,
    float factor
);

/**
 * @brief Apply gamma correction to an image.
 *
 * This function adjusts the luminance of the image using gamma correction,
 * which compensates for nonlinear display characteristics. The gamma value
 * must be greater than zero, with typical values ranging from 0.5 to 3.0.
 *
 * @param image Pointer to the fossil_image_t structure representing the image to process.
 * @param gamma Gamma correction value (> 0, typically 0.5–3.0).
 * @return true if the operation succeeds, false otherwise.
 */
bool fossil_image_color_gamma(
    fossil_image_t *image,
    float gamma
);

/**
 * @brief Adjust hue, saturation, and value (brightness) of an image.
 *
 * This function modifies the image's color properties in HSV space. The hue_shift
 * parameter rotates the hue in degrees (-180 to 180). The sat_mult and val_mult
 * parameters multiply the saturation and value, respectively, allowing for
 * increased or decreased color intensity and brightness.
 *
 * @param image Pointer to the fossil_image_t structure representing the image to process.
 * @param hue_shift Hue rotation in degrees (-180 to 180).
 * @param sat_mult Saturation multiplier (e.g., 1.0 = unchanged).
 * @param val_mult Value (brightness) multiplier (e.g., 1.0 = unchanged).
 * @return true if the operation succeeds, false otherwise.
 */
bool fossil_image_color_hsv_adjust(
    fossil_image_t *image,
    float hue_shift,
    float sat_mult,
    float val_mult
);

/**
 * @brief Swap or isolate color channels in an image.
 *
 * This function swaps two color channels (e.g., Red and Blue) or isolates a single
 * channel by setting others to zero. The channels are specified by their indices
 * (0 = Red, 1 = Green, 2 = Blue). For example, swapping ch_a=0 and ch_b=2 exchanges
 * the Red and Blue channels.
 *
 * @param image Pointer to the fossil_image_t structure representing the image to process.
 * @param ch_a Index of the first channel (0–2).
 * @param ch_b Index of the second channel (0–2).
 * @return true if the operation succeeds, false otherwise.
 */
bool fossil_image_color_channel_swap(
    fossil_image_t *image,
    uint32_t ch_a,
    uint32_t ch_b
);

/**
 * @brief Convert an RGB image to grayscale using luminance weighting.
 *
 * This function transforms the image to grayscale by computing the luminance
 * of each pixel based on standard weights for the Red, Green, and Blue channels.
 * The result is a single-channel image representing perceived brightness.
 *
 * @param image Pointer to the fossil_image_t structure representing the image to process.
 * @return true if the operation succeeds, false otherwise.
 */
bool fossil_image_color_to_grayscale(
    fossil_image_t *image
);

#ifdef __cplusplus
}


namespace fossil {

    namespace image {

        /**
         * @brief Color class providing static methods for color manipulation operations.
         *
         * This class serves as a C++ wrapper around the C image color functions.
         * It provides static methods for adjusting brightness, contrast, gamma,
         * HSV properties, channel swapping, and grayscale conversion for fossil_image_t objects.
         */
        class Color {
        public:
            /**
             * @brief Adjusts the brightness of the image by a specified offset.
             *
             * This method increases or decreases the brightness of all pixels in the image
             * by adding the given offset value to each color channel. The offset can range
             * from -255 (darken) to +255 (brighten). Values are clamped to valid channel ranges.
             *
             * @param image Pointer to the fossil_image_t structure representing the image to process.
             * @param offset Integer value to add to each color channel (-255 to +255).
             * @return true if the operation succeeds, false otherwise.
             */
            static bool brightness(
            fossil_image_t *image,
            int offset
            ) {
            return fossil_image_color_brightness(image, offset);
            }

            /**
             * @brief Adjusts the contrast of the image by a specified factor.
             *
             * This method modifies the contrast of the image by scaling the difference of
             * each pixel from the midpoint by the given factor. A factor of 1.0 leaves the
             * image unchanged, less than 1.0 reduces contrast, and greater than 1.0 increases it.
             * Valid range for factor is typically 0.0 to 4.0.
             *
             * @param image Pointer to the fossil_image_t structure representing the image to process.
             * @param factor Floating-point multiplier for contrast adjustment (0.0–4.0).
             * @return true if the operation succeeds, false otherwise.
             */
            static bool contrast(
            fossil_image_t *image,
            float factor
            ) {
            return fossil_image_color_contrast(image, factor);
            }

            /**
             * @brief Applies gamma correction to the image.
             *
             * This method adjusts the luminance of the image using gamma correction,
             * which compensates for nonlinear display characteristics. The gamma value
             * must be greater than zero, with typical values ranging from 0.5 to 3.0.
             *
             * @param image Pointer to the fossil_image_t structure representing the image to process.
             * @param gamma Gamma correction value (> 0, typically 0.5–3.0).
             * @return true if the operation succeeds, false otherwise.
             */
            static bool gamma(
            fossil_image_t *image,
            float gamma
            ) {
            return fossil_image_color_gamma(image, gamma);
            }

            /**
             * @brief Adjusts hue, saturation, and value (brightness) of the image.
             *
             * This method modifies the image's color properties in HSV space. The hue_shift
             * parameter rotates the hue in degrees (-180 to 180). The sat_mult and val_mult
             * parameters multiply the saturation and value, respectively, allowing for
             * increased or decreased color intensity and brightness.
             *
             * @param image Pointer to the fossil_image_t structure representing the image to process.
             * @param hue_shift Hue rotation in degrees (-180 to 180).
             * @param sat_mult Saturation multiplier (e.g., 1.0 = unchanged).
             * @param val_mult Value (brightness) multiplier (e.g., 1.0 = unchanged).
             * @return true if the operation succeeds, false otherwise.
             */
            static bool hsv_adjust(
            fossil_image_t *image,
            float hue_shift,
            float sat_mult,
            float val_mult
            ) {
            return fossil_image_color_hsv_adjust(image, hue_shift, sat_mult, val_mult);
            }

            /**
             * @brief Swaps or isolates color channels in the image.
             *
             * This method swaps two color channels (e.g., Red and Blue) or isolates a single
             * channel by setting others to zero. The channels are specified by their indices
             * (0 = Red, 1 = Green, 2 = Blue). For example, swapping ch_a=0 and ch_b=2 exchanges
             * the Red and Blue channels.
             *
             * @param image Pointer to the fossil_image_t structure representing the image to process.
             * @param ch_a Index of the first channel (0–2).
             * @param ch_b Index of the second channel (0–2).
             * @return true if the operation succeeds, false otherwise.
             */
            static bool channel_swap(
            fossil_image_t *image,
            uint32_t ch_a,
            uint32_t ch_b
            ) {
            return fossil_image_color_channel_swap(image, ch_a, ch_b);
            }

            /**
             * @brief Converts an RGB image to grayscale using luminance weighting.
             *
             * This method transforms the image to grayscale by computing the luminance
             * of each pixel based on standard weights for the Red, Green, and Blue channels.
             * The result is a single-channel image representing perceived brightness.
             *
             * @param image Pointer to the fossil_image_t structure representing the image to process.
             * @return true if the operation succeeds, false otherwise.
             */
            static bool to_grayscale(
            fossil_image_t *image
            ) {
            return fossil_image_color_to_grayscale(image);
            }
        };

    } // namespace image

} // namespace fossil

#endif

#endif /* FOSSIL_IMAGE_H */