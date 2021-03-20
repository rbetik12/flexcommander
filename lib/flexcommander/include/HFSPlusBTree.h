#pragma once

#include <List.h>
#include <stdbool.h>
#include "HFSPlus.h"
#include "Flexcommander.h"
#include "../src/copy/Copy.h"

enum BTNodeType {
    LeafNode = -1,
    IndexNode = 0,
    HeaderNode = 1,
    MapNode = 2
};

enum BTreeTypes {
    kHFSBTreeType = 0,      // control file
    kUserBTreeType = 128,      // user btree type starts from 128
    kReservedBTreeType = 255
};

enum {
    kBTBadCloseMask = 0x00000001,
    kBTBigKeysMask = 0x00000002,
    kBTVariableIndexKeysMask = 0x00000004
};

struct __attribute__((__packed__)) BTNodeDescriptor {
    UInt32 fLink;
    UInt32 bLink;
    SInt8 kind;
    UInt8 height;
    UInt16 numRecords;
    UInt16 reserved;
};
typedef struct BTNodeDescriptor BTNodeDescriptor;

struct __attribute__((__packed__)) BTHeaderRec {
    UInt16 treeDepth;
    UInt32 rootNode;
    UInt32 leafRecords;
    UInt32 firstLeafNode;
    UInt32 lastLeafNode;
    UInt16 nodeSize;
    UInt16 maxKeyLength;
    UInt32 totalNodes;
    UInt32 freeNodes;
    UInt16 reserved1;
    UInt32 clumpSize;      // misaligned
    UInt8 btreeType;
    UInt8 keyCompareType;
    UInt32 attributes;     // long aligned again
    UInt32 reserved3[16];
};
typedef struct BTHeaderRec BTHeaderRec;

void PrintBTreeHeader(BTHeaderRec header);
void PrintBTreeNodeDescriptor(BTNodeDescriptor descriptor);
uint32_t ParseLeafNode(char* rawNode, const char* folderName, uint32_t folderParentId, BTHeaderRec btreeHeader, BTNodeDescriptor descriptor);
uint32_t FindIdOfFolder(const char* folderName, uint32_t folderParentId, BTHeaderRec catalogBTHeader, FlexCommanderFS fs);
uint32_t ParseLeafNodeWithCondition(char *rawNode, const char *folderName, uint32_t folderParentId, BTHeaderRec btreeHeader,
                                    BTNodeDescriptor descriptor, enum HFSDataRecordType _recordType);
uint32_t FindIdOfFile(const char *fileName, uint32_t folderParentId, BTHeaderRec catalogBTHeader, FlexCommanderFS fs);
void ListDirectoryContent(uint32_t parentID, BTHeaderRec catalogBTHeader, FlexCommanderFS fs);
HFSPlusCatalogFile* GetFileRecord(uint32_t fileId, BTHeaderRec catalogBTHeader, FlexCommanderFS fs);
PathListNode* GetChildrenDirectoriesList(uint32_t parentFolderId, BTHeaderRec catalogBTHeader, FlexCommanderFS fs, CopyInfo copyInfo);


