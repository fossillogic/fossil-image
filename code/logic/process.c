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
#include "fossil/image/process.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ---------- Helper macros / inline functions ----------
static inline uint8_t clamp_u8(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

static inline float clampf(float v, float a, float b) {
    if (v < a) return a;
    if (v > b) return b;
    return v;
}

/* Accessors - change these if your fossil_image_t is different */
#define IMG_W(img) ((img)->width)
#define IMG_H(img) ((img)->height)
#define IMG_C(img) ((img)->channels)
#define IMG_PIX(img, x, y, c) ((img)->pixels[((size_t)(y) * IMG_W(img) + (x)) * IMG_C(img) + (c)])

/* Create a temporary buffer sized for image */
static uint8_t *alloc_temp(const fossil_image_t *img) {
    size_t sz = (size_t)IMG_W(img) * IMG_H(img) * IMG_C(img);
    return (uint8_t*)malloc(sz);
}

/* Safe fallback if pixels null */
static bool validate_image(const fossil_image_t *img) {
    if (!img) return false;
    if (!img->pixels) return false;
    if (IMG_W(img) == 0 || IMG_H(img) == 0) return false;
    if (IMG_C(img) < 1 || IMG_C(img) > 4) return false;
    return true;
}

/* Convert RGB to luminance (rec. 601) */
static inline uint8_t rgb_to_luma_u8(uint8_t r, uint8_t g, uint8_t b) {
    float y = 0.299f * r + 0.587f * g + 0.114f * b;
    return clamp_u8((int)roundf(y));
}

// -------------------- Implementations --------------------

bool fossil_image_process_filter(fossil_image_t *image, fossil_image_filter_e filter) {
    if (!validate_image(image)) return false;
    if (filter == FOSSIL_IMAGE_FILTER_NONE) return true;

    size_t w = IMG_W(image), h = IMG_H(image), c = IMG_C(image);
    uint8_t *tmp = alloc_temp(image);
    if (!tmp) return false;
    memcpy(tmp, image->pixels, (size_t)w * h * c);

    size_t x, y;
    if (filter == FOSSIL_IMAGE_FILTER_GRAYSCALE) {
        for (y = 0; y < h; ++y) {
            for (x = 0; x < w; ++x) {
                uint8_t r = tmp[(y * w + x) * c + 0];
                uint8_t g = (c > 1) ? tmp[(y * w + x) * c + 1] : r;
                uint8_t b = (c > 2) ? tmp[(y * w + x) * c + 2] : r;
                uint8_t yv = rgb_to_luma_u8(r,g,b);
                image->pixels[(y * w + x) * c + 0] = yv;
                if (c > 1) image->pixels[(y * w + x) * c + 1] = yv;
                if (c > 2) image->pixels[(y * w + x) * c + 2] = yv;
            }
        }
    } else if (filter == FOSSIL_IMAGE_FILTER_NEGATIVE) {
        size_t total = w * h * c;
        for (size_t i = 0; i < total; ++i) image->pixels[i] = 255u - tmp[i];
    } else if (filter == FOSSIL_IMAGE_FILTER_SEPIA) {
        for (y = 0; y < h; ++y) {
            for (x = 0; x < w; ++x) {
                uint8_t r = tmp[(y * w + x) * c + 0];
                uint8_t g = (c > 1) ? tmp[(y * w + x) * c + 1] : r;
                uint8_t b = (c > 2) ? tmp[(y * w + x) * c + 2] : r;
                int tr = (int)roundf(0.393f*r + 0.769f*g + 0.189f*b);
                int tg = (int)roundf(0.349f*r + 0.686f*g + 0.168f*b);
                int tb = (int)roundf(0.272f*r + 0.534f*g + 0.131f*b);
                image->pixels[(y * w + x) * c + 0] = clamp_u8(tr);
                if (c > 1) image->pixels[(y * w + x) * c + 1] = clamp_u8(tg);
                if (c > 2) image->pixels[(y * w + x) * c + 2] = clamp_u8(tb);
            }
        }
    } else if (filter == FOSSIL_IMAGE_FILTER_BLUR) {
        // simple 3x3 box blur
        for (y = 0; y < h; ++y) {
            for (x = 0; x < w; ++x) {
                for (size_t ch = 0; ch < c; ++ch) {
                    int sum = 0, count = 0;
                    for (int ky = -1; ky <= 1; ++ky) {
                        int yy = (int)y + ky;
                        if (yy < 0 || yy >= (int)h) continue;
                        for (int kx = -1; kx <= 1; ++kx) {
                            int xx = (int)x + kx;
                            if (xx < 0 || xx >= (int)w) continue;
                            sum += tmp[(yy * w + xx) * c + ch];
                            ++count;
                        }
                    }
                    image->pixels[(y * w + x) * c + ch] = clamp_u8(sum / (count ? count : 1));
                }
            }
        }
    } else if (filter == FOSSIL_IMAGE_FILTER_SHARPEN) {
        // simple unsharp mask: original + (original - blurred)
        // compute blurred into tmp2
        uint8_t *tmp2 = (uint8_t*)malloc((size_t)w * h * c);
        if (!tmp2) { free(tmp); return false; }
        // reuse same 3x3 box blur into tmp2
        for (y = 0; y < h; ++y) {
            for (x = 0; x < w; ++x) {
                for (size_t ch = 0; ch < c; ++ch) {
                    int sum = 0, count = 0;
                    for (int ky = -1; ky <= 1; ++ky) {
                        int yy = (int)y + ky;
                        if (yy < 0 || yy >= (int)h) continue;
                        for (int kx = -1; kx <= 1; ++kx) {
                            int xx = (int)x + kx;
                            if (xx < 0 || xx >= (int)w) continue;
                            sum += tmp[(yy * w + xx) * c + ch];
                            ++count;
                        }
                    }
                    tmp2[(y * w + x) * c + ch] = clamp_u8(sum / (count ? count : 1));
                }
            }
        }
        // combine: out = orig + (orig - blur) * 1.0
        for (size_t i = 0; i < (size_t)w * h * c; ++i) {
            int v = (int)tmp[i] + ((int)tmp[i] - (int)tmp2[i]);
            image->pixels[i] = clamp_u8(v);
        }
        free(tmp2);
    } else if (filter == FOSSIL_IMAGE_FILTER_EDGE) {
        // basic Sobel magnitude to grayscale output
        // produce grayscale in-place (keeps channels)
        // temporary buffer for gray
        uint8_t *gray = (uint8_t*)malloc((size_t)w * h);
        if (!gray) { free(tmp); return false; }
        for (y = 0; y < h; ++y) {
            for (x = 0; x < w; ++x) {
                uint8_t r = tmp[(y*w + x)*c + 0];
                uint8_t g = (c > 1) ? tmp[(y*w + x)*c + 1] : r;
                uint8_t b = (c > 2) ? tmp[(y*w + x)*c + 2] : r;
                gray[y*w + x] = rgb_to_luma_u8(r,g,b);
            }
        }
        int sx[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
        int sy[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};
        for (y = 0; y < h; ++y) {
            for (x = 0; x < w; ++x) {
                int gx = 0, gy = 0;
                for (int ky = -1; ky <= 1; ++ky) {
                    int yy = (int)y + ky;
                    if (yy < 0 || yy >= (int)h) continue;
                    for (int kx = -1; kx <= 1; ++kx) {
                        int xx = (int)x + kx;
                        if (xx < 0 || xx >= (int)w) continue;
                        int v = gray[yy*w + xx];
                        gx += v * sx[ky+1][kx+1];
                        gy += v * sy[ky+1][kx+1];
                    }
                }
                int mag = (int)roundf(sqrtf((float)(gx*gx + gy*gy)));
                uint8_t outv = clamp_u8(mag);
                for (size_t ch = 0; ch < c; ++ch) image->pixels[(y*w + x)*c + ch] = outv;
            }
        }
        free(gray);
    } else {
        // CUSTOM or unimplemented
        free(tmp);
        return false;
    }

    free(tmp);
    return true;
}

bool fossil_image_process_convolve(fossil_image_t *image, const float *kernel, size_t kw, size_t kh) {
    if (!validate_image(image) || !kernel) return false;
    if (kw == 0 || kh == 0) return false;

    size_t w = IMG_W(image), h = IMG_H(image), c = IMG_C(image);
    uint8_t *src = alloc_temp(image);
    if (!src) return false;
    memcpy(src, image->pixels, (size_t)w*h*c);

    int kcx = (int)kw/2;
    int kcy = (int)kh/2;

    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x) {
            for (size_t ch = 0; ch < c; ++ch) {
                float sum = 0.0f;
                for (int ky = 0; ky < (int)kh; ++ky) {
                    int yy = (int)y + ky - kcy;
                    if (yy < 0 || yy >= (int)h) continue;
                    for (int kx = 0; kx < (int)kw; ++kx) {
                        int xx = (int)x + kx - kcx;
                        if (xx < 0 || xx >= (int)w) continue;
                        float kval = kernel[ky * kw + kx];
                        sum += kval * (float)src[(yy * w + xx) * c + ch];
                    }
                }
                image->pixels[(y * w + x) * c + ch] = clamp_u8((int)roundf(sum));
            }
        }
    }

    free(src);
    return true;
}

