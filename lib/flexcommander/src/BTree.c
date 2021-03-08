#include "HFSPlusBTree.h"
#include <FlexIO.h>
#include <stdlib.h>
#include <HFSCatalog.h>
#include "utils/Endians.h"

// Use only with structures. Idk why, but it doesn't work with 4 byte values.
#define CAST_PTR_TO_TYPE(type, ptr) *(type*)ptr

void ParseFolderRecord(HFSPlusCatalogFolder *folder) {
    PrintCatalogFolder(*folder);
}

void ParseFileRecord(HFSPlusCatalogFile *file, FlexCommanderFS fs) {
    char* dataBlock = calloc(fs.blockSize, sizeof(char));
    PrintCatalogFile(*file);
    printf("File data:\n");
    for (int i = 0; i < 8; i++) {
        HFSPlusExtentRecord extent = file->dataFork.extents[i];
        if (extent.blockCount != 0 && extent.startBlock != 0) {
            uint64_t blockAddress = extent.startBlock * fs.blockSize;
            FlexFSeek(fs.file, blockAddress, SEEK_SET);
            FlexRead(dataBlock, fs.blockSize, 1, fs.file);
            for (int j = 0; j < fs.blockSize; j++) {
                printf("%c", dataBlock[j]);
            }
            printf("\n");
        }
    }
    free(dataBlock);
}

void ParseCatalogThread(HFSPlusCatalogThread *catalogThread) {
    PrintCatalogThread(*catalogThread);
}

void ParseNode(uint64_t nodeBlock, BTHeaderRec btreeHeader, FlexCommanderFS fs) {
    uint64_t nodeBlockAddress = nodeBlock * fs.blockSize;
    FlexFSeek(fs.file, nodeBlockAddress, SEEK_SET);

    char* rawNode = calloc(btreeHeader.nodeSize, sizeof(char));

    FlexRead(rawNode, btreeHeader.nodeSize, 1, fs.file);
    BTNodeDescriptor nodeDescr = CAST_PTR_TO_TYPE(BTNodeDescriptor, rawNode);
    ConvertBTreeNodeDescriptor(&nodeDescr);

    uint16_t recordAddress[nodeDescr.numRecords];
    int j = 0;
    for (int i = btreeHeader.nodeSize - 1; i >= btreeHeader.nodeSize - nodeDescr.numRecords * 2; i -= 2) {
        recordAddress[j] = (rawNode[i - 1] << 8) | (uint8_t)rawNode[i];
        j += 1;
    }

    printf("==========================================\n");
    for (int i = 0; i < nodeDescr.numRecords; i++) {
        HFSPlusCatalogKey key;
        key = CAST_PTR_TO_TYPE(HFSPlusCatalogKey, (rawNode + recordAddress[i]));
        ConvertCatalogKey(&key);
        PrintCatalogKey(key);

        uint16_t recordType = rawNode[recordAddress[i] + key.keyLength + sizeof(key.keyLength) + 1];
        HFSPlusCatalogFolder catalogFolder;
        HFSPlusCatalogFile catalogFile;
        HFSPlusCatalogThread catalogThread;

        switch (recordType) {
            case FolderRecord:
                catalogFolder = CAST_PTR_TO_TYPE(HFSPlusCatalogFolder,
                                                 (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
                ConvertCatalogFolder(&catalogFolder);
                ParseFolderRecord(&catalogFolder);
                break;
            case FileRecord:
                catalogFile = CAST_PTR_TO_TYPE(HFSPlusCatalogFile, (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
                ConvertCatalogFile(&catalogFile);
                ParseFileRecord(&catalogFile, fs);
                break;
            case FileThreadRecord:
            case FolderThreadRecord:
                catalogThread = CAST_PTR_TO_TYPE(HFSPlusCatalogThread, (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
                ConvertCatalogThread(&catalogThread);
                ParseCatalogThread(&catalogThread);
                break;
            default:
                fprintf(stderr, "Unknown data record type!\n");
                break;
        }
        printf("==========================================\n");
    }

    free(rawNode);
}

void ParseIndexNode(const char * rawNode, BTNodeDescriptor descriptor, BTHeaderRec btreeHeader) {
    uint16_t recordAddress[descriptor.numRecords];
    int j = 0;
    for (int i = btreeHeader.nodeSize - 1; i >= btreeHeader.nodeSize - descriptor.numRecords * 2; i -= 2) {
        recordAddress[j] = (rawNode[i - 1] << 8) | (uint8_t)rawNode[i];
        j += 1;
    }

    for (int i = 0; i < descriptor.numRecords; i++) {
        BTCatalogIndexNode node = CAST_PTR_TO_TYPE(BTCatalogIndexNode, (rawNode + recordAddress[i]));
        ConvertCatalogIndexNode(&node);
        uint64_t offset = recordAddress[i] + node.key.keyLength + sizeof(node.key.keyLength);
        uint32_t nextNode = rawNode[offset] << 24 | rawNode[offset + 1] << 16 | rawNode[offset + 2] << 8 | rawNode[offset + 3];
        node.nextNode = nextNode;
        PrintCatalogIndexNode(node);
    }
}

void ParseRootNode(uint64_t block, BTHeaderRec btHeader, FlexCommanderFS fs) {
    uint64_t nodeBlockAddress = block * fs.blockSize;
    FlexFSeek(fs.file, nodeBlockAddress, SEEK_SET);

    char* rawNode = calloc(btHeader.nodeSize, sizeof(char));

    FlexRead(rawNode, btHeader.nodeSize, 1, fs.file);
    BTNodeDescriptor nodeDescr = CAST_PTR_TO_TYPE(BTNodeDescriptor, rawNode);
    ConvertBTreeNodeDescriptor(&nodeDescr);

    switch (nodeDescr.kind) {
        case IndexNode:
            ParseIndexNode(rawNode, nodeDescr, btHeader);
            break;
        case LeafNode:
            break;
        default:
            fputs("Unexpected node type!\n", stderr);
            break;
    }
}