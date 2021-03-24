#include <Flexcommander.h>
#include <HFSPlus.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <byteswap.h>
#include <HFSPlusBTree.h>
#include <FlexIO.h>
#include <List.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utils/Endians.h"
#include "copy/Copy.h"
#include "utils/Utils.h"

int Verify(FlexCommanderFS *fs);

int ReadBtreeHeader(uint64_t pos, FlexCommanderFS *fs);

void ExtractCatalogBtreeHeader(uint64_t block, BTHeaderRec *header, FlexCommanderFS *fs);

PathListNode *SplitPath(char *path) { // TO DO: Get rid of this function
    PathListNode *listHead = NULL;
    char *pathToken;
    PathListNode node;
    node.token = "/";
    PathListAdd(&listHead, node);

    while ((pathToken = strsep(&path, "/"))) {
        if (strcmp(pathToken, "") == 0) {
            continue;
        }
        PathListNode newNode;
        memset(&newNode, 0, sizeof(PathListNode));
        newNode.token = calloc(sizeof(char), strlen(pathToken) + 1);
        newNode.token = strcpy(newNode.token, pathToken);
        PathListAdd(&listHead, newNode);
    }

    return listHead;
}

uint64_t GetCatalogueFileLocation(HFSPlusVolumeHeader header, FlexCommanderFS *fs) {
    uint64_t catalogFirstBlockNum = htonl(header.catalogFile.extents[0].startBlock);
    uint64_t catalogFileLocation = catalogFirstBlockNum * fs->blockSize;
    printf("Catalog file location: %lx\n", catalogFileLocation);
    printf("Catalog file first block: %lu\n", catalogFirstBlockNum);
    return catalogFileLocation;
}

void PrintVolumeHeader(HFSPlusVolumeHeader header, FlexCommanderFS *fs) {
    printf("Volume info:\n");
    printf("Total files: %d\n", htonl(header.fileCount));
    printf("Total folders: %d\n", htonl(header.folderCount));
    printf("Block size: %d\n", fs->blockSize);
    printf("Total blocks: %d\n", htonl(header.totalBlocks));
    printf("Next free block: %d\n", htonl(header.nextAllocation));

    printf("\nCatalog file info:\n");
    printf("Catalog file logical size: %lu\n", bswap_64(header.catalogFile.logicalSize));
    printf("Catalog file clump size: %u\n", htonl(header.catalogFile.clumpSize));
    printf("Catalog file total blocks: %u\n", htonl(header.catalogFile.totalBlocks));
}

int FlexOpen(const char *path, FlexCommanderFS *fs) {
    FILE *dev = fopen(path, "rb");

    if (!dev) {
        perror("Can't open dev!\n");
        return -1;
    }

    fs->file = dev;
    if (Verify(fs)) {
        return -1;
    }

    return 0;
}

int Verify(FlexCommanderFS *fs) {

    HFSPlusVolumeHeader header;

    if (fseek(fs->file, HFS_START_OFFSET, SEEK_SET)) {
        fprintf(stderr, "Can't set 1024 bytes offset!\n");
        return -1;
    }

    if (fread(&header, sizeof(HFSPlusVolumeHeader), 1, fs->file) != 1) {
        if (feof(fs->file)) {
            fprintf(stderr, "Unexpected EOF!\n");
        } else {
            fprintf(stderr, "Can't read HFS volume header!\n");
        }
        return -1;
    }

    if (header.signature == HFS_SIGNATURE) {
        fs->blockSize = htonl(header.blockSize);
        fs->catalogFileBlock = bswap_32(header.catalogFile.extents[0].startBlock);
        int extents = 0;
        int totalBlocks = 0;
        for (int i = 0; i < 8; i++) {
            header.catalogFile.extents[i].startBlock = bswap_32(header.catalogFile.extents[i].startBlock);
            header.catalogFile.extents[i].blockCount = bswap_32(header.catalogFile.extents[i].blockCount);
            totalBlocks += header.catalogFile.extents[i].blockCount;
            if (header.catalogFile.extents[i].startBlock != 0 && header.catalogFile.extents[i].blockCount != 0) {
                extents += 1;
            }
        }
        fs->volumeHeader = header;
        return 0;
    } else {
        fprintf(stderr, "Provided file is not a HFS volume!\n");
        return -1;
    }
}

int FlexListDirContent(const char *path, FlexCommanderFS *fs) {
    if (strcmp(path, "") == 0 | strcmp(path, "\n") == 0) {
        printf("Path doesn't exist!\n");
        return 0;
    }
    size_t strLength = strlen(path) + 1;
    char *pathCopy = malloc(strLength);
    memcpy(pathCopy, path, strLength);
    size_t pathCopyLength = strlen(pathCopy);
    if (pathCopy[pathCopyLength - 1] == '\n') {
        pathCopy[strlen(pathCopy) - 1] = 0;
    }
    PathListNode *list = SplitPath(pathCopy);
    PathListNode *listStart = list;
    free(pathCopy);

    BTHeaderRec catalogFileHeader;
    ExtractCatalogBtreeHeader(fs->catalogFileBlock, &catalogFileHeader, fs);

    uint32_t parentID = 2;
    while (list) {
        if (list->next == NULL) {
            if (parentID == 0) {
                printf("Path doesn't exist!\n");
            }
            ListDirectoryContent(parentID, catalogFileHeader, *fs);
            break;
        }
        else {
            parentID = FindIdOfFolder(list->next->token, parentID, catalogFileHeader, *fs);
        }
        list = list->next;
    }

    PathListClear(listStart);
    return 0;
}

