//
// Created by Kirill Zhukov on 10.05.2023.
//

#include "zip.h"

std::vector<filename> indexing_files(char *path) {
    DIR *dir;
    struct dirent *entry;
    dir = opendir(path);
    int files = 0;

    while((entry=readdir(dir))) {
        files++;
        printf("File %3d: %s\n", files, entry->d_name);
    }
}