//
// Created by Kirill Zhukov on 11.05.2023.
//

#ifndef THEIA_ZIP_CRC32_H
#define THEIA_ZIP_CRC32_H

#include <cstdint>
#include "unistd.h"

static uint32_t crc_table[256];
static inline void generate_crc32_table() {
    uint32_t c;
    int i, j;

    for (i = 0; i < 256; i++) {
        c = i;
        for (j = 0; j < 8; j++) {
            if (c & 1) {
                c = 0xEDB88320 ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        crc_table[i] = c;
    }
}

static inline uint32_t th_crc32(uint8_t *data, size_t length) {
    uint32_t c = 0 ^ ~0U;
    while (length--) {
        c = crc_table[(c ^ *data++) & 0xFF] ^ (c >> 8);
    }
    return c ^ ~0U;
}

#if 0

#define POLYNOMIAL 0xEDB88320
uint32_t crc32(uint8_t *data, size_t len) {
    uint32_t crc = ~0;
    uint32_t i, j;
    uint32_t byte;

    static uint32_t table[0x100];

    // Generate lookup table
    if (table[1] == 0) {
        for (i = 0; i < 0x100; ++i) {
            uint32_t crc = i;
            for (j = 0; j < 8; ++j) {
                crc = (crc >> 1) ^ (- (int32_t)(crc & 1) & POLYNOMIAL);
            }
            table[i] = crc;
        }
    }

    // Calculate CRC
    for (i = 0; i < len; ++i) {
        byte = data[i];
        crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
    }

    return ~crc;
}

#endif

#endif //THEIA_ZIP_CRC32_H
