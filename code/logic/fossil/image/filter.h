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
#ifndef FOSSIL_IMAGE_FILTER_H
#define FOSSIL_IMAGE_FILTER_H

#include "process.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// Fossil Image — Filter Sub-Library
// ======================================================

/**
 * @brief Apply a 3x3 convolution kernel to an image.
 *
 * This function applies a 3x3 convolution kernel to the given image. The kernel
 * is specified as a 3x3 matrix of floating-point values. The scale parameter is
 * used to normalize the result, and the bias parameter is added to each pixel
 * after convolution. This operation can be used for various effects such as
 * blurring, sharpening, or edge detection depending on the kernel provided.
 *
 * @param image Pointer to the fossil_image_t structure representing the image to process.
 * @param kernel 3x3 matrix of floats representing the convolution kernel.
 * @param scale Normalization factor applied to the convolution result.
 * @param bias Value added to each pixel after convolution.
 * @return true if the operation succeeds, false otherwise.
 */
bool fossil_image_filter_convolve3x3(
    fossil_image_t *image,
    const float kernel[3][3],
    float scale,
    float bias
);

/**
 * @brief Apply a Gaussian blur filter.
 *
 * This function applies a Gaussian blur to the given image. The blur is
 * controlled by the radius parameter, which determines the strength and spread
 * of the blur effect. Gaussian blur is commonly used to reduce image noise and
 * detail, creating a smooth, soft appearance.
 *
 * @param image Pointer to the fossil_image_t structure representing the image to process.
 * @param radius The radius of the Gaussian kernel; larger values produce more blur.
 * @return true if the operation succeeds, false otherwise.
 */
bool fossil_image_filter_blur(
    fossil_image_t *image,
    float radius
);

/**
 * @brief Apply a sharpening filter.
 *
 * This function enhances the edges and details in the image by applying a
 * sharpening filter. Sharpening increases the contrast between adjacent pixels,
 * making the image appear crisper and more defined.
 *
 * @param image Pointer to the fossil_image_t structure representing the image to process.
 * @return true if the operation succeeds, false otherwise.
 */
bool fossil_image_filter_sharpen(
    fossil_image_t *image
);

/**
 * @brief Apply an edge detection filter (Sobel).
 *
 * This function detects edges in the image using the Sobel operator. Edge
 * detection highlights areas of high contrast, which typically correspond to
 * boundaries and features within the image.
 *
 * @param image Pointer to the fossil_image_t structure representing the image to process.
 * @return true if the operation succeeds, false otherwise.
 */
bool fossil_image_filter_edge(
    fossil_image_t *image
);

/**
 * @brief Apply an embossing effect.
 *
 * This function applies an emboss filter to the image, creating a relief effect
 * that simulates a three-dimensional surface. Embossing emphasizes edges and
 * gives the image a raised or recessed appearance.
 *
 * @param image Pointer to the fossil_image_t structure representing the image to process.
 * @return true if the operation succeeds, false otherwise.
 */
bool fossil_image_filter_emboss(
    fossil_image_t *image
);

#ifdef __cplusplus
}


namespace fossil {

    namespace image {

        /**
         * @brief Filter class providing static methods for image filtering operations.
         *
         * This class serves as a C++ wrapper around the C image filtering functions.
         * It provides static methods for applying various filters to fossil_image_t objects,
         * including convolution, Gaussian blur, sharpening, edge detection, and embossing.
         * Each method delegates to the corresponding C function, allowing seamless integration
         * of image processing capabilities in C++ codebases.
         */
        class Filter {
        public:
            /**
             * @brief Apply a 3x3 convolution kernel to an image.
             *
             * This method applies a 3x3 convolution kernel to the specified image.
             * The kernel is provided as a 3x3 matrix of floating-point values.
             * The scale parameter normalizes the convolution result, and the bias
             * parameter is added to each pixel after convolution. This operation
             * can be used for effects such as blurring, sharpening, or edge detection.
             *
             * @param image Pointer to the fossil_image_t structure representing the image to process.
             * @param kernel 3x3 matrix of floats representing the convolution kernel.
             * @param scale Normalization factor applied to the convolution result.
             * @param bias Value added to each pixel after convolution.
             * @return true if the operation succeeds, false otherwise.
             */
            static bool convolve3x3(
            fossil_image_t *image,
            const float kernel[3][3],
            float scale,
            float bias
            ) {
                return fossil_image_filter_convolve3x3(image, kernel, scale, bias);
            }

            /**
             * @brief Apply a Gaussian blur filter.
             *
             * This method applies a Gaussian blur to the specified image.
             * The blur is controlled by the radius parameter, which determines
             * the strength and spread of the blur effect. Gaussian blur is
             * commonly used to reduce image noise and detail, creating a smooth,
             * soft appearance.
             *
             * @param image Pointer to the fossil_image_t structure representing the image to process.
             * @param radius The radius of the Gaussian kernel; larger values produce more blur.
             * @return true if the operation succeeds, false otherwise.
             */
            static bool blur(
            fossil_image_t *image,
            float radius
            ) {
                return fossil_image_filter_blur(image, radius);
            }

            /**
             * @brief Apply a sharpening filter.
             *
             * This method enhances the edges and details in the image by applying
             * a sharpening filter. Sharpening increases the contrast between adjacent
             * pixels, making the image appear crisper and more defined.
             *
             * @param image Pointer to the fossil_image_t structure representing the image to process.
             * @return true if the operation succeeds, false otherwise.
             */
            static bool sharpen(
            fossil_image_t *image
            ) {
                return fossil_image_filter_sharpen(image);
            }

            /**
             * @brief Apply an edge detection filter (Sobel).
             *
             * This method detects edges in the image using the Sobel operator.
             * Edge detection highlights areas of high contrast, which typically
             * correspond to boundaries and features within the image.
             *
             * @param image Pointer to the fossil_image_t structure representing the image to process.
             * @return true if the operation succeeds, false otherwise.
             */
            static bool edge(
            fossil_image_t *image
            ) {
                return fossil_image_filter_edge(image);
            }

            /**
             * @brief Apply an embossing effect.
             *
             * This method applies an emboss filter to the image, creating a relief
             * effect that simulates a three-dimensional surface. Embossing emphasizes
             * edges and gives the image a raised or recessed appearance.
             *
             * @param image Pointer to the fossil_image_t structure representing the image to process.
             * @return true if the operation succeeds, false otherwise.
             */
            static bool emboss(
            fossil_image_t *image
            ) {
                return fossil_image_filter_emboss(image);
            }

        };

    } // namespace image

} // namespace fossil

#endif

#endif /* FOSSIL_IMAGE_H */