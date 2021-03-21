#include "HFSPlusBTree.h"
#include <FlexIO.h>
#include <stdlib.h>
#include <HFSCatalog.h>
#include <List.h>
#include <stdbool.h>
#include <memory.h>
#include "utils/Endians.h"
#include "copy/Copy.h"

// GOD BLESS ME TO REWRITE CODE BELOW


// Use only with structures. Idk why, but it doesn't work with 4 byte values.
#define CAST_PTR_TO_TYPE(type, ptr) *(type*)ptr

char* HFSStringToBytes(HFSUniStr255 hfsStr) {
    char* str = calloc(hfsStr.length + 1, 1);
    for (int i = 0; i < hfsStr.length; i++) {
        str[i] = hfsStr.unicode[i];
    }
    return str;
}

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

void PrintHFSUnicode(HFSUniStr255 str) {
    for (int i = 0; i < str.length; i++) {
        printf("%lc", str.unicode[i]);
    }
}

uint32_t ParseLeafNode(char* rawNode, const char* folderName, uint32_t folderParentId, BTHeaderRec btreeHeader,
                       BTNodeDescriptor descriptor) {
    uint16_t recordAddress[descriptor.numRecords];
    int j = 0;
    for (int i = btreeHeader.nodeSize - 1; i >= btreeHeader.nodeSize - descriptor.numRecords * 2; i -= 2) {
        recordAddress[j] = (rawNode[i - 1] << 8) | (uint8_t) rawNode[i];
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
        } else {
            continue;
        }
    }

    return 0;
}

