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

int Verify(FlexCommanderFS* fs);

int ReadBtreeHeader(uint64_t pos, FlexCommanderFS* fs);

void ExtractCatalogBtreeHeader(uint64_t block, BTHeaderRec* header, FlexCommanderFS* fs);

PathListNode* SplitPath(char* path) {
    PathListNode* start = NULL;
    PathListNode* node = calloc(1, sizeof(PathListNode));
    start = node;
    char* pathToken;
    node->token = "/";

    while ((pathToken = strsep(&path, "/"))) {
        if (strcmp(pathToken, "") == 0) {
            continue;
        }
        node->next = calloc(1, sizeof(PathListNode));
        node = node->next;
        node->token = pathToken;
    }

    return start;
}

uint64_t GetCatalogueFileLocation(HFSPlusVolumeHeader header, FlexCommanderFS* fs) {
    uint64_t catalogFirstBlockNum = htonl(header.catalogFile.extents[0].startBlock);
    uint64_t catalogFileLocation = catalogFirstBlockNum * fs->blockSize;
    printf("Catalog file location: %lx\n", catalogFileLocation);
    printf("Catalog file first block: %lu\n", catalogFirstBlockNum);
    return catalogFileLocation;
}

void PrintVolumeHeader(HFSPlusVolumeHeader header, FlexCommanderFS* fs) {
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

int FlexOpen(const char* path, FlexCommanderFS* fs) {
    FILE* dev = fopen(path, "rb");

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

int Verify(FlexCommanderFS* fs) {

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
        return 0;
    } else {
        fprintf(stderr, "Provided file is not a HFS volume!\n");
        return -1;
    }
}

int FlexListDirContent(const char* path, FlexCommanderFS* fs) {
    char* pathCopy = malloc(strlen(path) + 1);
    strcpy(pathCopy, path);
    PathListNode* list = SplitPath(pathCopy);
    free(pathCopy);

    BTHeaderRec catalogFileHeader;
    ExtractCatalogBtreeHeader(fs->catalogFileBlock, &catalogFileHeader, fs);
    uint64_t rootFolderLeafBlock;
    while (list) {
        if (strcmp(list->token, "/") == 0) {
            rootFolderLeafBlock =
                    GetRecordBlockNumByCNID(catalogFileHeader.rootNode + fs->catalogFileBlock, 1, catalogFileHeader, *fs) + fs->catalogFileBlock;
            if (rootFolderLeafBlock == 0) {
                ParseLeafNode(catalogFileHeader.rootNode + fs->catalogFileBlock, 2, catalogFileHeader, *fs);
            }
            else if (rootFolderLeafBlock == -1) {
                fputs("Unexpected error! Didn't find a record.\n", stderr);
            }
            else {
                FSRecordListNode *fsRecordList = ParseLeafNode(rootFolderLeafBlock, 2, catalogFileHeader, *fs);
                while (fsRecordList) {
                    if (fsRecordList->name.length) {
                        for (int i = 0; i < fsRecordList->name.length; i++) {
                            printf("%lc", fsRecordList->name.unicode[i]);
                        }
                        if (fsRecordList->type == FolderRecord) {
                            printf("/");
                        }
                        printf("\n");
                    }
                    fsRecordList = fsRecordList->next;
                }
            }
        }
        list = list->next;
    }
    return 0;
}

void ExtractCatalogBtreeHeader(uint64_t block, BTHeaderRec* header, FlexCommanderFS* fs) {
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
