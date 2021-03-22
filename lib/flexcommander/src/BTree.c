#include "HFSPlusBTree.h"
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <List.h>
#include <HFSCatalog.h>
#include "utils/Endians.h"
#include "copy/Copy.h"
#include "utils/BTreeUtils.h"

uint32_t ParseLeafNode(char *rawNode, const char *folderName, uint32_t folderParentId, BTHeaderRec btreeHeader,
                       BTNodeDescriptor descriptor) {
    uint16_t recordAddress[descriptor.numRecords];
    FillRecordAddress(btreeHeader, descriptor, rawNode, recordAddress);

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

void ParseLeafNodeContent(char *rawNode, uint32_t parentID, BTHeaderRec btreeHeader, BTNodeDescriptor descriptor) {
    uint16_t recordAddress[descriptor.numRecords];
    FillRecordAddress(btreeHeader, descriptor, rawNode, recordAddress);

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
                printf("\t");
                PrintPermissions(catalogFolder.permissions, FolderRecord);
                break;
            case FileRecord:
                catalogFile = CAST_PTR_TO_TYPE(HFSPlusCatalogFile,
                                               (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
                ConvertCatalogFile(&catalogFile);
                printf("\t");
                PrintPermissions(catalogFolder.permissions, FileRecord);
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
    char *rawNode = calloc(sizeof(char), fs.blockSize);
    uint64_t nodeBlockNumber = catalogBTHeader.firstLeafNode + fs.catalogFileBlock;
    BTNodeDescriptor descriptor;
    bool isLastNode = false;
    uint64_t extentNum = 0;
    uint64_t currentBlockNum = catalogBTHeader.firstLeafNode;

    while (!isLastNode) {
        ReadNodeDescriptor(fs, nodeBlockNumber, &descriptor, rawNode);
        if (descriptor.fLink == 0) {
            isLastNode = true;
        }

        ParseLeafNodeContent(rawNode, parentID, catalogBTHeader, descriptor);

        GetNextBlockNum(&nodeBlockNumber, &extentNum, &currentBlockNum, descriptor, fs);
    }

    free(rawNode);
}

uint32_t FindIdOfFolder(const char *folderName, uint32_t folderParentId, BTHeaderRec catalogBTHeader, FlexCommanderFS fs) {
    char *rawNode = calloc(sizeof(char), fs.blockSize);
    uint64_t nodeBlockNumber = catalogBTHeader.firstLeafNode + fs.catalogFileBlock;
    BTNodeDescriptor descriptor;
    bool isLastNode = false;
    uint32_t id;
    uint64_t extentNum = 0;
    uint64_t currentBlockNum = catalogBTHeader.firstLeafNode;

    while (!isLastNode) {
        ReadNodeDescriptor(fs, nodeBlockNumber, &descriptor, rawNode);
        if (descriptor.fLink == 0) {
            isLastNode = true;
        }

        id = ParseLeafNode(rawNode, folderName, folderParentId, catalogBTHeader, descriptor);
        if (id != 0) break;

        GetNextBlockNum(&nodeBlockNumber, &extentNum, &currentBlockNum, descriptor, fs);
    }

    free(rawNode);
    return id;
}

HFSPlusCatalogFile * GetFileRecordFromLeafNode(uint32_t fileId, BTNodeDescriptor descriptor, BTHeaderRec btreeHeader, char *rawNode) {
    uint16_t recordAddress[descriptor.numRecords];

    FillRecordAddress(btreeHeader, descriptor, rawNode, recordAddress);

    HFSPlusCatalogFile *catalogFile = malloc(sizeof(HFSPlusCatalogFile));
    for (int i = 0; i < descriptor.numRecords; i++) {
        HFSPlusCatalogKey key;
        key = CAST_PTR_TO_TYPE(HFSPlusCatalogKey, (rawNode + recordAddress[i]));
        ConvertCatalogKey(&key);

        uint16_t recordType = rawNode[recordAddress[i] + key.keyLength + sizeof(key.keyLength) + 1];
        if (recordType == FileRecord) {
            *catalogFile = CAST_PTR_TO_TYPE(HFSPlusCatalogFile,
                                            (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
            ConvertCatalogFile(catalogFile);
            if (catalogFile->fileID == fileId) {
                return catalogFile;
            } else {
                continue;
            }
        } else {
            continue;
        }
    }

    free(catalogFile);
    return NULL;
}

uint32_t ParseLeafNodeWithCondition(char *rawNode, const char *folderName, uint32_t folderParentId, BTHeaderRec btreeHeader,
                                    BTNodeDescriptor descriptor,
                                    HFSDataRecordType recordTypeToFind) {
    uint16_t recordAddress[descriptor.numRecords];

    FillRecordAddress(btreeHeader, descriptor, rawNode, recordAddress);

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
            catalogFile = CAST_PTR_TO_TYPE(HFSPlusCatalogFile,
                                           (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
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

uint32_t FindIdOfFile(const char *fileName, uint32_t folderParentId, BTHeaderRec catalogBTHeader, FlexCommanderFS fs) {
    char *rawNode = calloc(sizeof(char), fs.blockSize);
    uint64_t nodeBlockNumber = catalogBTHeader.firstLeafNode + fs.catalogFileBlock;
    BTNodeDescriptor descriptor;
    bool isLastNode = false;
    uint32_t id;
    uint64_t extentNum = 0;
    uint64_t currentBlockNum = catalogBTHeader.firstLeafNode;

    while (!isLastNode) {
        ReadNodeDescriptor(fs, nodeBlockNumber, &descriptor, rawNode);
        if (descriptor.fLink == 0) {
            isLastNode = true;
        }

        id = ParseLeafNodeWithCondition(rawNode, fileName, folderParentId, catalogBTHeader, descriptor, FileRecord);
        if (id == 0) {
            id = ParseLeafNodeWithCondition(rawNode, fileName, folderParentId, catalogBTHeader, descriptor,
                                            FolderRecord);
        }
        if (id != 0) break;

        GetNextBlockNum(&nodeBlockNumber, &extentNum, &currentBlockNum, descriptor, fs);
    }

    free(rawNode);
    return id;
}

HFSPlusCatalogFile *GetFileRecord(uint32_t fileId, BTHeaderRec catalogBTHeader, FlexCommanderFS fs) {
    char *rawNode = calloc(sizeof(char), fs.blockSize);
    uint64_t nodeBlockNumber = catalogBTHeader.firstLeafNode + fs.catalogFileBlock;
    BTNodeDescriptor descriptor;
    bool isLastNode = false;
    uint64_t extentNum = 0;
    uint64_t currentBlockNum = catalogBTHeader.firstLeafNode;
    HFSPlusCatalogFile *catalogFile = NULL;

    while (!isLastNode) {
        ReadNodeDescriptor(fs, nodeBlockNumber, &descriptor, rawNode);
        if (descriptor.fLink == 0) {
            isLastNode = true;
        }

        catalogFile = GetFileRecordFromLeafNode(fileId, descriptor, catalogBTHeader, rawNode);
        if (catalogFile != NULL) {
            return catalogFile;
        }

        GetNextBlockNum(&nodeBlockNumber, &extentNum, &currentBlockNum, descriptor, fs);
    }

    free(rawNode);
    return NULL;
}

PathListNode *GetChildrenDirFromNode(uint32_t parentFolderId, const char *rawNode, BTHeaderRec btreeHeader,
                                     BTNodeDescriptor descriptor, PathListNode **listHead, CopyInfo copyInfo,
                                     FlexCommanderFS fs) {
    uint16_t recordAddress[descriptor.numRecords];

    FillRecordAddress(btreeHeader, descriptor, rawNode, recordAddress);
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
            catalogFile = CAST_PTR_TO_TYPE(HFSPlusCatalogFile,
                                           (rawNode + recordAddress[i] + key.keyLength + sizeof(key.keyLength)));
            ConvertCatalogFile(&catalogFile);
            CopyFile(copyInfo.dest, HFSStringToBytes(key.nodeName), catalogFile, &fs);
        } else {
            continue;
        }
    }
    return *listHead;
}

PathListNode *GetChildrenDirectoriesList(uint32_t parentFolderId, BTHeaderRec catalogBTHeader, FlexCommanderFS fs,
                                         CopyInfo copyInfo) {
    char *rawNode = calloc(sizeof(char), fs.blockSize);
    uint64_t nodeBlockNumber = catalogBTHeader.firstLeafNode + fs.catalogFileBlock;
    BTNodeDescriptor descriptor;
    bool isLastNode = false;
    uint64_t extentNum = 0;
    uint64_t currentBlockNum = catalogBTHeader.firstLeafNode;

    PathListNode *list = NULL;
    while (!isLastNode) {
        ReadNodeDescriptor(fs, nodeBlockNumber, &descriptor, rawNode);
        if (descriptor.fLink == 0) {
            isLastNode = true;
        }

        list = GetChildrenDirFromNode(parentFolderId, rawNode, catalogBTHeader, descriptor, &list, copyInfo, fs);

        GetNextBlockNum(&nodeBlockNumber, &extentNum, &currentBlockNum, descriptor, fs);
    }

    free(rawNode);
    return list;
}