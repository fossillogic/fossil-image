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

static inline void fossil_draw_set_pixel(fossil_image_t *image, uint32_t x, uint32_t y, const void *color) {
    if (!image) return;
    if (x >= image->width || y >= image->height) return;

    size_t idx = (y * image->width + x) * image->channels;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            if (!image->data) return;
            for (uint32_t c = 0; c < image->channels; ++c)
                image->data[idx + c] = ((const uint8_t *)color)[c];
            break;

        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            if (!image->data) return;
            {
                uint16_t *data16 = (uint16_t *)image->data;
                size_t idx16 = (y * image->width + x) * image->channels;
                for (uint32_t c = 0; c < image->channels; ++c)
                    data16[idx16 + c] = ((const uint16_t *)color)[c];
            }
            break;

        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            if (!image->fdata) return;
            {
                size_t idxf = (y * image->width + x) * image->channels;
                for (uint32_t c = 0; c < image->channels; ++c)
                    image->fdata[idxf + c] = ((const float *)color)[c];
            }
            break;

        default:
            // Unsupported format
            break;
    }
}

bool fossil_image_draw_pixel(fossil_image_t *image, uint32_t x, uint32_t y, const void *color) {
    if (!image || !color)
        return false;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            fossil_draw_set_pixel(image, x, y, (const uint8_t *)color);
            break;

        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            fossil_draw_set_pixel(image, x, y, (const uint16_t *)color);
            break;

        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            fossil_draw_set_pixel(image, x, y, (const float *)color);
            break;

        default:
            return false;
    }
    return true;
}

bool fossil_image_draw_line(fossil_image_t *image, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, const void *color) {
    if (!image || !color)
        return false;

    int dx = abs((int)x1 - (int)x0);
    int dy = abs((int)y1 - (int)y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        switch (image->format) {
            case FOSSIL_PIXEL_FORMAT_GRAY8:
            case FOSSIL_PIXEL_FORMAT_RGB24:
            case FOSSIL_PIXEL_FORMAT_RGBA32:
            case FOSSIL_PIXEL_FORMAT_INDEXED8:
            case FOSSIL_PIXEL_FORMAT_YUV24:
                fossil_draw_set_pixel(image, x0, y0, (const uint8_t *)color);
                break;
            case FOSSIL_PIXEL_FORMAT_GRAY16:
            case FOSSIL_PIXEL_FORMAT_RGB48:
            case FOSSIL_PIXEL_FORMAT_RGBA64:
                fossil_draw_set_pixel(image, x0, y0, (const uint16_t *)color);
                break;
            case FOSSIL_PIXEL_FORMAT_FLOAT32:
            case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
            case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
                fossil_draw_set_pixel(image, x0, y0, (const float *)color);
                break;
            default:
                // Unsupported format
                return false;
        }
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }

    return true;
}

bool fossil_image_draw_rect(fossil_image_t *image, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void *color, bool filled) {
    if (!image || !color)
        return false;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            if (filled) {
                for (uint32_t j = y; j < y + height && j < image->height; ++j) {
                    for (uint32_t i = x; i < x + width && i < image->width; ++i)
                        fossil_draw_set_pixel(image, i, j, (const uint8_t *)color);
                }
            } else {
                fossil_image_draw_line(image, x, y, x + width - 1, y, color);
                fossil_image_draw_line(image, x, y, x, y + height - 1, color);
                fossil_image_draw_line(image, x + width - 1, y, x + width - 1, y + height - 1, color);
                fossil_image_draw_line(image, x, y + height - 1, x + width - 1, y + height - 1, color);
            }
            break;

        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            if (filled) {
                for (uint32_t j = y; j < y + height && j < image->height; ++j) {
                    for (uint32_t i = x; i < x + width && i < image->width; ++i)
                        fossil_draw_set_pixel(image, i, j, (const uint16_t *)color);
                }
            } else {
                fossil_image_draw_line(image, x, y, x + width - 1, y, color);
                fossil_image_draw_line(image, x, y, x, y + height - 1, color);
                fossil_image_draw_line(image, x + width - 1, y, x + width - 1, y + height - 1, color);
                fossil_image_draw_line(image, x, y + height - 1, x + width - 1, y + height - 1, color);
            }
            break;

        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            if (filled) {
                for (uint32_t j = y; j < y + height && j < image->height; ++j) {
                    for (uint32_t i = x; i < x + width && i < image->width; ++i)
                        fossil_draw_set_pixel(image, i, j, (const float *)color);
                }
            } else {
                fossil_image_draw_line(image, x, y, x + width - 1, y, color);
                fossil_image_draw_line(image, x, y, x, y + height - 1, color);
                fossil_image_draw_line(image, x + width - 1, y, x + width - 1, y + height - 1, color);
                fossil_image_draw_line(image, x, y + height - 1, x + width - 1, y + height - 1, color);
            }
            break;

        default:
            return false;
    }

    return true;
}

