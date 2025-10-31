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
#include "fossil/image/draw.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// ======================================================
// Fossil Image — Draw Sub-Library Implementation
// ======================================================

static inline void fossil_draw_set_pixel(fossil_image_t *image, uint32_t x, uint32_t y, const uint8_t *color) {
    if (!image || !image->data) return;
    if (x >= image->width || y >= image->height) return;

    size_t idx = (y * image->width + x) * image->channels;
    for (uint32_t c = 0; c < image->channels; ++c)
        image->data[idx + c] = color[c];
}

bool fossil_image_draw_pixel(fossil_image_t *image, uint32_t x, uint32_t y, const uint8_t *color) {
    if (!image || !color)
        return false;
    fossil_draw_set_pixel(image, x, y, color);
    return true;
}

bool fossil_image_draw_line(fossil_image_t *image, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, const uint8_t *color) {
    if (!image || !color)
        return false;

    int dx = abs((int)x1 - (int)x0);
    int dy = abs((int)y1 - (int)y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        fossil_draw_set_pixel(image, x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }

    return true;
}

bool fossil_image_draw_rect(fossil_image_t *image, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint8_t *color, bool filled) {
    if (!image || !color)
        return false;

    if (filled) {
        for (uint32_t j = y; j < y + height && j < image->height; ++j) {
            for (uint32_t i = x; i < x + width && i < image->width; ++i)
                fossil_draw_set_pixel(image, i, j, color);
        }
    } else {
        fossil_image_draw_line(image, x, y, x + width - 1, y, color);
        fossil_image_draw_line(image, x, y, x, y + height - 1, color);
        fossil_image_draw_line(image, x + width - 1, y, x + width - 1, y + height - 1, color);
        fossil_image_draw_line(image, x, y + height - 1, x + width - 1, y + height - 1, color);
    }

    return true;
}

bool fossil_image_draw_circle(fossil_image_t *image, uint32_t cx, uint32_t cy, uint32_t radius, const uint8_t *color, bool filled) {
    if (!image || !color)
        return false;

    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (y >= x) {
        if (filled) {
            for (int i = cx - x; i <= (int)(cx + x); ++i) {
                fossil_draw_set_pixel(image, i, cy - y, color);
                fossil_draw_set_pixel(image, i, cy + y, color);
            }
            for (int i = cx - y; i <= (int)(cx + y); ++i) {
                fossil_draw_set_pixel(image, i, cy - x, color);
                fossil_draw_set_pixel(image, i, cy + x, color);
            }
        } else {
            fossil_draw_set_pixel(image, cx + x, cy + y, color);
            fossil_draw_set_pixel(image, cx - x, cy + y, color);
            fossil_draw_set_pixel(image, cx + x, cy - y, color);
            fossil_draw_set_pixel(image, cx - x, cy - y, color);
            fossil_draw_set_pixel(image, cx + y, cy + x, color);
            fossil_draw_set_pixel(image, cx - y, cy + x, color);
            fossil_draw_set_pixel(image, cx + y, cy - x, color);
            fossil_draw_set_pixel(image, cx - y, cy - x, color);
        }

        ++x;
        if (d > 0) {
            --y;
            d += 4 * (x - y) + 10;
        } else {
            d += 4 * x + 6;
        }
    }

    return true;
}

bool fossil_image_draw_fill(fossil_image_t *image, const uint8_t *color) {
    if (!image || !image->data || !color)
        return false;

    size_t npixels = (size_t)image->width * image->height;
    for (size_t i = 0; i < npixels; ++i) {
        size_t idx = i * image->channels;
        for (uint32_t c = 0; c < image->channels; ++c)
            image->data[idx + c] = color[c];
    }
    return true;
}

// ======================================================
// Minimal bitmap font (5x7 per character)
// ======================================================

static const uint8_t FONT_5x7[96][5] = {
    // Basic ASCII set (space to ~)
    // (Can be replaced or extended)
    {0x00,0x00,0x00,0x00,0x00}, // ' '
    {0x00,0x00,0x5F,0x00,0x00}, // '!'
    {0x00,0x03,0x00,0x03,0x00}, // '"'
    // ... truncated for brevity, you can fill the rest later
};

bool fossil_image_draw_text(fossil_image_t *image, uint32_t x, uint32_t y, const char *text, const uint8_t *color) {
    if (!image || !text || !color)
        return false;

    uint32_t start_x = x;
    for (const char *p = text; *p; ++p) {
        unsigned char ch = *p;
        if (ch < 32 || ch > 127) ch = '?';
        const uint8_t *glyph = FONT_5x7[ch - 32];
        for (int col = 0; col < 5; ++col) {
            uint8_t bits = glyph[col];
            for (int row = 0; row < 7; ++row) {
                if (bits & (1 << row))
                    fossil_draw_set_pixel(image, x + col, y + row, color);
            }
        }
        x += 6; // spacing
    }
    return true;
}