bool fossil_image_process_adjust(fossil_image_t *image, float brightness, float contrast, float saturation) {
    // brightness: -1.0..+1.0 (adds fraction of 255)
    // contrast: -1.0..+1.0 (scale factor around 0.5 -> 0..2)
    // saturation: -1.0..+1.0
    if (!validate_image(image)) return false;
    size_t w = IMG_W(image), h = IMG_H(image), c = IMG_C(image);
    uint8_t *src = alloc_temp(image);
    if (!src) return false;
    memcpy(src, image->pixels, (size_t)w*h*c);

    float badd = brightness * 255.0f;
    float cscale = 1.0f + contrast; // simple mapping
    float sscale = 1.0f + saturation;

    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x) {
            size_t idx = (y*w + x)*c;
            uint8_t r = src[idx + 0];
            uint8_t g = (c > 1) ? src[idx + 1] : r;
            uint8_t b = (c > 2) ? src[idx + 2] : r;
            // contrast: move away from 128
            float rf = ((float)r - 128.0f) * cscale + 128.0f + badd;
            float gf = ((float)g - 128.0f) * cscale + 128.0f + badd;
            float bf = ((float)b - 128.0f) * cscale + 128.0f + badd;

            // convert to HSL-lite to adjust saturation (approx)
            float maxv = fmaxf(fmaxf(rf, gf), bf);
            float minv = fminf(fminf(rf, gf), bf);
            float l = (maxv + minv) * 0.5f;
            float chroma = maxv - minv;
            float sat = 0.0f;
            if (chroma > 1e-6f) sat = chroma / (1.0f - fabsf(2.0f * (l/255.0f) - 1.0f));
            sat *= sscale;
            // Simple saturation scaling toward/from luma
            float yv = 0.299f*rf + 0.587f*gf + 0.114f*bf;
            rf = yv + (rf - yv) * sscale;
            gf = yv + (gf - yv) * sscale;
            bf = yv + (bf - yv) * sscale;

            image->pixels[idx + 0] = clamp_u8((int)roundf(rf));
            if (c > 1) image->pixels[idx + 1] = clamp_u8((int)roundf(gf));
            if (c > 2) image->pixels[idx + 2] = clamp_u8((int)roundf(bf));
        }
    }

    free(src);
    return true;
}

