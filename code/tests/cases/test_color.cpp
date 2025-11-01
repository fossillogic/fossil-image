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

FOSSIL_SUITE(cpp_image_color_fixture);

FOSSIL_SETUP(cpp_image_color_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_image_color_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Sort
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(cpp_test_image_color_brightness_increase) {
    fossil_image_t *img = fossil::image::Process::create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 10; img->data[1] = 20; img->data[2] = 30;
    img->data[3] = 40; img->data[4] = 50; img->data[5] = 60;
    img->data[6] = 70; img->data[7] = 80; img->data[8] = 90;
    img->data[9] = 100; img->data[10] = 110; img->data[11] = 120;
    bool ok = fossil::image::Color::brightness(img, 50);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->data[0], 60);
    ASSUME_ITS_EQUAL_I32(img->data[11], 170);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_color_brightness_decrease) {
    fossil_image_t *img = fossil::image::Process::create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    memset(img->data, 100, img->size);
    bool ok = fossil::image::Color::brightness(img, -50);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->data[0], 50);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_color_contrast_increase) {
    fossil_image_t *img = fossil::image::Process::create(2, 2, FOSSIL_PIXEL_FORMAT_GRAY8);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 64; img->data[1] = 128; img->data[2] = 192; img->data[3] = 255;
    bool ok = fossil::image::Color::contrast(img, 2.0f);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->data[0], 0);
    ASSUME_ITS_EQUAL_I32(img->data[3], 255);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_color_contrast_reduce) {
    fossil_image_t *img = fossil::image::Process::create(2, 2, FOSSIL_PIXEL_FORMAT_GRAY8);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 64; img->data[1] = 128; img->data[2] = 192; img->data[3] = 255;
    bool ok = fossil::image::Color::contrast(img, 0.5f);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_TRUE(img->data[1] > img->data[0]);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_color_gamma_basic) {
    fossil_image_t *img = fossil::image::Process::create(2, 2, FOSSIL_PIXEL_FORMAT_GRAY8);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 64; img->data[1] = 128; img->data[2] = 192; img->data[3] = 255;
    bool ok = fossil::image::Color::gamma(img, 2.2f);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_TRUE(img->data[0] < img->data[1]);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_color_gamma_invalid) {
    fossil_image_t *img = fossil::image::Process::create(2, 2, FOSSIL_PIXEL_FORMAT_GRAY8);
    ASSUME_NOT_CNULL(img);
    bool ok = fossil::image::Color::gamma(img, 0.0f);
    ASSUME_ITS_FALSE(ok);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_color_hsv_adjust_hue_shift) {
    fossil_image_t *img = fossil::image::Process::create(1, 1, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 255; img->data[1] = 0; img->data[2] = 0; // Red
    bool ok = fossil::image::Color::hsv_adjust(img, 120.0f, 1.0f, 1.0f);
    ASSUME_ITS_TRUE(ok);
    // After hue shift, should be green
    ASSUME_ITS_TRUE(img->data[1] > img->data[0]);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_color_hsv_adjust_saturation) {
    fossil_image_t *img = fossil::image::Process::create(1, 1, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 100; img->data[1] = 150; img->data[2] = 200;
    bool ok = fossil::image::Color::hsv_adjust(img, 0.0f, 0.0f, 1.0f);
    ASSUME_ITS_TRUE(ok);
    // Saturation 0: should be gray
    ASSUME_ITS_EQUAL_I32(img->data[0], img->data[1]);
    ASSUME_ITS_EQUAL_I32(img->data[1], img->data[2]);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_color_channel_swap_basic) {
    fossil_image_t *img = fossil::image::Process::create(1, 1, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 10; img->data[1] = 20; img->data[2] = 30;
    bool ok = fossil::image::Color::channel_swap(img, 0, 2);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->data[0], 30);
    ASSUME_ITS_EQUAL_I32(img->data[2], 10);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_color_channel_swap_invalid) {
    fossil_image_t *img = fossil::image::Process::create(1, 1, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    bool ok = fossil::image::Color::channel_swap(img, 0, 5);
    ASSUME_ITS_FALSE(ok);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_color_to_grayscale_basic) {
    fossil_image_t *img = fossil::image::Process::create(1, 1, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 255; img->data[1] = 0; img->data[2] = 0; // Red
    bool ok = fossil::image::Color::to_grayscale(img);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->channels, 1);
    ASSUME_ITS_TRUE(img->data[0] > 0);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_color_to_grayscale_already_gray) {
    fossil_image_t *img = fossil::image::Process::create(1, 1, FOSSIL_PIXEL_FORMAT_GRAY8);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 123;
    bool ok = fossil::image::Color::to_grayscale(img);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->data[0], 123);
    fossil::image::Process::destroy(img);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_image_color_tests) {
    FOSSIL_TEST_ADD(cpp_image_color_fixture, cpp_test_image_color_brightness_increase);
    FOSSIL_TEST_ADD(cpp_image_color_fixture, cpp_test_image_color_brightness_decrease);
    FOSSIL_TEST_ADD(cpp_image_color_fixture, cpp_test_image_color_contrast_increase);
    FOSSIL_TEST_ADD(cpp_image_color_fixture, cpp_test_image_color_contrast_reduce);
    FOSSIL_TEST_ADD(cpp_image_color_fixture, cpp_test_image_color_gamma_basic);
    FOSSIL_TEST_ADD(cpp_image_color_fixture, cpp_test_image_color_gamma_invalid);
    FOSSIL_TEST_ADD(cpp_image_color_fixture, cpp_test_image_color_hsv_adjust_hue_shift);
    FOSSIL_TEST_ADD(cpp_image_color_fixture, cpp_test_image_color_hsv_adjust_saturation);
    FOSSIL_TEST_ADD(cpp_image_color_fixture, cpp_test_image_color_channel_swap_basic);
    FOSSIL_TEST_ADD(cpp_image_color_fixture, cpp_test_image_color_channel_swap_invalid);
    FOSSIL_TEST_ADD(cpp_image_color_fixture, cpp_test_image_color_to_grayscale_basic);
    FOSSIL_TEST_ADD(cpp_image_color_fixture, cpp_test_image_color_to_grayscale_already_gray);

    FOSSIL_TEST_REGISTER(cpp_image_color_fixture);
} // end of tests
