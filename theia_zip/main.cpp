#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>
#include "vector"
#include "stack"
#include <filesystem>

class DirectoryScanner {
public:
    std::vector<std::string> files;
    void print_directory_contents(const std::string& root_path) {
        std::vector<std::string> directories = {root_path};

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
                    size_t pos = dir_path.find(root_path);
                    if (pos == std::string::npos) {
                        std::cerr << "Error: root path not found in directory path " << dir_path << std::endl;
                        continue;
                    }
                    std::string parent_path = dir_path.substr(pos + root_path.length());
                    if (parent_path.front() == '/') {
                        parent_path = parent_path.substr(1);
                    }
//                    std::cout << parent_path << "/" << entry->d_name << std::endl;
                    this->files.push_back(std::string(parent_path).append(entry->d_name));
                }
            }

            closedir(dir);
        }
    }
};

int main(int argc, char *argv[]) {
    DirectoryScanner directoryScanner;
    directoryScanner.print_directory_contents("/Users/kirillzhukov/Desktop/test");
//    print_directory_contents("/Users/kirillzhukov/Desktop/test", "");
    for(auto it : directoryScanner.files) {
        printf("dir: %s\n", it.c_str());
    }
    return 0;
}

