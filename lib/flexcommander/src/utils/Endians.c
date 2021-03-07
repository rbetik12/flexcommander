#include "Endians.h"
#include <byteswap.h>
#include <netinet/in.h>

void ConvertBTreeHeader(BTHeaderRec* header) {
    header->treeDepth = bswap_16(header->treeDepth);
    header->rootNode = htonl(header->rootNode);
    header->leafRecords = htonl(header->leafRecords);
    header->firstLeafNode = htonl(header->firstLeafNode);
    header->lastLeafNode = htonl(header->lastLeafNode);
    header->nodeSize = bswap_16(header->nodeSize);
    header->maxKeyLength = bswap_16(header->maxKeyLength);
    header->totalNodes = htonl(header->totalNodes);
    header->freeNodes = htonl(header->freeNodes);
    header->clumpSize = htonl(header->clumpSize);
}

void ConvertBTreeNodeDescriptor(BTNodeDescriptor* descr) {
    descr->fLink = bswap_32(descr->fLink);
    descr->bLink = bswap_32(descr->bLink);
    descr->numRecords = bswap_16(descr->numRecords);
}