bool fossil_image_process_flip(fossil_image_t *image, bool horizontal, bool vertical) {
    if (!validate_image(image)) return false;
    if (!horizontal && !vertical) return true;
    size_t w = IMG_W(image), h = IMG_H(image), c = IMG_C(image);
    uint8_t *tmp = alloc_temp(image);
    if (!tmp) return false;
    memcpy(tmp, image->pixels, (size_t)w*h*c);

    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x) {
            size_t sx = horizontal ? (w - 1 - x) : x;
            size_t sy = vertical ? (h - 1 - y) : y;
            memcpy(&image->pixels[(y*w + x)*c],
                   &tmp[(sy*w + sx)*c],
                   c);
        }
    }

    free(tmp);
    return true;
}

bool fossil_image_process_rotate(fossil_image_t *image, float degrees) {
    if (!validate_image(image)) return false;
    // Rotation implemented with nearest-neighbor and expands bounding box to fit rotated image.
    float rad = -degrees * (float)M_PI / 180.0f; // clockwise degrees -> negative radians for standard transform
    float cs = cosf(rad);
    float sn = sinf(rad);

    uint32_t w = IMG_W(image), h = IMG_H(image);
    uint8_t c = IMG_C(image);

    // compute new bounding box size
    float cx = (w - 1) * 0.5f, cy = (h - 1) * 0.5f;
    float corners[4][2] = {
        {-cx, -cy},
        { cx, -cy},
        {-cx,  cy},
        { cx,  cy}
    };
    float minx = 1e9f, miny = 1e9f, maxx = -1e9f, maxy = -1e9f;
    for (int i = 0; i < 4; ++i) {
        float x = corners[i][0], y = corners[i][1];
        float rx = x * cs - y * sn;
        float ry = x * sn + y * cs;
        if (rx < minx) minx = rx;
        if (rx > maxx) maxx = rx;
        if (ry < miny) miny = ry;
        if (ry > maxy) maxy = ry;
    }
    uint32_t nw = (uint32_t)ceilf(maxx - minx + 1.0f);
    uint32_t nh = (uint32_t)ceilf(maxy - miny + 1.0f);

    uint8_t *src = alloc_temp(image);
    if (!src) return false;
    memcpy(src, image->pixels, (size_t)w*h*c);

    uint8_t *out = (uint8_t*)malloc((size_t)nw * nh * c);
    if (!out) { free(src); return false; }
    // fill with transparent black or black
    memset(out, 0, (size_t)nw * nh * c);

    float ncx = (nw - 1) * 0.5f, ncy = (nh - 1) * 0.5f;
    for (uint32_t y = 0; y < nh; ++y) {
        for (uint32_t x = 0; x < nw; ++x) {
            // target coordinates relative to center
            float tx = (float)x - ncx;
            float ty = (float)y - ncy;
            // inverse rotate to sample source
            float sx_f = tx * cs + ty * sn;
            float sy_f = -tx * sn + ty * cs;
            float srcx = sx_f + cx;
            float srcy = sy_f + cy;
            int ix = (int)floorf(srcx + 0.5f);
            int iy = (int)floorf(srcy + 0.5f);
            if (ix >= 0 && ix < (int)w && iy >= 0 && iy < (int)h) {
                memcpy(&out[(y * nw + x) * c], &src[(iy * w + ix) * c], c);
            } else {
                // leave as zero
            }
        }
    }

    // replace image pixels and dims
    free(image->pixels);
    image->pixels = out;
    image->width = nw;
    image->height = nh;

    free(src);
    return true;
}

