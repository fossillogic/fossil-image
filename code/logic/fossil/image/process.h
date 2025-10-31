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
// Fossil Image — Core Type Definition
// ======================================================

/**
 * @brief Supported pixel formats.
 */

/// Supported pixel formats
typedef enum fossil_pixel_format_e {
    FOSSIL_PIXEL_FORMAT_NONE = 0,       ///< No format
    FOSSIL_PIXEL_FORMAT_GRAY8,          ///< 8-bit grayscale
    FOSSIL_PIXEL_FORMAT_GRAY16,         ///< 16-bit grayscale
    FOSSIL_PIXEL_FORMAT_RGB24,          ///< 8-bit per channel RGB
    FOSSIL_PIXEL_FORMAT_RGB48,          ///< 16-bit per channel RGB
    FOSSIL_PIXEL_FORMAT_RGBA32,         ///< 8-bit per channel RGBA
    FOSSIL_PIXEL_FORMAT_RGBA64,         ///< 16-bit per channel RGBA
    FOSSIL_PIXEL_FORMAT_FLOAT32,        ///< 32-bit float grayscale/intensity
    FOSSIL_PIXEL_FORMAT_FLOAT32_RGB,    ///< 32-bit float RGB
    FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA,   ///< 32-bit float RGBA
    FOSSIL_PIXEL_FORMAT_INDEXED8,       ///< 8-bit palette/indexed
    FOSSIL_PIXEL_FORMAT_YUV24,          ///< 24-bit YUV (optional for video)
} fossil_pixel_format_t;

/**
 * @brief Interpolation mode for scaling and transformation.
 */

/// Interpolation modes for scaling/rotation
typedef enum fossil_interp_e {
    FOSSIL_INTERP_NEAREST = 0,
    FOSSIL_INTERP_LINEAR,
    FOSSIL_INTERP_CUBIC,
    FOSSIL_INTERP_LANCZOS,
    FOSSIL_INTERP_BICUBIC,
    FOSSIL_INTERP_MITCHELL,
    FOSSIL_INTERP_BSPLINE
} fossil_interp_t;

/**
 * @brief Core image container for Fossil Image system.
 */

/// Core image container
typedef struct fossil_image_s {
    uint32_t width;                    ///< Image width in pixels
    uint32_t height;                   ///< Image height in pixels
    uint32_t channels;                 ///< Number of channels (1-4 or more for indexed/multi-channel)
    fossil_pixel_format_t format;      ///< Pixel format

    /// Flexible data buffer
    union {
        uint8_t *data;                 ///< 8/16-bit integer buffer
        float   *fdata;                ///< 32-bit float buffer
    };
    size_t size;                        ///< Total buffer size in bytes
    bool owns_data;                     ///< Free buffer on destroy

    // Optional metadata fields
    char name[64];                      ///< Debug/identifier
    char author[64];                    ///< Author or creator
    double dpi_x;                       ///< Horizontal resolution
    double dpi_y;                       ///< Vertical resolution
    double exposure;                    ///< For HDR images
    uint32_t channels_mask;             ///< Bitmask for multi-channel (custom)
    void *userdata;                     ///< Optional user-defined buffer

    // Extended origin metadata
    bool is_ai_generated;               ///< True if image is AI-generated
    char creation_os[32];               ///< OS used to create image (e.g., "Windows", "Linux", "MacOS")
    char software[64];                  ///< Software or tool used to create image
    char creation_date[32];             ///< Optional timestamp as string
} fossil_image_t;

// ======================================================
// Fossil Image — Process Sub-Library
// ======================================================

/**
 * @brief Create a new image with specified dimensions and format.
 *
 * Allocates and initializes a fossil_image_t structure with the given width,
 * height, pixel format, and number of channels. The pixel buffer is allocated
 * and zero-initialized. Returns a pointer to the new image, or NULL on failure.
 *
 * @param width Image width in pixels.
 * @param height Image height in pixels.
 * @param format Pixel format.
 * @return Pointer to the new fossil_image_t, or NULL if allocation fails.
 */
fossil_image_t *fossil_image_process_create(
    uint32_t width,
    uint32_t height,
    fossil_pixel_format_t format
);

/**
 * @brief Destroy an image and free its resources.
 *
 * Frees the pixel buffer and the fossil_image_t structure if it was allocated
 * by fossil_image_process_create. Safe to call with NULL.
 *
 * @param image Pointer to the fossil_image_t structure to destroy.
 */
