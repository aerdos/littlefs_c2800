/*
 * lfs util functions
 *
 * Copyright (c) 2022, The littlefs authors.
 * Copyright (c) 2017, Arm Limited. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "lfs_util.h"

// Only compile if user does not provide custom config
#ifndef LFS_CONFIG


// Software CRC implementation with small lookup table, size is in bytes
uint32_t lfs_crc(uint32_t crc, const void *buffer, size_t size) {
    static const uint32_t rtable[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
    };

    const uint8_t *data = buffer;

    for (size_t i = 0; i < size; i++) {
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 0)) & 0xf];
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 4)) & 0xf];
    }

    return crc;
}

#ifdef LFS_C2800
void lfs_uint32_to_arr(uint32_t val, uint16_t *arr) {
    arr[3] = ((0xFF000000 & val)>>24UL);
    arr[2] = ((0x00FF0000 & val)>>16UL);
    arr[1] = ((0x0000FF00 & val)>>8UL);
    arr[0] = ((0x000000FF & val)>>0UL);
}

uint32_t lfs_arr_to_uint32(uint16_t *arr) {
    return (((uint32_t)arr[3])<<24UL) | (((uint32_t)arr[2])<<16UL) | (((uint32_t)arr[1])<<8UL) | ((uint32_t)arr[0]);
}
#endif

#endif
