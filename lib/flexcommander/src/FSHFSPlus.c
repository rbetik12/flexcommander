#ifdef HFSPlus

#include <Flexcommander.h>
#include <HFSPlus.h>
#include <errno.h>
#include <sys/mount.h>
#include <string.h>
#include <fcntl.h>
#include <linux/loop.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <byteswap.h>
#include <HFSPlusBTree.h>
#include <FlexIO.h>

#define DATE_BUFFER_SIZE 64
#define RW_BLOCK_SIZE 4096
#define BTREE_NODE_SIZE 4096

int Verify(FlexCommanderFS* fs);
int ReadBtreeHeader(uint64_t pos, FlexCommanderFS* fs);

int FlexOpen(const char * path, FlexCommanderFS* fs) {
    FILE* dev = fopen(path, "rb");

    if (!dev) {
        perror("Can't open dev!\n");
        return -1;
    }

    fs->file = dev;
    if (Verify(fs)) {
        return -1;
    }

    return 0;
}

int Verify(FlexCommanderFS* fs) {

    HFSPlusVolumeHeader header;

    if (fseek(fs->file, HFS_START_OFFSET, SEEK_SET)) {
        fprintf(stderr, "Can't set 1024 bytes offset!\n");
        return -1;
    }

    if (fread(&header, sizeof(HFSPlusVolumeHeader), 1, fs->file) != 1) {
        if (feof(fs->file)) {
            fprintf(stderr, "Unexpected EOF!\n");
        } else {
            fprintf(stderr, "Can't read HFS volume header!\n");
        }
        return -1;
    }

    if (header.signature == HFS_SIGNATURE) {
        fs->blockSize = htonl(header.blockSize);
        printf("Total files: %d\n", htonl(header.fileCount));
        printf("Total folders: %d\n", htonl(header.folderCount));
        printf("Block size: %d\n", fs->blockSize);
        printf("Total blocks: %d\n", htonl(header.totalBlocks));
        printf("Next free block: %d\n", htonl(header.nextAllocation));

        printf("\nCatalog file:\n");
        printf("Catalog file logical size: %lu\n", bswap_64(header.catalogFile.logicalSize));
        printf("Catalog file clump size: %u\n", htonl(header.catalogFile.clumpSize));
        printf("Catalog file total blocks: %u\n", htonl(header.catalogFile.totalBlocks));
        uint64_t catalogFirstBlockNum = htonl(header.catalogFile.extents[0].startBlock);
        uint64_t catalogFileLocation =  catalogFirstBlockNum * fs->blockSize;
        printf("Catalog file location: %lx\n", catalogFileLocation);
        printf("Catalog file first block: %lu\n", catalogFirstBlockNum);
        ReadBtreeHeader(catalogFileLocation, fs);
        return 0;
    } else {
        fprintf(stderr, "Provided file is not a HFS volume!\n");
        return -1;
    }
}

int ReadBtreeHeader(uint64_t pos, FlexCommanderFS* fs) {
    BTNodeDescriptor btreeHeaderDescr;
    BTHeaderRec btreeHeader;

    FlexFSeek(fs->file, pos, SEEK_SET);
    FlexRead(&btreeHeaderDescr, sizeof(BTNodeDescriptor), 1, fs->file);
    FlexRead(&btreeHeader, sizeof(BTHeaderRec), 1, fs->file);

    printf("\nCatalog file btree header:\n");
    PrintBTreeHeader(btreeHeader);

    return 0;
}
#endif