void fossil_image_process_destroy(
    fossil_image_t *image
);

/**
 * @brief Resize an image to target dimensions.
 *
 * This function resizes the given image to the specified width and height,
 * using the selected interpolation mode. The pixel data is reallocated and
 * resampled according to the interpolation algorithm (nearest, linear, cubic,
 * or Lanczos). The image's metadata (width, height, and buffer size) is updated
 * accordingly. Returns true on success, false on failure (e.g., allocation error).
 *
 * @param image Pointer to the fossil_image_t structure to resize.
 * @param width Target width in pixels.
 * @param height Target height in pixels.
 * @param mode Interpolation mode for resampling.
 * @return true if successful, false otherwise.
 */
bool fossil_image_process_resize(
    fossil_image_t *image,
    uint32_t width,
    uint32_t height,
    fossil_interp_t mode
);

/**
 * @brief Crop an image to a specified rectangle.
 *
 * This function crops the image to the rectangle defined by (x, y, width, height).
 * The pixel buffer is updated to contain only the cropped region, and image metadata
 * is updated. Returns true on success, false if the crop rectangle is out of bounds
 * or allocation fails.
 *
 * @param image Pointer to the fossil_image_t structure to crop.
 * @param x X-coordinate of the top-left corner of the crop rectangle.
 * @param y Y-coordinate of the top-left corner of the crop rectangle.
 * @param width Width of the crop rectangle.
 * @param height Height of the crop rectangle.
 * @return true if successful, false otherwise.
 */
bool fossil_image_process_crop(
    fossil_image_t *image,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height
);

/**
 * @brief Flip the image horizontally or vertically.
 *
 * This function flips the image in-place either horizontally, vertically, or both,
 * depending on the boolean flags. The pixel buffer is rearranged accordingly.
 * Returns true on success, false on failure.
 *
 * @param image Pointer to the fossil_image_t structure to flip.
 * @param horizontal If true, flip horizontally.
 * @param vertical If true, flip vertically.
 * @return true if successful, false otherwise.
 */
bool fossil_image_process_flip(
    fossil_image_t *image,
    bool horizontal,
    bool vertical
);

/**
 * @brief Rotate an image by degrees (supports arbitrary angles).
 *
 * Rotates the image by the specified angle in degrees (clockwise). The output
 * image may be resized to fit the rotated content. Uses interpolation for pixel
 * sampling. Returns true on success, false on failure.
 *
 * @param image Pointer to the fossil_image_t structure to rotate.
 * @param degrees Angle in degrees to rotate (positive is clockwise).
 * @param interp Interpolation mode for resampling.
 * @return true if successful, false otherwise.
 */
bool fossil_image_process_rotate(
    fossil_image_t *image,
    float degrees,
    fossil_interp_t interp
);

/**
 * @brief Blend two images together using a blend ratio.
 *
 * Blends the source image into the destination image using the specified ratio.
 * The ratio determines the weight of each image in the blend (0.0 = only dst,
 * 1.0 = only src). Both images must have the same dimensions and format.
 * Returns true on success, false otherwise.
 *
 * @param dst Pointer to the destination fossil_image_t structure.
 * @param src Pointer to the source fossil_image_t structure.
 * @param ratio Blend ratio (0.0 to 1.0).
 * @return true if successful, false otherwise.
 */
bool fossil_image_process_blend(
    fossil_image_t *dst,
    const fossil_image_t *src,
    float ratio
);

/**
 * @brief Composite overlay of one image onto another using alpha.
 *
 * Composites the overlay image onto the destination image at position (x, y),
 * using the specified alpha value for blending. The overlay may be partially
 * outside the destination image. Returns true on success, false otherwise.
 *
 * @param dst Pointer to the destination fossil_image_t structure.
 * @param overlay Pointer to the overlay fossil_image_t structure.
 * @param x X-coordinate in the destination image for overlay placement.
 * @param y Y-coordinate in the destination image for overlay placement.
 * @param alpha Alpha value for blending (0.0 = transparent, 1.0 = opaque).
 * @return true if successful, false otherwise.
 */
bool fossil_image_process_composite(
    fossil_image_t *dst,
    const fossil_image_t *overlay,
    uint32_t x,
    uint32_t y,
    float alpha
);

/**
 * @brief Convert image to grayscale.
 *
 * Converts the image to grayscale in-place. For color images, the channels are
 * combined using standard luminance conversion. The pixel format and channel
 * count are updated accordingly. Returns true on success, false otherwise.
 *
 * @param image Pointer to the fossil_image_t structure to convert.
 * @return true if successful, false otherwise.
 */
