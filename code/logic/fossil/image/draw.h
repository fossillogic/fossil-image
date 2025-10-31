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
#ifndef FOSSIL_IMAGE_DRAW_H
#define FOSSIL_IMAGE_DRAW_H

#include "process.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// Fossil Image — Draw Sub-Library
// ======================================================

/**
 * @brief Draw a single pixel with RGB or grayscale color.
 */
bool fossil_image_draw_pixel(
    fossil_image_t *image,
    uint32_t x,
    uint32_t y,
    const uint8_t *color
);

/**
 * @brief Draw a straight line using Bresenham's algorithm.
 */
bool fossil_image_draw_line(
    fossil_image_t *image,
    uint32_t x0,
    uint32_t y0,
    uint32_t x1,
    uint32_t y1,
    const uint8_t *color
);

/**
 * @brief Draw a rectangle (optionally filled).
 */
bool fossil_image_draw_rect(
    fossil_image_t *image,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    const uint8_t *color,
    bool filled
);

/**
 * @brief Draw a circle (optionally filled).
 */
bool fossil_image_draw_circle(
    fossil_image_t *image,
    uint32_t cx,
    uint32_t cy,
    uint32_t radius,
    const uint8_t *color,
    bool filled
);

/**
 * @brief Fill the entire image with a solid color.
 */
bool fossil_image_draw_fill(
    fossil_image_t *image,
    const uint8_t *color
);

/**
 * @brief Placeholder for simple bitmap text drawing (monospace font).
 * Implemented as fixed-size pattern rendering.
 */
bool fossil_image_draw_text(
    fossil_image_t *image,
    uint32_t x,
    uint32_t y,
    const char *text,
    const uint8_t *color
);

#ifdef __cplusplus
}


namespace fossil {

    namespace image {



    } // namespace image

} // namespace fossil

#endif

#endif /* FOSSIL_IMAGE_H */