bool fossil_image_process_resize(fossil_image_t *image, uint32_t new_width, uint32_t new_height, fossil_image_scale_e method) {
    if (!validate_image(image)) return false;
    if (new_width == 0 || new_height == 0) return false;

    uint32_t w = IMG_W(image), h = IMG_H(image);
    uint8_t c = IMG_C(image);
    uint8_t *src = alloc_temp(image);
    if (!src) return false;
    memcpy(src, image->pixels, (size_t)w * h * c);

    uint8_t *out = (uint8_t*)malloc((size_t)new_width * new_height * c);
    if (!out) { free(src); return false; }

    if (method == FOSSIL_IMAGE_SCALE_NEAREST) {
        for (uint32_t y = 0; y < new_height; ++y) {
            for (uint32_t x = 0; x < new_width; ++x) {
                float fx = (float)x * (float)w / (float)new_width;
                float fy = (float)y * (float)h / (float)new_height;
                uint32_t sx = (uint32_t)clampf((float)floorf(fx + 0.5f), 0.0f, (float)(w-1));
                uint32_t sy = (uint32_t)clampf((float)floorf(fy + 0.5f), 0.0f, (float)(h-1));
                memcpy(&out[(y * new_width + x) * c], &src[(sy * w + sx) * c], c);
            }
        }
    } else {
        // Simple bilinear
        for (uint32_t y = 0; y < new_height; ++y) {
            for (uint32_t x = 0; x < new_width; ++x) {
                float fx = (float)x * (float)(w - 1) / (float)(new_width - 1);
                float fy = (float)y * (float)(h - 1) / (float)(new_height - 1);
                int x0 = (int)floorf(fx), x1 = x0 + 1;
                int y0 = (int)floorf(fy), y1 = y0 + 1;
                float sx = fx - x0, sy = fy - y0;
                if (x1 >= (int)w) x1 = (int)w - 1;
                if (y1 >= (int)h) y1 = (int)h - 1;
                for (size_t ch = 0; ch < c; ++ch) {
                    float v00 = (float)src[(y0 * w + x0) * c + ch];
                    float v10 = (float)src[(y0 * w + x1) * c + ch];
                    float v01 = (float)src[(y1 * w + x0) * c + ch];
                    float v11 = (float)src[(y1 * w + x1) * c + ch];
                    float v0 = v00 * (1.0f - sx) + v10 * sx;
                    float v1 = v01 * (1.0f - sx) + v11 * sx;
                    float v = v0 * (1.0f - sy) + v1 * sy;
                    out[(y * new_width + x) * c + ch] = clamp_u8((int)roundf(v));
                }
            }
        }
    }

    free(image->pixels);
    image->pixels = out;
    image->width = new_width;
    image->height = new_height;
    free(src);
    return true;
}

