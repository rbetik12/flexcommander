#pragma once

#include <stdint.h>

#define HFS_SIGNATURE 0x2b48
#define HFS_START_OFFSET 1024

typedef uint16_t UInt16;
typedef uint32_t UInt32;
typedef uint64_t UInt64;
typedef uint32_t HFSCatalogNodeID;
typedef uint16_t UniChar;
typedef uint8_t  UInt8;
typedef char     SInt8;
typedef short    SInt16;
typedef int      SInt32;
typedef UInt32        FourCharCode;
typedef FourCharCode  OSType;

#define S_ISUID 0004000     /* set user id on execution */
#define S_ISGID 0002000     /* set group id on execution */
#define S_ISTXT 0001000     /* sticky bit */

#define S_IRWXU 0000700     /* RWX mask for owner */
#define S_IRUSR 0000400     /* R for owner */
#define S_IWUSR 0000200     /* W for owner */
#define S_IXUSR 0000100     /* X for owner */

#define S_IRWXG 0000070     /* RWX mask for group */
#define S_IRGRP 0000040     /* R for group */
#define S_IWGRP 0000020     /* W for group */
#define S_IXGRP 0000010     /* X for group */

#define S_IRWXO 0000007     /* RWX mask for other */
#define S_IROTH 0000004     /* R for other */
#define S_IWOTH 0000002     /* W for other */
#define S_IXOTH 0000001     /* X for other */

#define S_IFMT   0170000    /* type of file mask */
#define S_IFIFO  0010000    /* named pipe (fifo) */
#define S_IFCHR  0020000    /* character special */
#define S_IFDIR  0040000    /* directory */
#define S_IFBLK  0060000    /* block special */
#define S_IFREG  0100000    /* regular */
#define S_IFLNK  0120000    /* symbolic link */
#define S_IFSOCK 0140000    /* socket */
#define S_IFWHT  0160000    /* whiteout */

struct HFSPlusExtentDescriptor {
    UInt32                  startBlock;
    UInt32                  blockCount;
};
typedef struct HFSPlusExtentDescriptor HFSPlusExtentDescriptor;

typedef HFSPlusExtentDescriptor HFSPlusExtentRecord;

struct HFSPlusForkData {
    UInt64                  logicalSize;
    UInt32                  clumpSize;
    UInt32                  totalBlocks;
    HFSPlusExtentRecord     extents[8];
};
typedef struct HFSPlusForkData HFSPlusForkData;

struct HFSPlusVolumeHeader {
    UInt16              signature;
    UInt16              version;
    UInt32              attributes;
    UInt32              lastMountedVersion;
    UInt32              journalInfoBlock;

    UInt32              createDate;
    UInt32              modifyDate;
    UInt32              backupDate;
    UInt32              checkedDate;

    UInt32              fileCount;
    UInt32              folderCount;

    UInt32              blockSize;
    UInt32              totalBlocks;
    UInt32              freeBlocks;

    UInt32              nextAllocation;
    UInt32              rsrcClumpSize;
    UInt32              dataClumpSize;
    HFSCatalogNodeID    nextCatalogID;

    UInt32              writeCount;
    UInt64              encodingsBitmap;

    UInt32              finderInfo[8];

    HFSPlusForkData     allocationFile;
    HFSPlusForkData     extentsFile;
    HFSPlusForkData     catalogFile;
    HFSPlusForkData     attributesFile;
    HFSPlusForkData     startupFile;
};
typedef struct HFSPlusVolumeHeader HFSPlusVolumeHeader;

struct HFSUniStr255 {
    UInt16  length;
    UniChar unicode[255];
};
typedef struct HFSUniStr255 HFSUniStr255;
typedef const  HFSUniStr255 *ConstHFSUniStr255Param;
typedef UInt32 HFSCatalogNodeID;