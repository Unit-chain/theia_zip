//
// Created by Kirill Zhukov on 11.05.2023.
//

#ifndef THEIA_ZIP_ZIPARCHIVE_H
#define THEIA_ZIP_ZIPARCHIVE_H
#include "iostream"
#include "vector"
#include "dirent.h"
#include "zip.h"
#include "crc32.h"

class ZIPArchive {
public:
    ZIPArchive(char *path, char *archive_path) : path(path), archive_path(archive_path) {
        generate_crc32_table();
    }
public:
    std::vector<std::string> files;
public:
    void archive();
    const std::string &getPath() const;
    void setPath(const std::string &path);
    const std::string &getArchivePath() const;
    void setArchivePath(const std::string &archivePath);
private:
    void lookup_directory();
private:
    std::string path;
    std::string archive_path;
};

#endif //THEIA_ZIP_ZIPARCHIVE_H
