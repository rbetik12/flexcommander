#include <stdio.h>
#include "HFSPlusBTree.h"

void PrintBTreeHeader(BTHeaderRec header) {
    printf("Btree depth: %d\n", header.treeDepth);
    printf("Btree root node: %d\n", header.rootNode);
    printf("Btree leaf nodes: %d\n", header.leafRecords);
    printf("Btree first leaf: %d\n", header.firstLeafNode);
    printf("Btree last leaf: %d\n", header.lastLeafNode);
    printf("Btree node size: %d\n", header.nodeSize);
    printf("Btree max key length: %d\n", header.maxKeyLength);
    printf("Btree total nodes: %d\n", header.totalNodes);
    printf("Btree free nodes: %d\n", header.freeNodes);
    printf("Btree clump size: %d\n", header.clumpSize);
    printf("Btree type: %d\n", header.btreeType);
    printf("Btree key compare type: 0x%x\n", header.keyCompareType);
}

void PrintBTreeNodeDescriptor(BTNodeDescriptor descriptor) {
    printf("Next node num: %d\n", descriptor.fLink);
    printf("Previous node num: %d\n", descriptor.bLink);
    switch (descriptor.kind) {
        case LeafNode:
            printf("Node kind: leaf\n");
            break;
        case IndexNode:
            printf("Node kind: index\n");
            break;
        case HeaderNode:
            printf("Node kind: header\n");
            break;
        case MapNode:
            printf("Node kind: map\n");
            break;
        default:
            printf("Node kind: unknown\n");
            break;
    }
    printf("Node level: %d\n", descriptor.height);
    printf("Number of records: %d\n", descriptor.numRecords);
}