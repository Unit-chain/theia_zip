//
// Created by Kirill Zhukov on 10.05.2023.
//

#ifndef THEIA_ZIP_ZIP_H
#define THEIA_ZIP_ZIP_H

#include "cstdio"
#include "vector"
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define DEBUG false

#pragma pack(push, 1)
typedef struct {
    uint32_t signature;
    uint16_t version;
    uint16_t flags;
    uint16_t compression;
    uint16_t mod_time;
    uint16_t mod_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_field_length;
} local_file_header;

typedef struct {
    uint32_t signature;
    uint16_t version_made_by;
    uint16_t version_needed;
    uint16_t flags;
    uint16_t compression;
    uint16_t mod_time;
    uint16_t mod_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_field_length;
    uint16_t comment_length;
    uint16_t disk_number_start;
    uint16_t internal_file_attributes;
    uint32_t external_file_attributes;
    uint32_t local_header_offset;
} central_directory_file_header;

typedef struct {
    uint32_t signature;
    uint16_t disk_number;
    uint16_t cd_disk_number;
    uint16_t disk_entries;
    uint16_t total_entries;
    uint32_t cd_size;
    uint32_t cd_offset;
    uint16_t comment_length;
} end_of_central_directory_record;


///@brief puts before end of central directory record.
typedef struct {
    uint32_t signature;
    uint32_t diskNumber;
    uint64_t eocd64Offset; //
    uint32_t totalDiskCount;
} end_of_central_directory_locator;

typedef struct {
    uint32_t signature;
    uint16_t disk_number;
    uint16_t cd_disk_number;
    uint16_t disk_entries;
    uint16_t total_entries;
    uint32_t cd_size;
    uint32_t cd_offset;
    uint16_t comment_length;
} end_of_central_directory_record_64;

/// @brief puts after central directory file header, e.g after filename.
typedef struct {
    uint16_t headerId;
    uint16_t dataSize;
    uint8_t *data;
} extra_field_record;

#pragma pack(pop)

typedef struct {
    size_t file_size;
    char* filename;
} filename;

std::vector<filename> indexing_files(char *path);

#endif //THEIA_ZIP_ZIP_H
