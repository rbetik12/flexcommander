#include <zconf.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <HFSCatalog.h>
#include <stdlib.h>
#include <List.h>
#include <HFSPlusBTree.h>
#include "Copy.h"
#include "../utils/Utils.h"

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
    if (destFile == NULL) {
        fprintf(stderr, "Unexpected NULL destination file!\n");
        return;
    }
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

void CopyDirectory(const char* _src, const char* _dest, uint32_t parentID, BTHeaderRec btreeHeader, FlexCommanderFS fs) {
    CopyInfo copyInfo;
    const size_t _srcLen = strlen(_src) + 256;
    const size_t _destLen = strlen(_dest) + 256;
    char *src = calloc(_srcLen, 1);
    char *dest = calloc(_destLen, 1);
    strcpy(src, _src);
    strcpy(dest, _dest);

    char *srcCopy = calloc(_srcLen, 1);
    strcpy(srcCopy, _src);
    PathListNode* list = SplitPathWithDelimeter(srcCopy, "/");
    free(srcCopy);

    if (list) {
        PathListNode* lastNode = GetPathListLastNode(&list);
        strcat(dest, "/");
        strcat(dest, lastNode->token);
    }

    MakePath(dest);
    printf("Created directory %s successfully!\n", dest);

    copyInfo.dest = dest;
    PathListNode * childrenDirs = GetChildrenDirectoriesList(parentID, btreeHeader, fs, copyInfo);
    PathListNode * childrenDirsListHead = childrenDirs;
    srcCopy = calloc(_srcLen, 1);

    while(childrenDirs) {
        strcpy(srcCopy, _src);
        strcat(srcCopy, "/");
        strcat(srcCopy, childrenDirs->token);
        CopyDirectory(srcCopy, dest, childrenDirs->_cnid, btreeHeader, fs);
        memset(srcCopy, 0, _srcLen);
        childrenDirs = childrenDirs->next;
    }

    free(src);
    free(dest);
    free(srcCopy);
}