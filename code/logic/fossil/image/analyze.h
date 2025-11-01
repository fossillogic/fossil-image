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
#ifndef FOSSIL_IMAGE_ANALYZER_H
#define FOSSIL_IMAGE_ANALYZER_H

#include "process.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// Fossil Image — Analyze Sub-Library
// ======================================================

/**
 * @brief Computes the histogram of pixel values for each channel in the image.
 *
 * This function analyzes the input image and calculates the frequency of each possible
 * pixel value (0–255) for all channels (e.g., Red, Green, Blue). The resulting histogram
 * is stored in the provided output array, which must be preallocated by the caller to
 * hold 256 bins per channel (i.e., array size = 256 * channels).
 *
 * @param image Pointer to the input fossil_image_t structure to analyze.
 * @param out_hist Pointer to an array of integers where the histogram will be stored.
 *        The array must be sized to 256 * channels.
 * @return true if the histogram is computed successfully, false otherwise.
 */
bool fossil_image_analyze_histogram(
    const fossil_image_t *image,
    uint32_t *out_hist
);

/**
 * @brief Computes the mean and standard deviation for each channel in the image.
 *
 * This function calculates the average (mean) and spread (standard deviation) of pixel
 * values for each channel in the input image. The results are written to the provided
 * mean and stddev arrays, which must be preallocated to hold one value per channel.
 *
 * @param image Pointer to the input fossil_image_t structure to analyze.
 * @param mean Pointer to an array of doubles to receive the mean values (per channel).
 * @param stddev Pointer to an array of doubles to receive the standard deviation values (per channel).
 * @return true if the computation succeeds, false otherwise.
 */
bool fossil_image_analyze_mean_stddev(
    const fossil_image_t *image,
    double *mean,
    double *stddev
);

/**
 * @brief Computes the average brightness of the image, normalized to [0.0, 1.0].
 *
 * This function calculates the overall brightness of the image by averaging pixel
 * intensities. For grayscale images, it averages the single channel; for RGB images,
 * it averages the luminance across all channels. The result is a normalized value
 * between 0.0 (dark) and 1.0 (bright).
 *
 * @param image Pointer to the input fossil_image_t structure to analyze.
 * @param out_brightness Pointer to a double to receive the computed brightness value.
 * @return true if the computation succeeds, false otherwise.
 */
bool fossil_image_analyze_brightness(
    const fossil_image_t *image,
    double *out_brightness
);

/**
 * @brief Estimates the image contrast using the root mean square (RMS) contrast formula.
 *
 * This function computes the contrast of the image by calculating the standard deviation
 * of pixel intensities, which reflects the degree of variation in brightness. The RMS
 * contrast is a common measure for visual contrast in images.
 *
 * @param image Pointer to the input fossil_image_t structure to analyze.
 * @param out_contrast Pointer to a double to receive the computed contrast value.
 * @return true if the computation succeeds, false otherwise.
 */
bool fossil_image_analyze_contrast(
    const fossil_image_t *image,
    double *out_contrast
);

/**
 * @brief Computes the edge magnitude map of the image using the Sobel operator.
 *
 * This function applies the Sobel edge detection algorithm to the input image,
 * producing a new grayscale image that highlights the magnitude of edges. The
 * result is written to the destination image, which must be preallocated and
 * compatible in size with the source image.
 *
 * @param src Pointer to the input fossil_image_t structure to analyze.
 * @param dst Pointer to the output fossil_image_t structure to receive the edge map.
 * @return true if the edge map is computed successfully, false otherwise.
 */
bool fossil_image_analyze_edge_sobel(
    const fossil_image_t *src,
    fossil_image_t *dst
);

/**
 * @brief Computes the color variance and entropy estimate for visual complexity analysis.
 *
 * This function analyzes the distribution of pixel values in the image to estimate
 * its entropy, which reflects the amount of information or complexity present. Higher
 * entropy values indicate more visually complex or detailed images.
 *
 * @param image Pointer to the input fossil_image_t structure to analyze.
 * @param out_entropy Pointer to a double to receive the computed entropy value.
 * @return true if the computation succeeds, false otherwise.
 */
bool fossil_image_analyze_entropy(
    const fossil_image_t *image,
    double *out_entropy
);

#ifdef __cplusplus
}

namespace fossil {

    namespace image {