bool fossil_image_draw_circle(fossil_image_t *image, uint32_t cx, uint32_t cy, uint32_t radius, const void *color, bool filled) {
    if (!image || !color)
        return false;

    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (y >= x) {
        switch (image->format) {
            case FOSSIL_PIXEL_FORMAT_GRAY8:
            case FOSSIL_PIXEL_FORMAT_RGB24:
            case FOSSIL_PIXEL_FORMAT_RGBA32:
            case FOSSIL_PIXEL_FORMAT_INDEXED8:
            case FOSSIL_PIXEL_FORMAT_YUV24:
                if (filled) {
                    for (int i = cx - x; i <= (int)(cx + x); ++i) {
                        fossil_draw_set_pixel(image, i, cy - y, (const uint8_t *)color);
                        fossil_draw_set_pixel(image, i, cy + y, (const uint8_t *)color);
                    }
                    for (int i = cx - y; i <= (int)(cx + y); ++i) {
                        fossil_draw_set_pixel(image, i, cy - x, (const uint8_t *)color);
                        fossil_draw_set_pixel(image, i, cy + x, (const uint8_t *)color);
                    }
                } else {
                    fossil_draw_set_pixel(image, cx + x, cy + y, (const uint8_t *)color);
                    fossil_draw_set_pixel(image, cx - x, cy + y, (const uint8_t *)color);
                    fossil_draw_set_pixel(image, cx + x, cy - y, (const uint8_t *)color);
                    fossil_draw_set_pixel(image, cx - x, cy - y, (const uint8_t *)color);
                    fossil_draw_set_pixel(image, cx + y, cy + x, (const uint8_t *)color);
                    fossil_draw_set_pixel(image, cx - y, cy + x, (const uint8_t *)color);
                    fossil_draw_set_pixel(image, cx + y, cy - x, (const uint8_t *)color);
                    fossil_draw_set_pixel(image, cx - y, cy - x, (const uint8_t *)color);
                }
                break;

            case FOSSIL_PIXEL_FORMAT_GRAY16:
            case FOSSIL_PIXEL_FORMAT_RGB48:
            case FOSSIL_PIXEL_FORMAT_RGBA64:
                if (filled) {
                    for (int i = cx - x; i <= (int)(cx + x); ++i) {
                        fossil_draw_set_pixel(image, i, cy - y, (const uint16_t *)color);
                        fossil_draw_set_pixel(image, i, cy + y, (const uint16_t *)color);
                    }
                    for (int i = cx - y; i <= (int)(cx + y); ++i) {
                        fossil_draw_set_pixel(image, i, cy - x, (const uint16_t *)color);
                        fossil_draw_set_pixel(image, i, cy + x, (const uint16_t *)color);
                    }
                } else {
                    fossil_draw_set_pixel(image, cx + x, cy + y, (const uint16_t *)color);
                    fossil_draw_set_pixel(image, cx - x, cy + y, (const uint16_t *)color);
                    fossil_draw_set_pixel(image, cx + x, cy - y, (const uint16_t *)color);
                    fossil_draw_set_pixel(image, cx - x, cy - y, (const uint16_t *)color);
                    fossil_draw_set_pixel(image, cx + y, cy + x, (const uint16_t *)color);
                    fossil_draw_set_pixel(image, cx - y, cy + x, (const uint16_t *)color);
                    fossil_draw_set_pixel(image, cx + y, cy - x, (const uint16_t *)color);
                    fossil_draw_set_pixel(image, cx - y, cy - x, (const uint16_t *)color);
                }
                break;

            case FOSSIL_PIXEL_FORMAT_FLOAT32:
            case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
            case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
                if (filled) {
                    for (int i = cx - x; i <= (int)(cx + x); ++i) {
                        fossil_draw_set_pixel(image, i, cy - y, (const float *)color);
                        fossil_draw_set_pixel(image, i, cy + y, (const float *)color);
                    }
                    for (int i = cx - y; i <= (int)(cx + y); ++i) {
                        fossil_draw_set_pixel(image, i, cy - x, (const float *)color);
                        fossil_draw_set_pixel(image, i, cy + x, (const float *)color);
                    }
                } else {
                    fossil_draw_set_pixel(image, cx + x, cy + y, (const float *)color);
                    fossil_draw_set_pixel(image, cx - x, cy + y, (const float *)color);
                    fossil_draw_set_pixel(image, cx + x, cy - y, (const float *)color);
                    fossil_draw_set_pixel(image, cx - x, cy - y, (const float *)color);
                    fossil_draw_set_pixel(image, cx + y, cy + x, (const float *)color);
                    fossil_draw_set_pixel(image, cx - y, cy + x, (const float *)color);
                    fossil_draw_set_pixel(image, cx + y, cy - x, (const float *)color);
                    fossil_draw_set_pixel(image, cx - y, cy - x, (const float *)color);
                }
                break;

            default:
                // Unsupported format
                return false;
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

bool fossil_image_draw_fill(fossil_image_t *image, const void *color) {
    if (!image || !color)
        return false;

    size_t npixels = (size_t)image->width * image->height;

    switch (image->format) {
        case FOSSIL_PIXEL_FORMAT_GRAY8:
        case FOSSIL_PIXEL_FORMAT_RGB24:
        case FOSSIL_PIXEL_FORMAT_RGBA32:
        case FOSSIL_PIXEL_FORMAT_INDEXED8:
        case FOSSIL_PIXEL_FORMAT_YUV24:
            if (!image->data) return false;
            for (size_t i = 0; i < npixels; ++i) {
                size_t idx = i * image->channels;
                for (uint32_t c = 0; c < image->channels; ++c)
                    image->data[idx + c] = ((const uint8_t *)color)[c];
            }
            break;

        case FOSSIL_PIXEL_FORMAT_GRAY16:
        case FOSSIL_PIXEL_FORMAT_RGB48:
        case FOSSIL_PIXEL_FORMAT_RGBA64:
            if (!image->data) return false;
            {
                uint16_t *data16 = (uint16_t *)image->data;
                for (size_t i = 0; i < npixels; ++i) {
                    size_t idx = i * image->channels;
                    for (uint32_t c = 0; c < image->channels; ++c)
                        data16[idx + c] = ((const uint16_t *)color)[c];
                }
            }
            break;

        case FOSSIL_PIXEL_FORMAT_FLOAT32:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
            if (!image->fdata) return false;
            for (size_t i = 0; i < npixels; ++i) {
                size_t idx = i * image->channels;
                for (uint32_t c = 0; c < image->channels; ++c)
                    image->fdata[idx + c] = ((const float *)color)[c];
            }
            break;

        default:
            return false;
    }
    return true;
}

// ======================================================
// Minimal bitmap font (5x7 per character)
// ======================================================

static const uint8_t FONT_5x7[96][5] = {
    // Space to '~' (ASCII 32-127)
    {0x00,0x00,0x00,0x00,0x00}, // ' '
    {0x00,0x00,0x5F,0x00,0x00}, // '!'
    {0x00,0x03,0x00,0x03,0x00}, // '"'
    {0x14,0x7F,0x14,0x7F,0x14}, // '#'
    {0x24,0x2A,0x7F,0x2A,0x12}, // '$'
    {0x23,0x13,0x08,0x64,0x62}, // '%'
    {0x36,0x49,0x55,0x22,0x50}, // '&'
    {0x00,0x05,0x03,0x00,0x00}, // '''
    {0x00,0x1C,0x22,0x41,0x00}, // '('
    {0x00,0x41,0x22,0x1C,0x00}, // ')'
    {0x14,0x08,0x3E,0x08,0x14}, // '*'
    {0x08,0x08,0x3E,0x08,0x08}, // '+'
    {0x00,0x50,0x30,0x00,0x00}, // ','
    {0x08,0x08,0x08,0x08,0x08}, // '-'
    {0x00,0x60,0x60,0x00,0x00}, // '.'
    {0x20,0x10,0x08,0x04,0x02}, // '/'
    {0x3E,0x51,0x49,0x45,0x3E}, // '0'
    {0x00,0x42,0x7F,0x40,0x00}, // '1'
    {0x42,0x61,0x51,0x49,0x46}, // '2'
    {0x21,0x41,0x45,0x4B,0x31}, // '3'
    {0x18,0x14,0x12,0x7F,0x10}, // '4'
    {0x27,0x45,0x45,0x45,0x39}, // '5'
    {0x3C,0x4A,0x49,0x49,0x30}, // '6'
    {0x01,0x71,0x09,0x05,0x03}, // '7'
    {0x36,0x49,0x49,0x49,0x36}, // '8'
    {0x06,0x49,0x49,0x29,0x1E}, // '9'
    {0x00,0x36,0x36,0x00,0x00}, // ':'
    {0x00,0x56,0x36,0x00,0x00}, // ';'
    {0x08,0x14,0x22,0x41,0x00}, // '<'
    {0x14,0x14,0x14,0x14,0x14}, // '='
    {0x00,0x41,0x22,0x14,0x08}, // '>'
    {0x02,0x01,0x51,0x09,0x06}, // '?'
    {0x32,0x49,0x79,0x41,0x3E}, // '@'
    {0x7E,0x11,0x11,0x11,0x7E}, // 'A'
    {0x7F,0x49,0x49,0x49,0x36}, // 'B'
    {0x3E,0x41,0x41,0x41,0x22}, // 'C'
    {0x7F,0x41,0x41,0x22,0x1C}, // 'D'
    {0x7F,0x49,0x49,0x49,0x41}, // 'E'
    {0x7F,0x09,0x09,0x09,0x01}, // 'F'
    {0x3E,0x41,0x49,0x49,0x7A}, // 'G'
    {0x7F,0x08,0x08,0x08,0x7F}, // 'H'
    {0x00,0x41,0x7F,0x41,0x00}, // 'I'
    {0x20,0x40,0x41,0x3F,0x01}, // 'J'
    {0x7F,0x08,0x14,0x22,0x41}, // 'K'
    {0x7F,0x40,0x40,0x40,0x40}, // 'L'
    {0x7F,0x02,0x0C,0x02,0x7F}, // 'M'
    {0x7F,0x04,0x08,0x10,0x7F}, // 'N'
    {0x3E,0x41,0x41,0x41,0x3E}, // 'O'
    {0x7F,0x09,0x09,0x09,0x06}, // 'P'
    {0x3E,0x41,0x51,0x21,0x5E}, // 'Q'
    {0x7F,0x09,0x19,0x29,0x46}, // 'R'
    {0x46,0x49,0x49,0x49,0x31}, // 'S'
    {0x01,0x01,0x7F,0x01,0x01}, // 'T'
    {0x3F,0x40,0x40,0x40,0x3F}, // 'U'
    {0x1F,0x20,0x40,0x20,0x1F}, // 'V'
    {0x3F,0x40,0x38,0x40,0x3F}, // 'W'
    {0x63,0x14,0x08,0x14,0x63}, // 'X'
    {0x07,0x08,0x70,0x08,0x07}, // 'Y'
    {0x61,0x51,0x49,0x45,0x43}, // 'Z'
    {0x00,0x7F,0x41,0x41,0x00}, // '['
    {0x02,0x04,0x08,0x10,0x20}, // '\'
    {0x00,0x41,0x41,0x7F,0x00}, // ']'
    {0x04,0x02,0x01,0x02,0x04}, // '^'
    {0x40,0x40,0x40,0x40,0x40}, // '_'
    {0x00,0x01,0x02,0x04,0x00}, // '`'
    {0x20,0x54,0x54,0x54,0x78}, // 'a'
    {0x7F,0x48,0x44,0x44,0x38}, // 'b'
    {0x38,0x44,0x44,0x44,0x20}, // 'c'
    {0x38,0x44,0x44,0x48,0x7F}, // 'd'
    {0x38,0x54,0x54,0x54,0x18}, // 'e'
    {0x08,0x7E,0x09,0x01,0x02}, // 'f'
    {0x0C,0x52,0x52,0x52,0x3E}, // 'g'
    {0x7F,0x08,0x04,0x04,0x78}, // 'h'
    {0x00,0x44,0x7D,0x40,0x00}, // 'i'
    {0x20,0x40,0x44,0x3D,0x00}, // 'j'
    {0x7F,0x10,0x28,0x44,0x00}, // 'k'
    {0x00,0x41,0x7F,0x40,0x00}, // 'l'
    {0x7C,0x04,0x18,0x04,0x78}, // 'm'
    {0x7C,0x08,0x04,0x04,0x78}, // 'n'
    {0x38,0x44,0x44,0x44,0x38}, // 'o'
    {0x7C,0x14,0x14,0x14,0x08}, // 'p'
    {0x08,0x14,0x14,0x18,0x7C}, // 'q'
    {0x7C,0x08,0x04,0x04,0x08}, // 'r'
    {0x48,0x54,0x54,0x54,0x20}, // 's'
    {0x04,0x3F,0x44,0x40,0x20}, // 't'
    {0x3C,0x40,0x40,0x20,0x7C}, // 'u'
    {0x1C,0x20,0x40,0x20,0x1C}, // 'v'
    {0x3C,0x40,0x30,0x40,0x3C}, // 'w'
    {0x44,0x28,0x10,0x28,0x44}, // 'x'
    {0x0C,0x50,0x50,0x50,0x3C}, // 'y'
    {0x44,0x64,0x54,0x4C,0x44}, // 'z'
    {0x00,0x08,0x36,0x41,0x00}, // '{'
    {0x00,0x00,0x7F,0x00,0x00}, // '|'
    {0x00,0x41,0x36,0x08,0x00}, // '}'
    {0x08,0x08,0x2A,0x1C,0x08}, // '~'
};

bool fossil_image_draw_text(fossil_image_t *image, uint32_t x, uint32_t y, const char *text, const void *color) {
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
                if (bits & (1 << row)) {
                    switch (image->format) {
                        case FOSSIL_PIXEL_FORMAT_GRAY8:
                        case FOSSIL_PIXEL_FORMAT_RGB24:
                        case FOSSIL_PIXEL_FORMAT_RGBA32:
                        case FOSSIL_PIXEL_FORMAT_INDEXED8:
                        case FOSSIL_PIXEL_FORMAT_YUV24:
                            fossil_draw_set_pixel(image, x + col, y + row, (const uint8_t *)color);
                            break;
                        case FOSSIL_PIXEL_FORMAT_GRAY16:
                        case FOSSIL_PIXEL_FORMAT_RGB48:
                        case FOSSIL_PIXEL_FORMAT_RGBA64:
                            fossil_draw_set_pixel(image, x + col, y + row, (const uint16_t *)color);
                            break;
                        case FOSSIL_PIXEL_FORMAT_FLOAT32:
                        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGB:
                        case FOSSIL_PIXEL_FORMAT_FLOAT32_RGBA:
                            fossil_draw_set_pixel(image, x + col, y + row, (const float *)color);
                            break;
                        default:
                            // Unsupported format
                            break;
                    }
                }
            }
        }
        x += 6; // spacing
    }
    return true;
}
