#include <zconf.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <HFSCatalog.h>
#include <stdlib.h>
#include "Copy.h"

void MakePath(char* dir) {
    char cwd[512];
    if (dir[0] == '.') {
        getcwd(cwd, sizeof(cwd));
    }

    char tmp[1024];
    char* p = NULL;
    size_t len;

    if (dir[0] == '.') {
        snprintf(tmp, sizeof(tmp), "%s%s", cwd, dir + 1);
    } else {
        snprintf(tmp, sizeof(tmp), "%s", dir);
    }

    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    mkdir(tmp, S_IRWXU);
}

void CopyFileBlock(uint64_t blockNum, FILE* fileDestination, FlexCommanderFS* fs) {
    char* fileBlock = calloc(1, fs->blockSize);
    fseek(fs->file, blockNum * fs->blockSize, SEEK_SET);
    uint32_t read = fread(fileBlock, fs->blockSize, 1, fs->file);
    fwrite(fileBlock, fs->blockSize, 1, fileDestination);
    free(fileBlock);
}

void CopyFile(const char* dest, const char* filename, HFSPlusCatalogFile file, FlexCommanderFS* fs) {
    MakePath(dest);
    char filePath[512];
    snprintf(filePath, sizeof(filePath), "%s/%s", dest, filename);
    FILE* destFile = NULL;
    destFile = fopen(filePath, "wb");
    for (int i = 0; i < 8; i++) {
        HFSPlusExtentRecord extent = file.dataFork.extents[i];
        if (extent.startBlock != 0 && extent.blockCount != 0) {
            for (int offset = 0; offset < extent.blockCount; offset++) {
                CopyFileBlock(extent.startBlock + offset, destFile, fs);
            }
        }
    }
    fclose(destFile);
}