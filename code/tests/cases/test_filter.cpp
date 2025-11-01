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

FOSSIL_SUITE(cpp_image_filter_fixture);

FOSSIL_SETUP(cpp_image_filter_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_image_filter_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Sort
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(cpp_test_image_filter_convolve3x3_basic) {
    fossil_image_t *img = fossil::image::Process::create(3, 3, FOSSIL_PIXEL_FORMAT_GRAY8);
    ASSUME_NOT_CNULL(img);
    for (int i = 0; i < 9; ++i) img->data[i] = i * 10;
    const float kernel[3][3] = {
        {0, 1, 0},
        {1, -4, 1},
        {0, 1, 0}
    };
    bool ok = fossil::image::Filter::convolve3x3(img, kernel, 1.0f, 0.0f);
    ASSUME_ITS_TRUE(ok);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_filter_convolve3x3_null_image) {
    const float kernel[3][3] = {{0}};
    bool ok = fossil::image::Filter::convolve3x3(NULL, kernel, 1.0f, 0.0f);
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(cpp_test_image_filter_blur_basic) {
    fossil_image_t *img = fossil::image::Process::create(3, 3, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    memset(img->data, 100, img->size);
    bool ok = fossil::image::Filter::blur(img, 1.0f);
    ASSUME_ITS_TRUE(ok);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_filter_blur_large_radius) {
    fossil_image_t *img = fossil::image::Process::create(5, 5, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    memset(img->data, 200, img->size);
    bool ok = fossil::image::Filter::blur(img, 3.0f);
    ASSUME_ITS_TRUE(ok);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_filter_sharpen_basic) {
    fossil_image_t *img = fossil::image::Process::create(3, 3, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    memset(img->data, 50, img->size);
    bool ok = fossil::image::Filter::sharpen(img);
    ASSUME_ITS_TRUE(ok);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_filter_edge_basic) {
    fossil_image_t *img = fossil::image::Process::create(3, 3, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    memset(img->data, 80, img->size);
    bool ok = fossil::image::Filter::edge(img);
    ASSUME_ITS_TRUE(ok);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_filter_emboss_basic) {
    fossil_image_t *img = fossil::image::Process::create(3, 3, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    memset(img->data, 120, img->size);
    bool ok = fossil::image::Filter::emboss(img);
    ASSUME_ITS_TRUE(ok);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_filter_blur_too_small_image) {
    fossil_image_t *img = fossil::image::Process::create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    bool ok = fossil::image::Filter::blur(img, 1.0f);
    ASSUME_ITS_FALSE(ok);
    fossil::image::Process::destroy(img);
}

FOSSIL_TEST(cpp_test_image_filter_sharpen_null_image) {
    bool ok = fossil::image::Filter::sharpen(NULL);
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(cpp_test_image_filter_edge_null_image) {
    bool ok = fossil::image::Filter::edge(NULL);
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(cpp_test_image_filter_emboss_null_image) {
    bool ok = fossil::image::Filter::emboss(NULL);
    ASSUME_ITS_FALSE(ok);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_image_filter_tests) {
    FOSSIL_TEST_ADD(cpp_image_filter_fixture, cpp_test_image_filter_convolve3x3_basic);
    FOSSIL_TEST_ADD(cpp_image_filter_fixture, cpp_test_image_filter_convolve3x3_null_image);
    FOSSIL_TEST_ADD(cpp_image_filter_fixture, cpp_test_image_filter_blur_basic);
    FOSSIL_TEST_ADD(cpp_image_filter_fixture, cpp_test_image_filter_blur_large_radius);
    FOSSIL_TEST_ADD(cpp_image_filter_fixture, cpp_test_image_filter_sharpen_basic);
    FOSSIL_TEST_ADD(cpp_image_filter_fixture, cpp_test_image_filter_edge_basic);
    FOSSIL_TEST_ADD(cpp_image_filter_fixture, cpp_test_image_filter_emboss_basic);
    FOSSIL_TEST_ADD(cpp_image_filter_fixture, cpp_test_image_filter_blur_too_small_image);
    FOSSIL_TEST_ADD(cpp_image_filter_fixture, cpp_test_image_filter_sharpen_null_image);
    FOSSIL_TEST_ADD(cpp_image_filter_fixture, cpp_test_image_filter_edge_null_image);
    FOSSIL_TEST_ADD(cpp_image_filter_fixture, cpp_test_image_filter_emboss_null_image);

    FOSSIL_TEST_REGISTER(cpp_image_filter_fixture);
} // end of tests
