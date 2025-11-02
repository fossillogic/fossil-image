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
#include <fossil/pizza/framework.h>

#include "fossil/image/framework.h"

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Utilities
// * * * * * * * * * * * * * * * * * * * * * * * *
// Setup steps for things like test fixtures and
// mock objects are set here.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_SUITE(cpp_image_draw_fixture);

FOSSIL_SETUP(cpp_image_draw_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_image_draw_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Sort
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(cpp_test_image_draw_pixel_rgb24) {
    fossil_image_t *img = fossil::image::Process::create(4, 4, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    uint8_t color[3] = {255, 128, 64};
    bool ok = fossil::image::Draw::pixel(img, 2, 2, color);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->data[2 * 3 + 2 * 4 * 3 + 0], 255);
    ASSUME_ITS_EQUAL_I32(img->data[2 * 3 + 2 * 4 * 3 + 1], 128);
    ASSUME_ITS_EQUAL_I32(img->data[2 * 3 + 2 * 4 * 3 + 2], 64);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_draw_pixel_out_of_bounds) {
    fossil_image_t *img = fossil::image::Process::create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    uint8_t color[3] = {1, 2, 3};
    bool ok = fossil::image::Draw::pixel(img, 5, 5, color);
    ASSUME_ITS_TRUE(ok); // function returns true, but does nothing
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_draw_line_horizontal) {
    fossil_image_t *img = fossil::image::Process::create(4, 4, FOSSIL_PIXEL_FORMAT_GRAY8);
    uint8_t color = 200;
    bool ok = fossil::image::Draw::line(img, 0, 1, 3, 1, &color);
    ASSUME_ITS_TRUE(ok);
    for (int x = 0; x < 4; ++x)
        ASSUME_ITS_EQUAL_I32(img->data[1 * 4 + x], 200);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_draw_line_vertical) {
    fossil_image_t *img = fossil::image::Process::create(4, 4, FOSSIL_PIXEL_FORMAT_GRAY8);
    uint8_t color = 100;
    bool ok = fossil::image::Draw::line(img, 2, 0, 2, 3, &color);
    ASSUME_ITS_TRUE(ok);
    for (int y = 0; y < 4; ++y)
        ASSUME_ITS_EQUAL_I32(img->data[y * 4 + 2], 100);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_draw_rect_outline) {
    fossil_image_t *img = fossil::image::Process::create(5, 5, FOSSIL_PIXEL_FORMAT_GRAY8);
    uint8_t color = 50;
    bool ok = fossil::image::Draw::rect(img, 1, 1, 3, 3, &color, false);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->data[1 * 5 + 1], 50);
    ASSUME_ITS_EQUAL_I32(img->data[1 * 5 + 3], 50);
    ASSUME_ITS_EQUAL_I32(img->data[3 * 5 + 1], 50);
    ASSUME_ITS_EQUAL_I32(img->data[3 * 5 + 3], 50);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_draw_rect_filled) {
    fossil_image_t *img = fossil::image::Process::create(4, 4, FOSSIL_PIXEL_FORMAT_GRAY8);
    uint8_t color = 77;
    bool ok = fossil::image::Draw::rect(img, 1, 1, 2, 2, &color, true);
    ASSUME_ITS_TRUE(ok);
    for (int y = 1; y <= 2; ++y)
        for (int x = 1; x <= 2; ++x)
            ASSUME_ITS_EQUAL_I32(img->data[y * 4 + x], 77);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_draw_circle_outline) {
    fossil_image_t *img = fossil::image::Process::create(7, 7, FOSSIL_PIXEL_FORMAT_GRAY8);
    uint8_t color = 99;
    bool ok = fossil::image::Draw::circle(img, 3, 3, 2, &color, false);
    ASSUME_ITS_TRUE(ok);
    // Just check some expected outline pixels
    ASSUME_ITS_EQUAL_I32(img->data[1 * 7 + 3], 99);
    ASSUME_ITS_EQUAL_I32(img->data[3 * 7 + 1], 99);
    ASSUME_ITS_EQUAL_I32(img->data[5 * 7 + 3], 99);
    ASSUME_ITS_EQUAL_I32(img->data[3 * 7 + 5], 99);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_draw_circle_filled) {
    fossil_image_t *img = fossil::image::Process::create(7, 7, FOSSIL_PIXEL_FORMAT_GRAY8);
    uint8_t color = 123;
    bool ok = fossil::image::Draw::circle(img, 3, 3, 2, &color, true);
    ASSUME_ITS_TRUE(ok);
    // Center pixel must be filled
    ASSUME_ITS_EQUAL_I32(img->data[3 * 7 + 3], 123);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_draw_fill_rgb24) {
    fossil_image_t *img = fossil::image::Process::create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    uint8_t color[3] = {10, 20, 30};
    bool ok = fossil::image::Draw::fill(img, color);
    ASSUME_ITS_TRUE(ok);
    for (int i = 0; i < 4 * 3; i += 3) {
        ASSUME_ITS_EQUAL_I32(img->data[i], 10);
        ASSUME_ITS_EQUAL_I32(img->data[i + 1], 20);
        ASSUME_ITS_EQUAL_I32(img->data[i + 2], 30);
    }
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_draw_text_out_of_bounds) {
    fossil_image_t *img = fossil::image::Process::create(8, 8, FOSSIL_PIXEL_FORMAT_GRAY8);
    uint8_t color = 200;
    bool ok = fossil::image::Draw::text(img, 7, 7, "B", &color);
    ASSUME_ITS_TRUE(ok); // Should not crash, may not draw anything
    fossil::image::Process::destroy(img);
}


// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_image_draw_tests) {
    FOSSIL_TEST_ADD(cpp_image_draw_fixture, cpp_test_image_draw_pixel_rgb24);
    FOSSIL_TEST_ADD(cpp_image_draw_fixture, cpp_test_image_draw_pixel_out_of_bounds);
    FOSSIL_TEST_ADD(cpp_image_draw_fixture, cpp_test_image_draw_line_horizontal);
    FOSSIL_TEST_ADD(cpp_image_draw_fixture, cpp_test_image_draw_line_vertical);
    FOSSIL_TEST_ADD(cpp_image_draw_fixture, cpp_test_image_draw_rect_outline);
    FOSSIL_TEST_ADD(cpp_image_draw_fixture, cpp_test_image_draw_rect_filled);
    FOSSIL_TEST_ADD(cpp_image_draw_fixture, cpp_test_image_draw_circle_outline);
    FOSSIL_TEST_ADD(cpp_image_draw_fixture, cpp_test_image_draw_circle_filled);
    FOSSIL_TEST_ADD(cpp_image_draw_fixture, cpp_test_image_draw_fill_rgb24);
    FOSSIL_TEST_ADD(cpp_image_draw_fixture, cpp_test_image_draw_text_out_of_bounds);

    FOSSIL_TEST_REGISTER(cpp_image_draw_fixture);
} // end of tests
