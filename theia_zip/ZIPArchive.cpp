//
// Created by Kirill Zhukov on 11.05.2023.
//

#include "ZIPArchive.h"


void ZIPArchive::lookup_directory() {
    std::vector<std::string> directories = {this->path};

    while (!directories.empty()) {
        std::string dir_path = directories.back();
        directories.pop_back();

        DIR* dir = opendir(dir_path.c_str());
        if (dir == nullptr) {
            std::cerr << "Error opening directory " << dir_path << std::endl;
            continue;
        }

        dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_DIR) {
                if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..") {
                    continue; // Skip current and parent directories
                }
                std::string subdir_path = dir_path + "/" + entry->d_name;
                directories.push_back(subdir_path);
            } else {
                size_t pos = dir_path.find(this->path);
                if (pos == std::string::npos) {
                    std::cerr << "Error: root path not found in directory path " << dir_path << std::endl;
                    continue;
                }
                std::string parent_path = dir_path.substr(pos + this->path.length());
                if (parent_path.front() == '/') {
                    parent_path = parent_path.substr(1);
                }

                if (parent_path.empty()) {
                    this->files.emplace_back(entry->d_name);
                } else {
                    this->files.push_back(std::string(parent_path).append("/").append(entry->d_name));
                }
            }
        }
        closedir(dir);
    }
}

const std::string &ZIPArchive::getPath() const {
    return path;
}

void ZIPArchive::setPath(const std::string &path) {
    ZIPArchive::path = path;
}

const std::string &ZIPArchive::getArchivePath() const {
    return archive_path;
}

void ZIPArchive::setArchivePath(const std::string &archivePath) {
    archive_path = archivePath;
}

void ZIPArchive::archive() {
    if (this->files.empty()) {
        this->lookup_directory();
    }
    FILE *zip = fopen(this->archive_path.c_str(), "wb");

    time_t mod_time = time(NULL);
    tm_t *mod_tm = localtime(&mod_time);
    uint16_t dos_mod_time = zip_time(mod_tm);
    uint16_t dos_mod_date = zip_date(mod_tm);
    std::vector<Compressed_file> compressed_files;

    for (auto &it : this->files) {
        std::string filepath = this->path + "/" + it;
        if (isDirectory(filepath.c_str())) {
            FILE *fp = fopen(filepath.c_str(), "r");
            size_t uncompressed_filesize = getFileSize(fp);
            char data[uncompressed_filesize];
            fread(data, sizeof(data), 1, fp);
            uint32_t crc32 = th_crc32((uint8_t *) data, uncompressed_filesize);
            Compressed_file cf = compress_file(data);
            cf.crc32 = crc32;
            cf.lfh_offset = ftell(zip);
            cf.uncompressed_size = uncompressed_filesize;
            compressed_files.push_back(cf);
            write_local_file_header(zip, it.c_str(), crc32, cf.compressed_size, uncompressed_filesize, dos_mod_time,
                                    dos_mod_date);
            fwrite(cf.compressed_data.c_str(), cf.compressed_size, 1, fp);
            fclose(fp);
            continue;
        }
        Compressed_file cf{};
        cf.crc32 = 0;
        cf.lfh_offset = ftell(zip);
        cf.compressed_size = 0;
        cf.uncompressed_size = 0;
        compressed_files.push_back(cf);
        write_local_file_header(zip, it.c_str(), 0, cf.compressed_size, 0, dos_mod_time, dos_mod_date);
    }
    ulong_t cd_offset = ftell(zip);
    for (long_t i = 0; i < this->files.size(); i++) {
        Compressed_file cf = compressed_files.at(i);
        write_central_directory_file_header(zip, this->files.at(i).c_str(), cf.crc32, cf.compressed_size, cf.uncompressed_size, cf.lfh_offset, dos_mod_time, dos_mod_date);
    }
    ulong_t cd_sz = cd_offset - ftell(zip);
    write_end_of_central_directory_record(zip, this->files.size(), cd_sz, cd_offset);
}