bool fossil_image_process_grayscale(
    fossil_image_t *image
);

/**
 * @brief Apply binary threshold operation.
 *
 * Applies a binary threshold to the image, setting pixels above the threshold
 * to maximum value and below to zero. Only applicable to grayscale images.
 * Returns true on success, false otherwise.
 *
 * @param image Pointer to the fossil_image_t structure to threshold.
 * @param threshold Threshold value (0-255).
 * @return true if successful, false otherwise.
 */
bool fossil_image_process_threshold(
    fossil_image_t *image,
    uint8_t threshold
);

/**
 * @brief Invert image colors.
 *
 * Inverts the colors of the image in-place. For grayscale, each pixel value is
 * replaced by its complement. For color images, each channel is inverted.
 * Returns true on success, false otherwise.
 *
 * @param image Pointer to the fossil_image_t structure to invert.
 * @return true if successful, false otherwise.
 */
bool fossil_image_process_invert(
    fossil_image_t *image
);

/**
 * @brief Normalize brightness and contrast.
 *
 * Normalizes the image's brightness and contrast by stretching pixel values
 * to the full available range. Useful for enhancing low-contrast images.
 * Returns true on success, false otherwise.
 *
 * @param image Pointer to the fossil_image_t structure to normalize.
 * @return true if successful, false otherwise.
 */
bool fossil_image_process_normalize(
    fossil_image_t *image
);

#ifdef __cplusplus
}


namespace fossil {

    namespace image {

        /**
         * @brief C++ wrapper for Fossil Image processing functions.
         *
         * The Process class provides static methods that wrap the core C functions
         * for image processing operations such as resizing, cropping, flipping,
         * rotating, blending, compositing, grayscale conversion, thresholding,
         * inversion, and normalization. Each method forwards its arguments to the
         * corresponding C function, allowing seamless integration with C++ codebases.
         */
        class Process {
        public:
            /**
             * @brief Create a new image with specified dimensions and format.
             *
             * Allocates and initializes a fossil_image_t structure with the given width,
             * height, pixel format, and number of channels. The pixel buffer is allocated
             * and zero-initialized. Returns a pointer to the new image, or NULL on failure.
             *
             * @param width Image width in pixels.
             * @param height Image height in pixels.
             * @param format Pixel format.
             * @return Pointer to the new fossil_image_t, or NULL if allocation fails.
             */
            static fossil_image_t *create(uint32_t width, uint32_t height, fossil_pixel_format_t format) {
                return fossil_image_process_create(width, height, format);
            }

            /**
             * @brief Destroy an image and free its resources.
             *
             * Frees the pixel buffer and the fossil_image_t structure if it was allocated
             * by fossil_image_process_create. Safe to call with NULL.
             *
             * @param image Pointer to the fossil_image_t structure to destroy.
             */
            static void destroy(fossil_image_t *image) {
                fossil_image_process_destroy(image);
            }

            /**
             * @brief Resize an image to target dimensions.
             *
             * Resizes the given image to the specified width and height using the
             * selected interpolation mode. The pixel data is reallocated and
             * resampled according to the interpolation algorithm. Image metadata
             * is updated accordingly.
             *
             * @param image Pointer to the fossil_image_t structure to resize.
             * @param width Target width in pixels.
             * @param height Target height in pixels.
             * @param mode Interpolation mode for resampling.
             * @return true if successful, false otherwise.
             */
            static bool resize(fossil_image_t *image, uint32_t width, uint32_t height, fossil_interp_t mode) {
                return fossil_image_process_resize(image, width, height, mode);
            }

            /**
             * @brief Crop an image to a specified rectangle.
             *
             * Crops the image to the rectangle defined by (x, y, width, height).
             * The pixel buffer is updated to contain only the cropped region, and
             * image metadata is updated.
             *
             * @param image Pointer to the fossil_image_t structure to crop.
             * @param x X-coordinate of the top-left corner of the crop rectangle.
             * @param y Y-coordinate of the top-left corner of the crop rectangle.
             * @param width Width of the crop rectangle.
             * @param height Height of the crop rectangle.
             * @return true if successful, false otherwise.
             */
            static bool crop(fossil_image_t *image, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
                return fossil_image_process_crop(image, x, y, width, height);
            }

