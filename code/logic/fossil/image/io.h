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
// Fossil Image — IO & Generator Sub-Library
// ======================================================

/**
 * @brief Load image from file by format string ID.
 * Supported formats: "bmp", "ppm"
 */
bool fossil_image_load(
    const char *filename,
    const char *format_id,
    fossil_image_t *out_image
);

/**
 * @brief Save image to file by format string ID.
 * Supported formats: "bmp", "ppm"
 */
bool fossil_image_save(
    const char *filename,
    const char *format_id,
    const fossil_image_t *image
);

/**
 * @brief Generate a new image procedurally.
 * Supported generator types: "solid", "gradient", "checker", "noise"
 * @param out_image Image to initialize
 * @param type_id Generator type string
 * @param width Width in pixels
 * @param height Height in pixels
 * @param channels Number of channels (1, 3, 4)
 * @param params Optional parameters as float array (depends on generator type)
 */
bool fossil_image_generate(
    fossil_image_t *out_image,
    const char *type_id,
    uint32_t width,
    uint32_t height,
    uint32_t channels,
    const float *params

#ifdef __cplusplus
}


namespace fossil {

    namespace image {



    } // namespace image

} // namespace fossil

#endif

#endif /* FOSSIL_IMAGE_H */