bool fossil_image_process_crop(fossil_image_t *image, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if (!validate_image(image)) return false;
    uint32_t w = IMG_W(image), h = IMG_H(image);
    uint8_t c = IMG_C(image);
    if (x >= w || y >= h) return false;
    if (x + width > w) width = w - x;
    if (y + height > h) height = h - y;

    uint8_t *out = (uint8_t*)malloc((size_t)width * height * c);
    if (!out) return false;

    for (uint32_t row = 0; row < height; ++row) {
        memcpy(&out[(row * width) * c],
               &image->pixels[((size_t)(y + row) * w + x) * c],
               (size_t)width * c);
    }

    free(image->pixels);
    image->pixels = out;
    image->width = width;
    image->height = height;
    return true;
}

bool fossil_image_process_blend(fossil_image_t *dest, const fossil_image_t *src, float alpha) {
    if (!validate_image(dest) || !validate_image(src)) return false;
    if (IMG_W(dest) != IMG_W(src) || IMG_H(dest) != IMG_H(src) || IMG_C(dest) != IMG_C(src)) return false;
    alpha = clampf(alpha, 0.0f, 1.0f);
    size_t total = (size_t)IMG_W(dest) * IMG_H(dest) * IMG_C(dest);
    for (size_t i = 0; i < total; ++i) {
        float d = (float)dest->pixels[i];
        float s = (float)src->pixels[i];
        int out = (int)roundf((1.0f - alpha) * d + alpha * s);
        dest->pixels[i] = clamp_u8(out);
    }
    return true;
}

bool fossil_image_process_gamma(fossil_image_t *image, float gamma_value) {
    if (!validate_image(image)) return false;
    if (gamma_value <= 0.0f) return false;
    size_t total = (size_t)IMG_W(image) * IMG_H(image) * IMG_C(image);
    // build lookup table
    uint8_t lut[256];
    float invg = 1.0f / gamma_value;
    for (int i = 0; i < 256; ++i) {
        lut[i] = clamp_u8((int)roundf(255.0f * powf((float)i / 255.0f, invg)));
    }
    for (size_t i = 0; i < total; ++i) {
        image->pixels[i] = lut[image->pixels[i]];
    }
    return true;
}

