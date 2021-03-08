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

int ReadBtreeHeader(uint64_t pos, FlexCommanderFS* fs) {
    BTNodeDescriptor btreeHeaderDescr;
    BTHeaderRec btreeHeader;

    FlexFSeek(fs->file, pos, SEEK_SET);
    FlexRead(&btreeHeaderDescr, sizeof(BTNodeDescriptor), 1, fs->file);
    FlexRead(&btreeHeader, sizeof(BTHeaderRec), 1, fs->file);
    ConvertBTreeHeader(&btreeHeader);
    ConvertBTreeNodeDescriptor(&btreeHeaderDescr);

//    ParseNode(btreeHeader.rootNode + pos / fs->blockSize, btreeHeader, *fs);
    return 0;
}

int FlexListDirContent(const char* path, FlexCommanderFS* fs) {
    char* pathCopy = malloc(strlen(path) + 1);
    strcpy(pathCopy, path);
    PathListNode* list = SplitPath(pathCopy);
    free(pathCopy);

//    while (list) {
//        printf("%s\n", list->token);
//        list = list->next;
//    }

    BTHeaderRec catalogFileHeader;
    ExtractCatalogBtreeHeader(fs->catalogFileBlock, &catalogFileHeader, fs);
    ParseRootNode(catalogFileHeader.rootNode + fs->catalogFileBlock, catalogFileHeader, *fs);
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
