#include "ZIPArchive.h"

int main(int argc, char *argv[]) {
    ZIPArchive archiver("/Users/kirillzhukov/Desktop/mvfst", "/Users/kirillzhukov/Desktop/ar/mvfst.zip");
    archiver.archive();
    return 0;
}

