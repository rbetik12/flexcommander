#include "HFSPlusBTree.h"
#include <FlexIO.h>
#include <stdlib.h>
#include <HFSCatalog.h>
#include <List.h>
#include <stdbool.h>
#include <memory.h>
#include "utils/Endians.h"

// Use only with structures. Idk why, but it doesn't work with 4 byte values.
#define CAST_PTR_TO_TYPE(type, ptr) *(type*)ptr

bool HFSStrToStrCmp(HFSUniStr255 hfsStr, const char* str) {
    size_t strLen = strlen(str);
    if (strLen != hfsStr.length) {
        return false;
    }

    int i = 0;
    for (i = 0; i < hfsStr.length; i++) {
        if (hfsStr.unicode[i] != str[i]) {
            return false;
        }
    }

    return true;
}

//Checks whether given node is HFS+ Private Data node
bool CheckForHFSPrivateDataNode(HFSPlusCatalogKey key) {
    if (key.nodeName.length != 21) return false;
    uint16_t checkSymbols[21] = {0, 0, 0, 0, 72, 70, 83, 43, 32, 80, 114, 105, 118, 97, 116, 101, 32, 68, 97, 116, 97};
    for (int i = 0; i < 21; i++) {
        if (key.nodeName.unicode[i] != checkSymbols[i]) return false;
    }
    return true;
}

uint32_t ParseLeafNode(char* rawNode, const char* folderName, uint32_t folderParentId, BTHeaderRec btreeHeader, BTNodeDescriptor descriptor) {
    uint16_t recordAddress[descriptor.numRecords];
    int j = 0;
    for (int i = btreeHeader.nodeSize - 1; i >= btreeHeader.nodeSize - descriptor.numRecords * 2; i -= 2) {
        recordAddress[j] = (rawNode[i - 1] << 8) | (uint8_t)rawNode[i];
        j += 1;
    }

    for (int i = 0; i < descriptor.numRecords; i++) {
        HFSPlusCatalogKey key;
        key = CAST_PTR_TO_TYPE(HFSPlusCatalogKey, (rawNode + recordAddress[i]));
        ConvertCatalogKey(&key);

        if (key.parentID != folderParentId) continue;

        uint16_t recordType = rawNode[recordAddress[i] + key.keyLength + sizeof(key.keyLength) + 1];
        HFSPlusCatalogFolder catalogFolder;
        if (recordType == FolderRecord) {
            catalogFolder = CAST_PTR_TO_TYPE(HFSPlusCatalogFolder,
                                             (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
            ConvertCatalogFolder(&catalogFolder);
            if (HFSStrToStrCmp(key.nodeName, folderName)) {
                return catalogFolder.folderID;
            }
        }
        else {
            continue;
        }
    }

    return 0;
}

uint32_t FindIdOfFolder(const char* folderName, uint32_t folderParentId, BTHeaderRec catalogBTHeader, FlexCommanderFS fs) {
    char* rawNode = calloc(sizeof(char), fs.blockSize);
    uint64_t nodeBlockNumber = catalogBTHeader.firstLeafNode + fs.catalogFileBlock;
    BTNodeDescriptor descriptor;
    bool isLastNode = false;
    uint32_t id;

    FlexFSeek(fs.file, nodeBlockNumber * fs.blockSize, SEEK_SET);
    FlexRead(rawNode, fs.blockSize, 1, fs.file);

    while (!isLastNode) {
        descriptor = CAST_PTR_TO_TYPE(BTNodeDescriptor, rawNode);
        ConvertBTreeNodeDescriptor(&descriptor);
        PrintBTreeNodeDescriptor(descriptor);
        if (descriptor.fLink == 0) {
            isLastNode = true;
        }

        id = ParseLeafNode(rawNode, folderName, folderParentId, catalogBTHeader, descriptor);
        if (id != 0) break;

        nodeBlockNumber = fs.catalogFileBlock + descriptor.fLink;
        FlexFSeek(fs.file, nodeBlockNumber * fs.blockSize, SEEK_SET);
        FlexRead(rawNode, fs.blockSize, 1, fs.file);
    }

    free(rawNode);

    return id;
}