bool fossil_image_process_colorspace(fossil_image_t *image, fossil_color_space_e target_space) {
    if (!validate_image(image)) return false;
    // Minimal support:
    // - convert RGB/RGBA -> GRAYSCALE (assumes target_space enum contains sensible values)
    // We'll assume fossil_color_space_e contains at least FOSSIL_COLOR_SPACE_RGB and FOSSIL_COLOR_SPACE_GRAY
    // If your enum differs, wire up accordingly.
#ifdef FOSSIL_COLOR_SPACE_GRAY
    if (target_space == FOSSIL_COLOR_SPACE_GRAY) {
        if (IMG_C(image) == 1) return true;
        uint32_t w = IMG_W(image), h = IMG_H(image);
        uint8_t *out = (uint8_t*)malloc((size_t)w * h);
        if (!out) return false;
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                uint8_t r = IMG_PIX(image, x, y, 0);
                uint8_t g = (IMG_C(image) > 1) ? IMG_PIX(image, x, y, 1) : r;
                uint8_t b = (IMG_C(image) > 2) ? IMG_PIX(image, x, y, 2) : r;
                out[y * w + x] = rgb_to_luma_u8(r,g,b);
            }
        }
        free(image->pixels);
        image->pixels = out;
        image->channels = 1;
        return true;
    }
#endif
    // If target space not recognized or mapping unavailable, return false (no-op).
    return false;
}

bool fossil_image_process_equalize(fossil_image_t *image) {
    if (!validate_image(image)) return false;
    // histogram equalization per channel (or on luminance)
    uint32_t w = IMG_W(image), h = IMG_H(image);
    uint8_t c = IMG_C(image);
    size_t npix = (size_t)w * h;

    if (c == 1) {
        // single-channel
        int hist[256] = {0};
        for (size_t i = 0; i < npix; ++i) ++hist[image->pixels[i]];
        int cdf[256] = {0};
        cdf[0] = hist[0];
        for (int i = 1; i < 256; ++i) cdf[i] = cdf[i-1] + hist[i];
        for (size_t i = 0; i < npix; ++i) {
            int v = image->pixels[i];
            int nv = (int)roundf(((float)(cdf[v] - cdf[0]) / (float)(npix - cdf[0])) * 255.0f);
            image->pixels[i] = clamp_u8(nv);
        }
        return true;
    } else {
        // equalize luminance and scale channels proportionally (simpler)
        // compute luminance histogram
        int hist[256] = {0};
        uint8_t *lum = (uint8_t*)malloc(npix);
        if (!lum) return false;
        for (size_t i = 0; i < npix; ++i) {
            uint8_t r = image->pixels[i*c + 0];
            uint8_t g = image->pixels[i*c + 1];
            uint8_t b = image->pixels[i*c + 2];
            uint8_t L = rgb_to_luma_u8(r,g,b);
            lum[i] = L;
            ++hist[L];
        }
        int cdf[256] = {0};
        cdf[0] = hist[0];
        for (int i = 1; i < 256; ++i) cdf[i] = cdf[i-1] + hist[i];
        for (size_t i = 0; i < npix; ++i) {
            int v = lum[i];
            int nv = (int)roundf(((float)(cdf[v] - cdf[0]) / (float)(npix - cdf[0])) * 255.0f);
            // scale color channels by ratio nv / v (guard v==0)
            if (v == 0) {
                // set to equalized intensity
                for (size_t ch = 0; ch < 3; ++ch) image->pixels[i*c + ch] = clamp_u8(nv);
            } else {
                float ratio = (float)nv / (float)v;
                for (size_t ch = 0; ch < 3; ++ch) {
                    int outv = (int)roundf((float)image->pixels[i*c + ch] * ratio);
                    image->pixels[i*c + ch] = clamp_u8(outv);
                }
            }
        }
        free(lum);
        return true;
    }
}