int FlexSetCurrentDir(const char* path, FlexCommanderFS* fs) {
    if (strcmp(path, "") == 0 | strcmp(path, "\n") == 0) {
        return -1;
    }
    int result = -1;
    char *pathCopy = malloc(strlen(path) + 1);
    strcpy(pathCopy, path);
    PathListNode *list = SplitPath(pathCopy);
    PathListNode *listHead = list;
    free(pathCopy);

    BTHeaderRec catalogFileHeader;
    ExtractCatalogBtreeHeader(fs->catalogFileBlock, &catalogFileHeader, fs);

    uint32_t parentID = 2;
    while (list) {
        if (list->next == NULL) {
            if (parentID == 0) {
                result = -1;
                break;
            }
            result = 0;
            break;
        }
        else {
            parentID = FindIdOfFolder(list->next->token, parentID, catalogFileHeader, *fs);
        }
        list = list->next;
    }
    PathListClear(listHead);
    return result;
}

void ExtractCatalogBtreeHeader(uint64_t block, BTHeaderRec *header, FlexCommanderFS *fs) {
    BTNodeDescriptor btreeHeaderDescr;
    FlexFSeek(fs->file, block * fs->blockSize, SEEK_SET);
    FlexRead(&btreeHeaderDescr, sizeof(BTNodeDescriptor), 1, fs->file);

    if (btreeHeaderDescr.kind != HeaderNode) {
        fputs("Unexpected node type. Expected header type!\n", stderr);
        return;
    }

    FlexRead(header, sizeof(BTHeaderRec), 1, fs->file);
    ConvertBTreeHeader(header);
    ConvertBTreeNodeDescriptor(&btreeHeaderDescr);
}

int FlexCopy(const char* path, const char* currentDir, FlexCommanderFS* fs) {
    PathListNode *list = SplitPathWithDelimeter(path, " ");
    PathListNode *listHead = list;
    char *src;
    char *dest;
    uint32_t i = 0;

    while (list) {
        if (i == 0) {
            src = list->token;
        }
        else {
            dest = list->token;
            if (dest[strlen(dest) - 1] == '\n') {
                dest[strlen(dest) - 1] = '\0';
            }
        }
        i++;
        list = list->next;
    }
    PathListClear(listHead);

    char srcPath[1024];
    if (src[0] == '.') {
        if (strlen(src) > 1) {
            snprintf(srcPath, sizeof(srcPath), "%s%s", currentDir, src + 1);
        }
        else {
            snprintf(srcPath, sizeof(srcPath), "%s", currentDir);
        }
    }
    else {
        snprintf(srcPath, sizeof(srcPath), "%s", src);
    }

    BTHeaderRec header;

    ExtractCatalogBtreeHeader(fs->catalogFileBlock, &header, fs);

    char * srcPathCopy = calloc(strlen(srcPath) + 1, 1);
    strcpy(srcPathCopy, srcPath);
    PathListNode *splitedSrcPathList = SplitPath(srcPathCopy);
    PathListNode *splitedSrcPathListStart = splitedSrcPathList;
    free(srcPathCopy);

    uint32_t parentID = 2;
    while (splitedSrcPathList) {
        if (splitedSrcPathList->next == NULL) {
            if (parentID == 0) {
                parentID = 0;
                break;
            }
            break;
        }
        else {
            parentID = FindIdOfFolder(splitedSrcPathList->next->token, parentID, header, *fs);
        }
        splitedSrcPathList  = splitedSrcPathList->next;
    }

    if (parentID != 0) {
        CopyDirectory(srcPath, dest, parentID, header, *fs);
    }
    else {
        splitedSrcPathList = splitedSrcPathListStart;
        parentID = 2;
        HFSPlusCatalogFile *file = NULL;
        while (splitedSrcPathList) {
            if (splitedSrcPathList->next == NULL) {
                if (parentID == 0) {
                    parentID = 0;
                    break;
                }
                file = GetFileRecord(parentID, header, *fs);
                break;
            } else {
                parentID = FindIdOfFile(splitedSrcPathList->next->token, parentID, header, *fs);
            }
            splitedSrcPathList = splitedSrcPathList->next;
        }
        if (parentID == 0) {
            printf("Something gone wrong! :(\n");
        }
        else {
            CopyFile(dest, splitedSrcPathList->token, *file, fs);
            printf("Copied file successfully!\n");
        }
        free(file);
    }

    PathListClear(splitedSrcPathListStart);
    return 0;
}
