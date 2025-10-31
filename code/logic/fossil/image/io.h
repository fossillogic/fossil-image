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
#ifndef FOSSIL_IMAGE_IO_H
#define FOSSIL_IMAGE_IO_H

#include "process.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// Fossil Image — IO + Generator Sub-Library
// ======================================================

/**
 * @brief Supported IO formats by string identifier.
 *
 * String identifiers:
 *  - "bmp"   → Windows Bitmap
 *  - "ppm"   → Portable Pixmap (ASCII/Binary)
 *  - "pgm"   → Portable Graymap
 *  - "tga"   → Targa TrueVision
 *  - "raw"   → Raw pixel dump
 *  - "fossil"→ Fossil custom format (metadata + pixel data)
 */
typedef enum fossil_image_io_format_e {
    FOSSIL_IMAGE_IO_BMP = 0,
    FOSSIL_IMAGE_IO_PPM,
    FOSSIL_IMAGE_IO_PGM,
    FOSSIL_IMAGE_IO_TGA,
    FOSSIL_IMAGE_IO_RAW,
    FOSSIL_IMAGE_IO_FOSSIL
} fossil_image_io_format_t;

/**
 * @brief Map a string (like "bmp") to an IO format enum.
 */
fossil_image_io_format_t fossil_image_io_format_from_string(const char *id);

/**
 * @brief Save image to disk in a given format.
 *
 * @param image   The image to save.
 * @param path    Destination file path.
 * @param format  String ID for format ("bmp", "ppm", etc.)
 */
bool fossil_image_io_save(
    const fossil_image_t *image,
    const char *path,
    const char *format
);

/**
 * @brief Load image from file path, format auto-detected or forced.
 *
 * @param path    Source file path.
 * @param format  Optional string ID; if NULL, auto-detect from extension.
 */
fossil_image_t *fossil_image_io_load(
    const char *path,
    const char *format
);

// ======================================================
// Fossil Image — Generator Sub-Library
// ======================================================

/**
 * @brief Supported built-in generators by string identifier.
 *
 *  - "solid"     → Uniform color
 *  - "gradient"  → Linear gradient
 *  - "noise"     → Random noise
 *  - "checker"   → Checkerboard pattern
 *  - "circle"    → Circular mask / dot
 *  - "test"      → Fossil test pattern
 */
typedef enum fossil_image_gen_type_e {
    FOSSIL_IMAGE_GEN_SOLID = 0,
    FOSSIL_IMAGE_GEN_GRADIENT,
    FOSSIL_IMAGE_GEN_NOISE,
    FOSSIL_IMAGE_GEN_CHECKER,
    FOSSIL_IMAGE_GEN_CIRCLE,
    FOSSIL_IMAGE_GEN_TEST
} fossil_image_gen_type_t;

/**
 * @brief Map a generator string (like "noise") to an enum.
 */
fossil_image_gen_type_t fossil_image_gen_type_from_string(const char *id);

/**
 * @brief Generate a new image procedurally.
 *
 * @param width    Image width
 * @param height   Image height
 * @param format   Pixel format
 * @param gen_id   String ID for generator type
 * @param param    Optional string for parameters (e.g. color, density)
 */
fossil_image_t *fossil_image_io_generate(
    uint32_t width,
    uint32_t height,
    fossil_pixel_format_t format,
    const char *gen_id,
    const char *param
);

#ifdef __cplusplus
}


namespace fossil {

    namespace image {



    } // namespace image

} // namespace fossil

#endif

#endif /* FOSSIL_IMAGE_H */
