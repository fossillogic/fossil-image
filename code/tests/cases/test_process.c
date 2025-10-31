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

FOSSIL_SUITE(c_image_process_fixture);

FOSSIL_SETUP(c_image_process_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_image_process_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Sort
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(c_test_image_process_create_basic) {
    fossil_image_t *img = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    ASSUME_ITS_EQUAL_I32(img->width, 2);
    ASSUME_ITS_EQUAL_I32(img->height, 2);
    fossil_image_process_destroy(img);
}

FOSSIL_TEST(c_test_image_process_create_zero_size) {
    fossil_image_t *img = fossil_image_process_create(0, 0, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_ITS_FALSE(img != NULL);
}

FOSSIL_TEST(c_test_image_process_destroy_basic) {
    fossil_image_t *img = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    fossil_image_process_destroy(img);
    // No assertion needed, just ensure no crash
}

FOSSIL_TEST(c_test_image_process_resize_basic) {
    fossil_image_t *img = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    memset(img->data, 123, img->size);
    bool ok = fossil_image_process_resize(img, 4, 4, FOSSIL_INTERP_NEAREST);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->width, 4);
    ASSUME_ITS_EQUAL_I32(img->height, 4);
    fossil_image_process_destroy(img);
}

FOSSIL_TEST(c_test_image_process_resize_invalid) {
    fossil_image_t *img = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    bool ok = fossil_image_process_resize(img, 0, 0, FOSSIL_INTERP_NEAREST);
    ASSUME_ITS_FALSE(ok);
    fossil_image_process_destroy(img);
}

FOSSIL_TEST(c_test_image_process_resize_null_image) {
    bool ok = fossil_image_process_resize(NULL, 4, 4, FOSSIL_INTERP_NEAREST);
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(c_test_image_process_crop_basic) {
    fossil_image_t *img = fossil_image_process_create(4, 4, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    memset(img->data, 50, img->size);
    bool ok = fossil_image_process_crop(img, 1, 1, 2, 2);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->width, 2);
    ASSUME_ITS_EQUAL_I32(img->height, 2);
    fossil_image_process_destroy(img);
}

FOSSIL_TEST(c_test_image_process_crop_out_of_bounds) {
    fossil_image_t *img = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    bool ok = fossil_image_process_crop(img, 1, 1, 5, 5);
    ASSUME_ITS_FALSE(ok);
    fossil_image_process_destroy(img);
}

FOSSIL_TEST(c_test_image_process_flip_horizontal) {
    fossil_image_t *img = fossil_image_process_create(2, 1, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 10; img->data[3] = 20;
    bool ok = fossil_image_process_flip(img, true, false);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->data[0], 20);
    fossil_image_process_destroy(img);
}

FOSSIL_TEST(c_test_image_process_flip_vertical) {
    fossil_image_t *img = fossil_image_process_create(1, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 30; img->data[3] = 40;
    bool ok = fossil_image_process_flip(img, false, true);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->data[0], 40);
    fossil_image_process_destroy(img);
}

FOSSIL_TEST(c_test_image_process_rotate_90) {
    fossil_image_t *img = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 1; img->data[3] = 2; img->data[6] = 3; img->data[9] = 4;
    bool ok = fossil_image_process_rotate(img, 90.0f);
    ASSUME_ITS_TRUE(ok);
    fossil_image_process_destroy(img);
}

FOSSIL_TEST(c_test_image_process_blend_basic) {
    fossil_image_t *img1 = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    fossil_image_t *img2 = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_ITS_TRUE(img1 && img2);
    memset(img1->data, 0, img1->size);
    memset(img2->data, 255, img2->size);
    bool ok = fossil_image_process_blend(img1, img2, 0.5f);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img1->data[0], 127);
    fossil_image_process_destroy(img1);
    fossil_image_process_destroy(img2);
}

FOSSIL_TEST(c_test_image_process_blend_mismatched_size) {
    fossil_image_t *img1 = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    fossil_image_t *img2 = fossil_image_process_create(3, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_ITS_TRUE(img1 && img2);
    bool ok = fossil_image_process_blend(img1, img2, 0.5f);
    ASSUME_ITS_FALSE(ok);
    fossil_image_process_destroy(img1);
    fossil_image_process_destroy(img2);
}

FOSSIL_TEST(c_test_image_process_composite_basic) {
    fossil_image_t *dst = fossil_image_process_create(4, 4, FOSSIL_PIXEL_FORMAT_RGB24);
    fossil_image_t *overlay = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_ITS_TRUE(dst && overlay);
    memset(dst->data, 0, dst->size);
    memset(overlay->data, 255, overlay->size);
    bool ok = fossil_image_process_composite(dst, overlay, 1, 1, 0.5f);
    ASSUME_ITS_TRUE(ok);
    fossil_image_process_destroy(dst);
    fossil_image_process_destroy(overlay);
}

FOSSIL_TEST(c_test_image_process_grayscale_basic) {
    fossil_image_t *img = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_RGB24);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 255; img->data[1] = 0; img->data[2] = 0;
    img->data[3] = 0; img->data[4] = 255; img->data[5] = 0;
    img->data[6] = 0; img->data[7] = 0; img->data[8] = 255;
    img->data[9] = 255; img->data[10] = 255; img->data[11] = 255;
    bool ok = fossil_image_process_grayscale(img);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->channels, 1);
    fossil_image_process_destroy(img);
}

FOSSIL_TEST(c_test_image_process_threshold_basic) {
    fossil_image_t *img = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_GRAY8);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 100; img->data[1] = 200; img->data[2] = 50; img->data[3] = 255;
    bool ok = fossil_image_process_threshold(img, 128);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->data[0], 0);
    ASSUME_ITS_EQUAL_I32(img->data[1], 255);
    fossil_image_process_destroy(img);
}

FOSSIL_TEST(c_test_image_process_invert_basic) {
    fossil_image_t *img = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_GRAY8);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 0; img->data[1] = 128; img->data[2] = 255; img->data[3] = 64;
    bool ok = fossil_image_process_invert(img);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->data[0], 255);
    ASSUME_ITS_EQUAL_I32(img->data[2], 0);
    fossil_image_process_destroy(img);
}

FOSSIL_TEST(c_test_image_process_normalize_basic) {
    fossil_image_t *img = fossil_image_process_create(2, 2, FOSSIL_PIXEL_FORMAT_GRAY8);
    ASSUME_NOT_CNULL(img);
    img->data[0] = 50; img->data[1] = 100; img->data[2] = 150; img->data[3] = 200;
    bool ok = fossil_image_process_normalize(img);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32(img->data[0], 0);
    ASSUME_ITS_EQUAL_I32(img->data[3], 255);
    fossil_image_process_destroy(img);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_image_process_tests) {
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_create_basic);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_create_zero_size);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_destroy_basic);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_resize_basic);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_resize_invalid);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_resize_null_image);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_crop_basic);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_crop_out_of_bounds);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_flip_horizontal);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_flip_vertical);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_rotate_90);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_blend_basic);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_blend_mismatched_size);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_composite_basic);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_grayscale_basic);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_threshold_basic);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_invert_basic);
    FOSSIL_TEST_ADD(c_image_process_fixture, c_test_image_process_normalize_basic);

    FOSSIL_TEST_REGISTER(c_image_process_fixture);
} // end of tests