            /**
             * @brief Flip the image horizontally or vertically.
             *
             * Flips the image in-place either horizontally, vertically, or both,
             * depending on the boolean flags. The pixel buffer is rearranged
             * accordingly.
             *
             * @param image Pointer to the fossil_image_t structure to flip.
             * @param horizontal If true, flip horizontally.
             * @param vertical If true, flip vertically.
             * @return true if successful, false otherwise.
             */
            static bool flip(fossil_image_t *image, bool horizontal, bool vertical) {
                return fossil_image_process_flip(image, horizontal, vertical);
            }

            /**
             * @brief Rotate an image by degrees (supports arbitrary angles).
             *
             * Rotates the image by the specified angle in degrees (clockwise).
             * The output image may be resized to fit the rotated content. Uses
             * interpolation for pixel sampling.
             *
             * @param image Pointer to the fossil_image_t structure to rotate.
             * @param degrees Angle in degrees to rotate (positive is clockwise).
             * @param interp Interpolation mode for resampling.
             * @return true if successful, false otherwise.
             */
            static bool rotate(fossil_image_t *image, float degrees, fossil_interp_t interp) {
                return fossil_image_process_rotate(image, degrees, interp);
            }

            /**
             * @brief Blend two images together using a blend ratio.
             *
             * Blends the source image into the destination image using the
             * specified ratio. The ratio determines the weight of each image in
             * the blend (0.0 = only dst, 1.0 = only src). Both images must have
             * the same dimensions and format.
             *
             * @param dst Pointer to the destination fossil_image_t structure.
             * @param src Pointer to the source fossil_image_t structure.
             * @param ratio Blend ratio (0.0 to 1.0).
             * @return true if successful, false otherwise.
             */
            static bool blend(fossil_image_t *dst, const fossil_image_t *src, float ratio) {
                return fossil_image_process_blend(dst, src, ratio);
            }

            /**
             * @brief Composite overlay of one image onto another using alpha.
             *
             * Composites the overlay image onto the destination image at position
             * (x, y), using the specified alpha value for blending. The overlay
             * may be partially outside the destination image.
             *
             * @param dst Pointer to the destination fossil_image_t structure.
             * @param overlay Pointer to the overlay fossil_image_t structure.
             * @param x X-coordinate in the destination image for overlay placement.
             * @param y Y-coordinate in the destination image for overlay placement.
             * @param alpha Alpha value for blending (0.0 = transparent, 1.0 = opaque).
             * @return true if successful, false otherwise.
             */
            static bool composite(fossil_image_t *dst, const fossil_image_t *overlay, uint32_t x, uint32_t y, float alpha) {
                return fossil_image_process_composite(dst, overlay, x, y, alpha);
            }

            /**
             * @brief Convert image to grayscale.
             *
             * Converts the image to grayscale in-place. For color images, the
             * channels are combined using standard luminance conversion. The pixel
             * format and channel count are updated accordingly.
             *
             * @param image Pointer to the fossil_image_t structure to convert.
             * @return true if successful, false otherwise.
             */
            static bool grayscale(fossil_image_t *image) {
                return fossil_image_process_grayscale(image);
            }

            /**
             * @brief Apply binary threshold operation.
             *
             * Applies a binary threshold to the image, setting pixels above the
             * threshold to maximum value and below to zero. Only applicable to
             * grayscale images.
             *
             * @param image Pointer to the fossil_image_t structure to threshold.
             * @param threshold Threshold value (0-255).
             * @return true if successful, false otherwise.
             */
            static bool threshold(fossil_image_t *image, uint8_t threshold) {
                return fossil_image_process_threshold(image, threshold);
            }

            /**
             * @brief Invert image colors.
             *
             * Inverts the colors of the image in-place. For grayscale, each pixel
             * value is replaced by its complement. For color images, each channel
             * is inverted.
             *
             * @param image Pointer to the fossil_image_t structure to invert.
             * @return true if successful, false otherwise.
             */
            static bool invert(fossil_image_t *image) {
                return fossil_image_process_invert(image);
            }

            /**
             * @brief Normalize brightness and contrast.
             *
             * Normalizes the image's brightness and contrast by stretching pixel
             * values to the full available range. Useful for enhancing low-contrast
             * images.
             *
             * @param image Pointer to the fossil_image_t structure to normalize.
             * @return true if successful, false otherwise.
             */
            static bool normalize(fossil_image_t *image) {
                return fossil_image_process_normalize(image);
            }

        };

    } // namespace image

} // namespace fossil

#endif

#endif /* FOSSIL_IMAGE_H */
