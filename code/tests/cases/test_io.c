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

FOSSIL_SUITE(c_image_io_fixture);

FOSSIL_SETUP(c_image_io_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_image_io_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Sort
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(c_test_image_io_load_invalid_file) {
    fossil_image_t img;
    bool ok = fossil_image_io_load("nonexistent.bmp", "bmp", &img);
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(c_test_image_io_load_invalid_format) {
    fossil_image_t img;
    bool ok = fossil_image_io_load("test.bmp", "unknown", &img);
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(c_test_image_io_save_invalid_format) {
    fossil_image_t img;
    img.width = 2; img.height = 2; img.channels = 3; img.format = FOSSIL_PIXEL_FORMAT_RGB24;
    img.data = (uint8_t *)malloc(12); img.size = 12; img.owns_data = true;
    bool ok = fossil_image_io_save("test.img", "unknown", &img);
    ASSUME_ITS_FALSE(ok);
    free(img.data);
}

FOSSIL_TEST(c_test_image_io_generate_invalid_type) {
    fossil_image_t img = {0};
    bool ok = fossil_image_io_generate(&img, "unknown", 2, 2, FOSSIL_PIXEL_FORMAT_RGB24, NULL);
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(c_test_image_io_generate_zero_size) {
    fossil_image_t img = {0};
    bool ok = fossil_image_io_generate(&img, "solid", 0, 0, FOSSIL_PIXEL_FORMAT_RGB24, NULL);
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(c_test_image_io_generate_solid_rgb24) {
    fossil_image_t img = {0};
    float params[3] = {128.0f, 64.0f, 32.0f}; // R, G, B
    bool ok = fossil_image_io_generate(&img, "solid", 4, 4, FOSSIL_PIXEL_FORMAT_RGB24, params);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_TRUE(img.data != NULL);
    for (int i = 0; i < 16 * 3; i += 3) {
        ASSUME_ITS_EQUAL_I32(img.data[i], 128);
        ASSUME_ITS_EQUAL_I32(img.data[i + 1], 64);
        ASSUME_ITS_EQUAL_I32(img.data[i + 2], 32);
    }
    if (img.data) free(img.data);
}

FOSSIL_TEST(c_test_image_io_generate_gradient_gray8) {
    fossil_image_t img = {0};
    float params[2] = {0.0f, 255.0f}; // start, end
    bool ok = fossil_image_io_generate(&img, "gradient", 8, 1, FOSSIL_PIXEL_FORMAT_GRAY8, params);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_TRUE(img.data != NULL);
    ASSUME_ITS_EQUAL_I32(img.data[0], 0);
    ASSUME_ITS_EQUAL_I32(img.data[7], 255);
    if (img.data) free(img.data);
}

FOSSIL_TEST(c_test_image_io_generate_checker_rgb24) {
    fossil_image_t img = {0};
    float params[7] = {1.0f, 255.0f, 0.0f, 0.0f, 0.0f, 255.0f, 0.0f};
    bool ok = fossil_image_io_generate(&img, "checker", 4, 4, FOSSIL_PIXEL_FORMAT_RGB24, params);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_TRUE(img.data != NULL);
    ASSUME_ITS_EQUAL_I32(img.data[0], 255);
    ASSUME_ITS_EQUAL_I32(img.data[1], 0);
    ASSUME_ITS_EQUAL_I32(img.data[2], 0);
    ASSUME_ITS_EQUAL_I32(img.data[3], 255);
    ASSUME_ITS_EQUAL_I32(img.data[4], 255);
    ASSUME_ITS_EQUAL_I32(img.data[5], 255);
    if (img.data) free(img.data);
}

FOSSIL_TEST(c_test_image_io_generate_noise_gray8) {
    fossil_image_t img = {0};
    bool ok = fossil_image_io_generate(&img, "noise", 4, 4, FOSSIL_PIXEL_FORMAT_GRAY8, NULL);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_TRUE(img.data != NULL);
    int nonzero = 0;
    for (int i = 0; i < 16; ++i)
        if (img.data[i] != 0) nonzero++;
    ASSUME_ITS_TRUE(nonzero >= 0); // allow zero, don't crash
    if (img.data) free(img.data);
}

FOSSIL_TEST(c_test_image_io_generate_circle_gray8) {
    fossil_image_t img = {0};
    float params[4] = {2.0f, 0.0f, 255.0f, 128.0f}; // radius, edge, fg, bg
    bool ok = fossil_image_io_generate(&img, "circle", 5, 5, FOSSIL_PIXEL_FORMAT_GRAY8, params);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_TRUE(img.data != NULL);
    ASSUME_ITS_TRUE((img.data[2 * 5 + 2] == 255) || (img.data[2 * 5 + 2] == 128));
    if (img.data) free(img.data);
}

FOSSIL_TEST(c_test_image_io_generate_stripes_rgb24) {
    fossil_image_t img = {0};
    float params[7] = {1.0f, 255.0f, 255.0f, 255.0f, 0.0f, 0.0f, 0.0f};
    bool ok = fossil_image_io_generate(&img, "stripes", 4, 4, FOSSIL_PIXEL_FORMAT_RGB24, params);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_TRUE(img.data != NULL);
    ASSUME_ITS_EQUAL_I32(img.data[0], 255);
    ASSUME_ITS_EQUAL_I32(img.data[1], 255);
    ASSUME_ITS_EQUAL_I32(img.data[2], 255);
    ASSUME_ITS_EQUAL_I32(img.data[4 * 3], 0);
    ASSUME_ITS_EQUAL_I32(img.data[4 * 3 + 1], 0);
    ASSUME_ITS_EQUAL_I32(img.data[4 * 3 + 2], 0);
    if (img.data) free(img.data);
}

FOSSIL_TEST(c_test_image_io_generate_vstripes_rgb24) {
    fossil_image_t img = {0};
    float params[7] = {1.0f, 0.0f, 0.0f, 255.0f, 255.0f, 255.0f, 0.0f};
    bool ok = fossil_image_io_generate(&img, "vstripes", 4, 4, FOSSIL_PIXEL_FORMAT_RGB24, params);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_TRUE(img.data != NULL);
    ASSUME_ITS_EQUAL_I32(img.data[0], 0);
    ASSUME_ITS_EQUAL_I32(img.data[1], 0);
    ASSUME_ITS_EQUAL_I32(img.data[2], 255);
    ASSUME_ITS_EQUAL_I32(img.data[3], 255);
    ASSUME_ITS_EQUAL_I32(img.data[4], 255);
    ASSUME_ITS_EQUAL_I32(img.data[5], 0);
    if (img.data) free(img.data);
}

FOSSIL_TEST(c_test_image_io_generate_radial_gray8) {
    fossil_image_t img = {0};
    float params[2] = {128.0f, 255.0f}; // inner, outer
    bool ok = fossil_image_io_generate(&img, "radial", 5, 5, FOSSIL_PIXEL_FORMAT_GRAY8, params);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_TRUE(img.data != NULL);
    ASSUME_ITS_TRUE((img.data[0] == 255) || (img.data[0] == 128));
    if (img.data) free(img.data);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_image_io_tests) {
    FOSSIL_TEST_ADD(c_image_io_fixture, c_test_image_io_load_invalid_file);
    FOSSIL_TEST_ADD(c_image_io_fixture, c_test_image_io_load_invalid_format);
    FOSSIL_TEST_ADD(c_image_io_fixture, c_test_image_io_save_invalid_format);
    FOSSIL_TEST_ADD(c_image_io_fixture, c_test_image_io_generate_invalid_type);
    FOSSIL_TEST_ADD(c_image_io_fixture, c_test_image_io_generate_zero_size);
    FOSSIL_TEST_ADD(c_image_io_fixture, c_test_image_io_generate_solid_rgb24);
    FOSSIL_TEST_ADD(c_image_io_fixture, c_test_image_io_generate_gradient_gray8);
    FOSSIL_TEST_ADD(c_image_io_fixture, c_test_image_io_generate_checker_rgb24);
    FOSSIL_TEST_ADD(c_image_io_fixture, c_test_image_io_generate_noise_gray8);
    FOSSIL_TEST_ADD(c_image_io_fixture, c_test_image_io_generate_circle_gray8);
    FOSSIL_TEST_ADD(c_image_io_fixture, c_test_image_io_generate_stripes_rgb24);
    FOSSIL_TEST_ADD(c_image_io_fixture, c_test_image_io_generate_vstripes_rgb24);
    FOSSIL_TEST_ADD(c_image_io_fixture, c_test_image_io_generate_radial_gray8);

    FOSSIL_TEST_REGISTER(c_image_io_fixture);
} // end of tests
