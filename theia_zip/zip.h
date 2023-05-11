//
// Created by Kirill Zhukov on 10.05.2023.
//

#ifndef THEIA_ZIP_ZIP_H
#define THEIA_ZIP_ZIP_H

#include "cstdio"
#include "vector"
#include "string"
#include "iostream"
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "zstd.h"

#ifdef _LP64
typedef long long_t;
typedef unsigned long ulong_t;
#else
typedef long long tlong;
typedef unsigned long long ulong_t;
#endif

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

typedef struct tm tm_t;

typedef struct {
    uint32_t crc32;
    ulong_t lfh_offset;
    size_t compressed_size;
    size_t uncompressed_size;
    std::string compressed_data;
} Compressed_file;

/// @brief Auxiliary function. `zip_time` function uses bit-shifting to pack the hour, minute, and second values of the tm struct into a 16-bit value.
/// It shifts the hour value 11 bits to the left, the minute value 5 bits to the left, and the second value 1 bit to the right, and then combines the three values using bitwise OR (|).
/// This results in a 16-bit value where the high 5 bits represent the hour, the middle 6 bits represent the minute, and the low 5 bits represent the second.
static uint16_t zip_time(tm_t *tm) {
    return (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec >> 1);
}

/// @brief Auxiliary function. The `zip_date` function also uses bit-shifting to pack the year, month, and day values of the tm struct into a 16-bit value.
/// It shifts the year value (which is offset by 80 to account for the fact that the ZIP file format uses a two-digit year representation starting from 1980) 9 bits to the left,
/// the month value 5 bits to the left, and then combines the three values using bitwise OR (|). This results in a 16-bit value where the high 7 bits represent the year,
/// the middle 4 bits represent the month, and the low 5 bits represent the day.
static uint16_t zip_date(tm_t *tm) {
    return ((tm->tm_year - 80) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;
}

/// @brief Auxiliary function, used for writing local header for each file.
/// @param zip file descriptor for lookup_directory(program).
/// @param filename name of the file.
/// @param crc stores the check code for the file.
/// @param compressed_size stores the compressed size of the file.
/// @param uncompressed_size stores the uncompressed size of the file.
/// @param mod_time bit-shifted representation of hour, minute, and second values in a 16-bit format.
/// @param mod_date bit-shifted representation of year, month, and day values in a 16-bit format.
static void write_local_file_header(FILE *zip, const char *filename, long_t crc, long_t compressed_size, long_t uncompressed_size,
                        uint16_t mod_time, uint16_t mod_date) {
    local_file_header header;
    header.signature = 0x04034b50;
    header.version = 20; // 2.0
    header.flags = 0;
    header.compression = 0x5D; // Zstandard (zstd)
    header.mod_time = mod_time;
    header.mod_date = mod_date;
    header.crc32 = crc;
    header.compressed_size = compressed_size;
    header.uncompressed_size = uncompressed_size;
    header.filename_length = strlen(filename);
    header.extra_field_length = 0;
    fwrite(&header, sizeof(header), 1, zip);
    fwrite(filename, header.filename_length, 1, zip);
}
/// @brief Auxiliary function, used for writing central directory for each file.
/// @param zip file descriptor for lookup_directory(program).
/// @param filename name of the file.
/// @param crc stores the check code for the file.
/// @param compressed_size stores the compressed size of the file.
/// @param uncompressed_size stores the uncompressed size of the file.
/// @param local_header_offset offset from the beginning of the lookup_directory(program) for the file.
/// @param mod_time bit-shifted representation of hour, minute, and second values in a 16-bit format.
/// @param mod_date bit-shifted representation of year, month, and day values in a 16-bit format.
static void write_central_directory_file_header(FILE *zip, const char *filename, long_t crc, long_t compressed_size,
                                         long_t uncompressed_size, long_t local_header_offset, uint16_t mod_time,
                                         uint16_t mod_date) {
    central_directory_file_header header;
    header.signature = 0x02014b50;
    header.version_made_by = 20; // 2.0
    header.version_needed = 20;  // 2.0
    header.flags = 0;
    header.compression = 0x5D; // Zstandard (zstd)
    header.mod_time = mod_time;
    header.mod_date = mod_date;
    header.crc32 = crc;
    header.compressed_size = compressed_size;
    header.uncompressed_size = uncompressed_size;
    header.filename_length = strlen(filename);
    header.extra_field_length = 0;
    header.comment_length = 0;
    header.disk_number_start = 0;
    header.internal_file_attributes = 0;
    header.external_file_attributes = 0x20; // Set the lookup_directory attribute
    header.local_header_offset = local_header_offset;
    fwrite(&header, sizeof(header), 1, zip);
    fwrite(filename, header.filename_length, 1, zip);
}
/// @brief Auxiliary function, used for writing end of central directory.
/// @param zip file descriptor for lookup_directory(program).
/// @param num_files number of files
/// @param central_directory_size size of central directories(amount).
/// @param central_directory_offset offset from the beginning of the lookup_directory(program) to the beginning of central directories.
static void write_end_of_central_directory_record(FILE *zip, int num_files, long_t central_directory_size,
                                           long_t central_directory_offset) {
    end_of_central_directory_record record;
    record.signature = 0x06054b50;
    record.disk_number = 0;
    record.cd_disk_number = 0;
    record.disk_entries = num_files;
    record.total_entries = num_files;
    record.cd_size = central_directory_size;
    record.cd_offset = central_directory_offset;
    record.comment_length = 0;
    fwrite(&record, sizeof(record), 1, zip);
}
///@brief returns file size
///@return file size of -1 if error occurred
static inline long_t getFileSize(FILE *fp) {
    long_t pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long_t size = ftell(fp);
    fseek(fp, pos, SEEK_SET);
    return size;
}

static inline int isDirectory(const char* path) {
#if _WIN32
    DWORD attributes = GetFileAttributes(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        printf("Error occurred while checking the path.\n");
        return -1;
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat pathStat;
    if (stat(path, &pathStat) == -1) {
        printf("Error occurred while checking the path.\n");
        return -1;
    }
    return S_ISDIR(pathStat.st_mode);
#endif
}

static inline Compressed_file compress_file(std::string input, int level = 19) {
    // Get the maximum size of the compressed data
    const size_t max_compressed_size = ZSTD_compressBound(input.size());
    // Allocate a buffer for the compressed data
    std::string compressed(max_compressed_size, '\0');
    // Compress the data
    const size_t compressed_size = ZSTD_compress(&compressed[0], max_compressed_size, input.data(), input.size(), level);
    // Resize the buffer to the actual compressed size
    compressed.resize(compressed_size);
    return {0, 0, compressed_size, 0, compressed};
}
#endif //THEIA_ZIP_ZIP_H
