#pragma once
#include "HFSPlus.h"
#include "Flexcommander.h"

enum {
    kBTLeafNode       = -1,
    kBTIndexNode      =  0,
    kBTHeaderNode     =  1,
    kBTMapNode        =  2
};

enum BTreeTypes{
    kHFSBTreeType           =   0,      // control file
    kUserBTreeType          = 128,      // user btree type starts from 128
    kReservedBTreeType      = 255
};

enum {
    kBTBadCloseMask           = 0x00000001,
    kBTBigKeysMask            = 0x00000002,
    kBTVariableIndexKeysMask  = 0x00000004
};

struct __attribute__((__packed__)) BTNodeDescriptor {
    UInt32    fLink;
    UInt32    bLink;
    SInt8     kind;
    UInt8     height;
    UInt16    numRecords;
    UInt16    reserved;
};
typedef struct BTNodeDescriptor BTNodeDescriptor;

struct __attribute__((__packed__)) BTHeaderRec {
    UInt16    treeDepth;
    UInt32    rootNode;
    UInt32    leafRecords;
    UInt32    firstLeafNode;
    UInt32    lastLeafNode;
    UInt16    nodeSize;
    UInt16    maxKeyLength;
    UInt32    totalNodes;
    UInt32    freeNodes;
    UInt16    reserved1;
    UInt32    clumpSize;      // misaligned
    UInt8     btreeType;
    UInt8     keyCompareType;
    UInt32    attributes;     // long aligned again
    UInt32    reserved3[16];
};
typedef struct BTHeaderRec BTHeaderRec;

void PrintBTreeHeader(BTHeaderRec header);
void PrintBTreeNodeDescriptor(BTNodeDescriptor descriptor);
void ParseNode(uint64_t nodeBlock, BTHeaderRec btreeHeader, FlexCommanderFS fs);