        /**
         * @brief Analyzer class provides static methods for image analysis operations.
         *
         * This class wraps the C-style image analysis functions in a C++ interface,
         * offering convenient static methods for common image statistics and feature
         * extraction tasks. All methods require a pointer to a fossil_image_t structure
         * and appropriate output buffers for results.
         */
        class Analyzer
        {
        public:
            /**
             * @brief Computes the histogram of pixel values for each channel in the image.
             *
             * This method analyzes the input image and calculates the frequency of each possible
             * pixel value (0–255) for all channels. The resulting histogram is stored in the
             * provided output array, which must be preallocated by the caller to hold 256 bins
             * per channel (i.e., array size = 256 * channels).
             *
             * @param image Pointer to the input fossil_image_t structure to analyze.
             * @param out_hist Pointer to an array of integers where the histogram will be stored.
             *        The array must be sized to 256 * channels.
             * @return true if the histogram is computed successfully, false otherwise.
             */
            static bool histogram(const fossil_image_t *image, uint32_t *out_hist)
            {
            return fossil_image_analyze_histogram(image, out_hist);
            }

            /**
             * @brief Computes the mean and standard deviation for each channel in the image.
             *
             * This method calculates the average (mean) and spread (standard deviation) of pixel
             * values for each channel in the input image. The results are written to the provided
             * mean and stddev arrays, which must be preallocated to hold one value per channel.
             *
             * @param image Pointer to the input fossil_image_t structure to analyze.
             * @param mean Pointer to an array of doubles to receive the mean values (per channel).
             * @param stddev Pointer to an array of doubles to receive the standard deviation values (per channel).
             * @return true if the computation succeeds, false otherwise.
             */
            static bool meanStddev(const fossil_image_t *image, double *mean, double *stddev)
            {
            return fossil_image_analyze_mean_stddev(image, mean, stddev);
            }

            /**
             * @brief Computes the average brightness of the image, normalized to [0.0, 1.0].
             *
             * This method calculates the overall brightness of the image by averaging pixel
             * intensities. For grayscale images, it averages the single channel; for RGB images,
             * it averages the luminance across all channels. The result is a normalized value
             * between 0.0 (dark) and 1.0 (bright).
             *
             * @param image Pointer to the input fossil_image_t structure to analyze.
             * @param out_brightness Pointer to a double to receive the computed brightness value.
             * @return true if the computation succeeds, false otherwise.
             */
            static bool brightness(const fossil_image_t *image, double *out_brightness)
            {
            return fossil_image_analyze_brightness(image, out_brightness);
            }

            /**
             * @brief Estimates the image contrast using the root mean square (RMS) contrast formula.
             *
             * This method computes the contrast of the image by calculating the standard deviation
             * of pixel intensities, which reflects the degree of variation in brightness. The RMS
             * contrast is a common measure for visual contrast in images.
             *
             * @param image Pointer to the input fossil_image_t structure to analyze.
             * @param out_contrast Pointer to a double to receive the computed contrast value.
             * @return true if the computation succeeds, false otherwise.
             */
            static bool contrast(const fossil_image_t *image, double *out_contrast)
            {
            return fossil_image_analyze_contrast(image, out_contrast);
            }

            /**
             * @brief Computes the edge magnitude map of the image using the Sobel operator.
             *
             * This method applies the Sobel edge detection algorithm to the input image,
             * producing a new grayscale image that highlights the magnitude of edges. The
             * result is written to the destination image, which must be preallocated and
             * compatible in size with the source image.
             *
             * @param src Pointer to the input fossil_image_t structure to analyze.
             * @param dst Pointer to the output fossil_image_t structure to receive the edge map.
             * @return true if the edge map is computed successfully, false otherwise.
             */
            static bool edgeSobel(const fossil_image_t *src, fossil_image_t *dst)
            {
            return fossil_image_analyze_edge_sobel(src, dst);
            }

            /**
             * @brief Computes the color variance and entropy estimate for visual complexity analysis.
             *
             * This method analyzes the distribution of pixel values in the image to estimate
             * its entropy, which reflects the amount of information or complexity present. Higher
             * entropy values indicate more visually complex or detailed images.
             *
             * @param image Pointer to the input fossil_image_t structure to analyze.
             * @param out_entropy Pointer to a double to receive the computed entropy value.
             * @return true if the computation succeeds, false otherwise.
             */
            static bool entropy(const fossil_image_t *image, double *out_entropy)
            {
            return fossil_image_analyze_entropy(image, out_entropy);
            }
        };

    } // namespace image

} // namespace fossil

#endif

#endif /* FOSSIL_IMAGE_H */
