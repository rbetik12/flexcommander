#include <stdio.h>
#include <byteswap.h>
#include <netinet/in.h>
#include "HFSPlusBTree.h"

void PrintBTreeHeader(BTHeaderRec header) {
    printf("Btree depth: %d\n", bswap_16(header.treeDepth));
    printf("Btree root node: %d\n", htonl(header.rootNode));
    printf("Btree leaf nodes: %d\n", htonl(header.leafRecords));
    printf("Btree first leaf: %d\n", htonl(header.firstLeafNode));
    printf("Btree last leaf: %d\n", htonl(header.lastLeafNode));
    printf("Btree node size: %d\n", bswap_16(header.nodeSize));
    printf("Btree max key length: %d\n", bswap_16(header.maxKeyLength));
    printf("Btree total nodes: %d\n", htonl(header.totalNodes));
    printf("Btree free nodes: %d\n", htonl(header.freeNodes));
    printf("Btree clump size: %d\n", htonl(header.clumpSize));
    printf("Btree type: %d\n", header.btreeType);
    printf("Btree key compare type: 0x%x\n", header.keyCompareType);
}

void PrintBTreeNodeDescriptor(BTNodeDescriptor descriptor) {
    printf("Next node num: %d\n", bswap_32(descriptor.fLink));
    printf("Previous node num: %d\n", bswap_32(descriptor.bLink));
    printf("Node kind: %d\n", descriptor.kind);
    printf("Node level: %d\n", descriptor.height);
    printf("Number of records: %d\n", bswap_16(descriptor.numRecords));
}