void ParseLeafNodeContent(char* rawNode, uint32_t parentID, BTHeaderRec btreeHeader, FlexCommanderFS fs,
                          BTNodeDescriptor descriptor) {
    uint16_t recordAddress[descriptor.numRecords];
    int j = 0;
    for (int i = btreeHeader.nodeSize - 1; i >= btreeHeader.nodeSize - descriptor.numRecords * 2; i -= 2) {
        recordAddress[j] = (rawNode[i - 1] << 8) | (uint8_t) rawNode[i];
        j += 1;
    }

    for (int i = 0; i < descriptor.numRecords; i++) {
        HFSPlusCatalogKey key;
        key = CAST_PTR_TO_TYPE(HFSPlusCatalogKey, (rawNode + recordAddress[i]));
        ConvertCatalogKey(&key);

        if (key.parentID != parentID | key.nodeName.length == 0) continue;

        uint16_t recordType = rawNode[recordAddress[i] + key.keyLength + sizeof(key.keyLength) + 1];
        HFSPlusCatalogFolder catalogFolder;
        HFSPlusCatalogFile catalogFile;

        if (CheckForHFSPrivateDataNode(key)) {
            continue;
        }

        PrintHFSUnicode(key.nodeName);

        switch (recordType) {
            case FolderRecord:
                catalogFolder = CAST_PTR_TO_TYPE(HFSPlusCatalogFolder,
                                                 (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
                ConvertCatalogFolder(&catalogFolder);
                printf("/");
                break;
            case FileRecord:
                catalogFile = CAST_PTR_TO_TYPE(HFSPlusCatalogFile,
                                               (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
                ConvertCatalogFile(&catalogFile);
                break;
            case FileThreadRecord:
            case FolderThreadRecord:
                break;
            default:
                fprintf(stderr, "Unknown data record type!\n");
                break;
        }
        printf("\n");
    }
}

void ListDirectoryContent(uint32_t parentID, BTHeaderRec catalogBTHeader, FlexCommanderFS fs) {
    char* rawNode = calloc(sizeof(char), fs.blockSize);
    uint64_t nodeBlockNumber = catalogBTHeader.firstLeafNode + fs.catalogFileBlock;
    BTNodeDescriptor descriptor;
    bool isLastNode = false;
    uint32_t id;
    uint32_t extentNum = 0;
    uint32_t blockNum = 0;

    FlexFSeek(fs.file, nodeBlockNumber * fs.blockSize, SEEK_SET);
    FlexRead(rawNode, fs.blockSize, 1, fs.file);

    while (!isLastNode) {
        descriptor = CAST_PTR_TO_TYPE(BTNodeDescriptor, rawNode);
        ConvertBTreeNodeDescriptor(&descriptor);
        if (descriptor.fLink == 0) {
            isLastNode = true;
        }

        ParseLeafNodeContent(rawNode, parentID, catalogBTHeader, fs, descriptor);
        if (blockNum == fs.volumeHeader.catalogFile.extents[extentNum].blockCount - 1) {
            blockNum = 0;
            extentNum += 1;
        }
        nodeBlockNumber = fs.volumeHeader.catalogFile.extents[extentNum].startBlock
                          + (descriptor.fLink % fs.volumeHeader.catalogFile.extents[extentNum].blockCount);

        FlexFSeek(fs.file, nodeBlockNumber * fs.blockSize, SEEK_SET);
        FlexRead(rawNode, fs.blockSize, 1, fs.file);
        blockNum += 1;
    }

    free(rawNode);
}

uint32_t FindIdOfFolder(const char* folderName, uint32_t folderParentId, BTHeaderRec catalogBTHeader, FlexCommanderFS fs) {
    char* rawNode = calloc(sizeof(char), fs.blockSize);
    uint64_t nodeBlockNumber = catalogBTHeader.firstLeafNode + fs.catalogFileBlock;
    BTNodeDescriptor descriptor;
    bool isLastNode = false;
    uint32_t id;
    uint32_t extentNum = 0;
    uint32_t blockNum = 0;

    FlexFSeek(fs.file, nodeBlockNumber * fs.blockSize, SEEK_SET);
    FlexRead(rawNode, fs.blockSize, 1, fs.file);

    while (!isLastNode) {
        descriptor = CAST_PTR_TO_TYPE(BTNodeDescriptor, rawNode);
        ConvertBTreeNodeDescriptor(&descriptor);
        if (descriptor.fLink == 0) {
            isLastNode = true;
        }

        id = ParseLeafNode(rawNode, folderName, folderParentId, catalogBTHeader, descriptor);
        if (id != 0) break;
        if (blockNum == fs.volumeHeader.catalogFile.extents[extentNum].blockCount - 1) {
            blockNum = 0;
            extentNum += 1;
        }
        nodeBlockNumber = fs.volumeHeader.catalogFile.extents[extentNum].startBlock
                          + (descriptor.fLink % fs.volumeHeader.catalogFile.extents[extentNum].blockCount);

        FlexFSeek(fs.file, nodeBlockNumber * fs.blockSize, SEEK_SET);
        FlexRead(rawNode, fs.blockSize, 1, fs.file);
        blockNum += 1;
    }

    free(rawNode);

    return id;
}


// That shit below must be rewritten

HFSPlusCatalogFile* GetFileRecordFromLeafNode(uint32_t fileId, BTNodeDescriptor descriptor, BTHeaderRec btreeHeader, char* rawNode) {
    uint16_t recordAddress[descriptor.numRecords];
    int j = 0;
    for (int i = btreeHeader.nodeSize - 1; i >= btreeHeader.nodeSize - descriptor.numRecords * 2; i -= 2) {
        recordAddress[j] = (rawNode[i - 1] << 8) | (uint8_t) rawNode[i];
        j += 1;
    }

    HFSPlusCatalogFile *catalogFile = malloc(sizeof(HFSPlusCatalogFile));
    for (int i = 0; i < descriptor.numRecords; i++) {
        HFSPlusCatalogKey key;
        key = CAST_PTR_TO_TYPE(HFSPlusCatalogKey, (rawNode + recordAddress[i]));
        ConvertCatalogKey(&key);

        uint16_t recordType = rawNode[recordAddress[i] + key.keyLength + sizeof(key.keyLength) + 1];
        if (recordType == FileRecord) {
            *catalogFile = CAST_PTR_TO_TYPE(HFSPlusCatalogFile, (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
            ConvertCatalogFile(catalogFile);
            if (catalogFile->fileID == fileId) {
                return catalogFile;
            }
            else {
                continue;
            }
        } else {
            continue;
        }
    }

    free(catalogFile);
    return NULL;
}

uint32_t ParseLeafNodeWithCondition(char* rawNode, const char* folderName, uint32_t folderParentId, BTHeaderRec btreeHeader,
                                    BTNodeDescriptor descriptor, enum HFSDataRecordType recordTypeToFind) {
    uint16_t recordAddress[descriptor.numRecords];
    int j = 0;
    for (int i = btreeHeader.nodeSize - 1; i >= btreeHeader.nodeSize - descriptor.numRecords * 2; i -= 2) {
        recordAddress[j] = (rawNode[i - 1] << 8) | (uint8_t) rawNode[i];
        j += 1;
    }

    for (int i = 0; i < descriptor.numRecords; i++) {
        HFSPlusCatalogKey key;
        key = CAST_PTR_TO_TYPE(HFSPlusCatalogKey, (rawNode + recordAddress[i]));
        ConvertCatalogKey(&key);

        if (key.parentID != folderParentId) continue;

        uint16_t recordType = rawNode[recordAddress[i] + key.keyLength + sizeof(key.keyLength) + 1];
        HFSPlusCatalogFolder catalogFolder;
        HFSPlusCatalogFile catalogFile;
        if (recordType == FolderRecord && recordTypeToFind == FolderRecord) {
            catalogFolder = CAST_PTR_TO_TYPE(HFSPlusCatalogFolder,
                                             (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
            ConvertCatalogFolder(&catalogFolder);
            if (HFSStrToStrCmp(key.nodeName, folderName)) {
                return catalogFolder.folderID;
            }
        } else if (recordType == FileRecord && recordTypeToFind == FileRecord) {
            catalogFile = CAST_PTR_TO_TYPE(HFSPlusCatalogFile, (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
            ConvertCatalogFile(&catalogFile);
            if (HFSStrToStrCmp(key.nodeName, folderName)) {
                return catalogFile.fileID;
            }
        } else {
            continue;
        }
    }

    return 0;
}

uint32_t FindIdOfFile(const char* fileName, uint32_t folderParentId, BTHeaderRec catalogBTHeader, FlexCommanderFS fs) {
    char* rawNode = calloc(sizeof(char), fs.blockSize);
    uint64_t nodeBlockNumber = catalogBTHeader.firstLeafNode + fs.catalogFileBlock;
    BTNodeDescriptor descriptor;
    bool isLastNode = false;
    uint32_t id;
    uint32_t extentNum = 0;
    uint32_t blockNum = 0;

    FlexFSeek(fs.file, nodeBlockNumber * fs.blockSize, SEEK_SET);
    FlexRead(rawNode, fs.blockSize, 1, fs.file);

    while (!isLastNode) {
        descriptor = CAST_PTR_TO_TYPE(BTNodeDescriptor, rawNode);
        ConvertBTreeNodeDescriptor(&descriptor);
        if (descriptor.fLink == 0) {
            isLastNode = true;
        }

        id = ParseLeafNodeWithCondition(rawNode, fileName, folderParentId, catalogBTHeader, descriptor, FileRecord);
        if (id == 0) {
            id = ParseLeafNodeWithCondition(rawNode, fileName, folderParentId, catalogBTHeader, descriptor, FolderRecord);
        }
        if (id != 0) break;
        if (blockNum == fs.volumeHeader.catalogFile.extents[extentNum].blockCount - 1) {
            blockNum = 0;
            extentNum += 1;
        }
        nodeBlockNumber = fs.volumeHeader.catalogFile.extents[extentNum].startBlock
                          + (descriptor.fLink % fs.volumeHeader.catalogFile.extents[extentNum].blockCount);

        FlexFSeek(fs.file, nodeBlockNumber * fs.blockSize, SEEK_SET);
        FlexRead(rawNode, fs.blockSize, 1, fs.file);
        blockNum += 1;
    }

    free(rawNode);

    return id;
}

HFSPlusCatalogFile* GetFileRecord(uint32_t fileId, BTHeaderRec catalogBTHeader, FlexCommanderFS fs) {
    char* rawNode = calloc(sizeof(char), fs.blockSize);
    uint64_t nodeBlockNumber = catalogBTHeader.firstLeafNode + fs.catalogFileBlock;
    BTNodeDescriptor descriptor;
    bool isLastNode = false;
    uint32_t extentNum = 0;
    uint32_t blockNum = 0;

    FlexFSeek(fs.file, nodeBlockNumber * fs.blockSize, SEEK_SET);
    FlexRead(rawNode, fs.blockSize, 1, fs.file);

    HFSPlusCatalogFile * catalogFile = NULL;

    while (!isLastNode) {
        descriptor = CAST_PTR_TO_TYPE(BTNodeDescriptor, rawNode);
        ConvertBTreeNodeDescriptor(&descriptor);
        if (descriptor.fLink == 0) {
            isLastNode = true;
        }

        catalogFile = GetFileRecordFromLeafNode(fileId, descriptor, catalogBTHeader, rawNode);
        if (catalogFile != NULL) {
            return catalogFile;
        }
        if (blockNum == fs.volumeHeader.catalogFile.extents[extentNum].blockCount - 1) {
            blockNum = 0;
            extentNum += 1;
        }
        nodeBlockNumber = fs.volumeHeader.catalogFile.extents[extentNum].startBlock
                          + (descriptor.fLink % fs.volumeHeader.catalogFile.extents[extentNum].blockCount);

        FlexFSeek(fs.file, nodeBlockNumber * fs.blockSize, SEEK_SET);
        FlexRead(rawNode, fs.blockSize, 1, fs.file);
        blockNum += 1;
    }

    free(rawNode);
    return NULL;
}

PathListNode* _GetChildrenDirs(uint32_t parentFolderId, const char* rawNode, BTHeaderRec btreeHeader,
                               BTNodeDescriptor descriptor, PathListNode** listHead, CopyInfo copyInfo, FlexCommanderFS fs) {
    uint16_t recordAddress[descriptor.numRecords];
    int j = 0;
    for (int i = btreeHeader.nodeSize - 1; i >= btreeHeader.nodeSize - descriptor.numRecords * 2; i -= 2) {
        recordAddress[j] = (rawNode[i - 1] << 8) | (uint8_t) rawNode[i];
        j += 1;
    }

    for (int i = 0; i < descriptor.numRecords; i++) {
        HFSPlusCatalogKey key;
        key = CAST_PTR_TO_TYPE(HFSPlusCatalogKey, (rawNode + recordAddress[i]));
        ConvertCatalogKey(&key);
        if (key.parentID != parentFolderId) continue;

        uint16_t recordType = rawNode[recordAddress[i] + key.keyLength + sizeof(key.keyLength) + 1];
        HFSPlusCatalogFolder catalogFolder;
        HFSPlusCatalogFile catalogFile;
        if (recordType == FolderRecord) {
            catalogFolder = CAST_PTR_TO_TYPE(HFSPlusCatalogFolder,
                                             (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
            ConvertCatalogFolder(&catalogFolder);
            PathListNode node;
            node.token = HFSStringToBytes(key.nodeName);
            node._cnid = catalogFolder.folderID;
            PathListAdd(listHead, node);
        } else if (recordType == FileRecord) {
            catalogFile = CAST_PTR_TO_TYPE(HFSPlusCatalogFile, (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
            ConvertCatalogFile(&catalogFile);
            CopyFile(copyInfo.dest, HFSStringToBytes(key.nodeName), catalogFile, &fs);
        } else {
            continue;
        }
    }
    return *listHead;
}

PathListNode* GetChildrenDirectoriesList(uint32_t parentFolderId, BTHeaderRec catalogBTHeader, FlexCommanderFS fs,
                                         CopyInfo copyInfo) {
    char* rawNode = calloc(sizeof(char), fs.blockSize);
    uint64_t nodeBlockNumber = catalogBTHeader.firstLeafNode + fs.catalogFileBlock;
    BTNodeDescriptor descriptor;
    bool isLastNode = false;
    uint32_t extentNum = 0;
    uint32_t currentBlockNum = catalogBTHeader.firstLeafNode;

    PathListNode* list = NULL;
    // Here's traversal algorithm that I am pretty fucking sure works
    while (!isLastNode) {
        FlexFSeek(fs.file, nodeBlockNumber * fs.blockSize, SEEK_SET);
        FlexRead(rawNode, fs.blockSize, 1, fs.file);
        descriptor = CAST_PTR_TO_TYPE(BTNodeDescriptor, rawNode);
        ConvertBTreeNodeDescriptor(&descriptor);
        if (descriptor.fLink == 0) {
            isLastNode = true;
        }

        list = _GetChildrenDirs(parentFolderId, rawNode, catalogBTHeader, descriptor, &list, copyInfo, fs);

        if (nodeBlockNumber == fs.volumeHeader.catalogFile.extents[extentNum].startBlock +
                               fs.volumeHeader.catalogFile.extents[extentNum].blockCount - 1) {
            extentNum += 1;
            nodeBlockNumber = fs.volumeHeader.catalogFile.extents[extentNum].startBlock;
        } else {
            nodeBlockNumber += descriptor.fLink - currentBlockNum;
            if (nodeBlockNumber >= fs.volumeHeader.catalogFile.extents[extentNum].startBlock +
                                   fs.volumeHeader.catalogFile.extents[extentNum].blockCount) {
                extentNum += 1;
                nodeBlockNumber = fs.volumeHeader.catalogFile.extents[extentNum].startBlock;
            }
        }

        currentBlockNum = descriptor.fLink;
    }

    free(rawNode);
    return list;
}