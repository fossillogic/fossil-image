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

FOSSIL_SUITE(cpp_image_analyze_fixture);

FOSSIL_SETUP(cpp_image_analyze_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_image_analyze_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Sort
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(cpp_test_image_analyze_histogram_basic) {
    fossil::image::Process proc;
    fossil_image_t *img = proc.create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 10; img->data[1] = 20; img->data[2] = 30;
    img->data[3] = 10; img->data[4] = 20; img->data[5] = 30;
    img->data[6] = 10; img->data[7] = 20; img->data[8] = 30;
    img->data[9] = 10; img->data[10] = 20; img->data[11] = 30;
    uint32_t hist[256 * 3] = {0};
    bool ok = fossil::image::Analyzer::histogram(img, hist);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(hist[0 * 256 + 10], 4);
    ASSUME_ITS_EQUAL_I32(hist[1 * 256 + 20], 4);
    ASSUME_ITS_EQUAL_I32(hist[2 * 256 + 30], 4);
    proc.destroy(img);
}

FOSSIL_TEST(cpp_test_image_analyze_mean_stddev_basic) {
    fossil::image::Process proc;
    fossil_image_t *img = proc.create(2, 2, FOSSIL_PIXEL_FORMAT_GRAY8);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 10; img->data[1] = 20; img->data[2] = 30; img->data[3] = 40;
    double mean[1] = {0}, stddev[1] = {0};
    bool ok = fossil::image::Analyzer::meanStddev(img, mean, stddev);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_F64(mean[0], 25.0, 0.01);
    ASSUME_ITS_EQUAL_F64(stddev[0], sqrt(125.0), 0.01);
    proc.destroy(img);
}

FOSSIL_TEST(cpp_test_image_analyze_brightness_basic) {
    fossil::image::Process proc;
    fossil_image_t *img = proc.create(2, 2, FOSSIL_PIXEL_FORMAT_GRAY8);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 0; img->data[1] = 255; img->data[2] = 255; img->data[3] = 0;
    double brightness = 0.0;
    bool ok = fossil::image::Analyzer::brightness(img, &brightness);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_F64(brightness, 0.5, 0.01);
    proc.destroy(img);
}

FOSSIL_TEST(cpp_test_image_analyze_contrast_basic) {
    fossil::image::Process proc;
    fossil_image_t *img = proc.create(2, 2, FOSSIL_PIXEL_FORMAT_GRAY8);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 0; img->data[1] = 255; img->data[2] = 255; img->data[3] = 0;
    double contrast = 0.0;
    bool ok = fossil::image::Analyzer::contrast(img, &contrast);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_TRUE(contrast > 0.0);
    proc.destroy(img);
}

FOSSIL_TEST(cpp_test_image_analyze_entropy_basic) {
    fossil::image::Process proc;
    fossil_image_t *img = proc.create(2, 2, FOSSIL_PIXEL_FORMAT_GRAY8);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 0; img->data[1] = 0; img->data[2] = 255; img->data[3] = 255;
    double entropy = 0.0;
    bool ok = fossil::image::Analyzer::entropy(img, &entropy);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_TRUE(entropy > 0.0);
    proc.destroy(img);
}


// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_image_analyze_tests) {
    FOSSIL_TEST_ADD(cpp_image_analyze_fixture, cpp_test_image_analyze_histogram_basic);
    FOSSIL_TEST_ADD(cpp_image_analyze_fixture, cpp_test_image_analyze_mean_stddev_basic);
    FOSSIL_TEST_ADD(cpp_image_analyze_fixture, cpp_test_image_analyze_brightness_basic);
    FOSSIL_TEST_ADD(cpp_image_analyze_fixture, cpp_test_image_analyze_contrast_basic);
    FOSSIL_TEST_ADD(cpp_image_analyze_fixture, cpp_test_image_analyze_entropy_basic);

    FOSSIL_TEST_REGISTER(cpp_image_analyze_fixture);
} // end of tests
