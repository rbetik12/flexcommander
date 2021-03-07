#include "HFSPlusBTree.h"
#include <FlexIO.h>
#include <stdlib.h>
#include <HFSCatalog.h>
#include "utils/Endians.h"

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