bool fossil_image_process_edge_detect(fossil_image_t *image, bool normalize) {
    // Reuse parts of FILTER_EDGE implementation; this will put grayscale edge magnitude across channels.
    if (!validate_image(image)) return false;
    size_t w = IMG_W(image), h = IMG_H(image), c = IMG_C(image);
    uint8_t *src = alloc_temp(image);
    if (!src) return false;
    memcpy(src, image->pixels, (size_t)w*h*c);

    uint8_t *gray = (uint8_t*)malloc((size_t)w*h);
    if (!gray) { free(src); return false; }
    for (size_t i = 0; i < (size_t)w*h; ++i) {
        uint8_t r = src[i*c + 0];
        uint8_t g = (c > 1) ? src[i*c + 1] : r;
        uint8_t b = (c > 2) ? src[i*c + 2] : r;
        gray[i] = rgb_to_luma_u8(r,g,b);
    }

    int sx[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
    int sy[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};
    uint8_t *out = (uint8_t*)malloc((size_t)w*h);
    if (!out) { free(src); free(gray); return false; }

    int maxmag = 0;
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x) {
            int gx = 0, gy = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                int yy = (int)y + ky;
                if (yy < 0 || yy >= (int)h) continue;
                for (int kx = -1; kx <= 1; ++kx) {
                    int xx = (int)x + kx;
                    if (xx < 0 || xx >= (int)w) continue;
                    int v = gray[yy*w + xx];
                    gx += v * sx[ky+1][kx+1];
                    gy += v * sy[ky+1][kx+1];
                }
            }
            int mag = (int)roundf(sqrtf((float)(gx*gx + gy*gy)));
            if (mag > maxmag) maxmag = mag;
            out[y*w + x] = (uint8_t)clamp_u8(mag);
        }
    }

    if (normalize && maxmag > 0) {
        float scale = 255.0f / (float)maxmag;
        for (size_t i = 0; i < (size_t)w*h; ++i) out[i] = clamp_u8((int)roundf(out[i] * scale));
    }

    // write back to image channels
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x) {
            uint8_t v = out[y*w + x];
            for (size_t ch = 0; ch < c; ++ch) image->pixels[(y*w + x)*c + ch] = v;
        }
    }

    free(src);
    free(gray);
    free(out);
    return true;
}

bool fossil_image_process_gaussian(fossil_image_t *image, float radius) {
    if (!validate_image(image)) return false;
    if (radius <= 0.0f) return false;
    // create 1D kernel (separable)
    int r = (int)ceilf(radius * 3.0f);
    int size = r * 2 + 1;
    float *kernel = (float*)malloc(sizeof(float) * size);
    if (!kernel) return false;
    float sigma = radius;
    float sum = 0.0f;
    for (int i = -r; i <= r; ++i) {
        float v = expf(-(i*i) / (2.0f * sigma * sigma));
        kernel[i + r] = v;
        sum += v;
    }
    for (int i = 0; i < size; ++i) kernel[i] /= sum;

    size_t w = IMG_W(image), h = IMG_H(image), c = IMG_C(image);
    uint8_t *temp = alloc_temp(image);
    if (!temp) { free(kernel); return false; }
    memcpy(temp, image->pixels, (size_t)w*h*c);

    // horizontal pass
    uint8_t *buf = (uint8_t*)malloc((size_t)w*h*c);
    if (!buf) { free(temp); free(kernel); return false; }
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x) {
            for (size_t ch = 0; ch < c; ++ch) {
                float acc = 0.0f;
                for (int k = -r; k <= r; ++k) {
                    int sx = (int)x + k;
                    if (sx < 0) sx = 0;
                    if (sx >= (int)w) sx = (int)w - 1;
                    acc += kernel[k + r] * (float)temp[(y * w + sx) * c + ch];
                }
                buf[(y * w + x) * c + ch] = clamp_u8((int)roundf(acc));
            }
        }
    }
    // vertical pass
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x) {
            for (size_t ch = 0; ch < c; ++ch) {
                float acc = 0.0f;
                for (int k = -r; k <= r; ++k) {
                    int sy = (int)y + k;
                    if (sy < 0) sy = 0;
                    if (sy >= (int)h) sy = (int)h - 1;
                    acc += kernel[k + r] * (float)buf[(sy * w + x) * c + ch];
                }
                image->pixels[(y * w + x) * c + ch] = clamp_u8((int)roundf(acc));
            }
        }
    }

    free(kernel);
    free(temp);
    free(buf);
    return true;
}
