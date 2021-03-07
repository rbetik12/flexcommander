#pragma once

#include "HFSPlus.h"

struct __attribute__((__packed__)) HFSPlusCatalogKey {
    UInt16              keyLength;
    HFSCatalogNodeID    parentID;
    HFSUniStr255        nodeName;
};
typedef struct HFSPlusCatalogKey HFSPlusCatalogKey;

enum HFSDataRecordType {
    FolderRecord        = 0x0001,
    FileRecord          = 0x0002,
    FolderThreadRecord  = 0x0003,
    FileThreadRecord    = 0x0004
};

struct __attribute__((__packed__)) HFSPlusBSDInfo {
    UInt32  ownerID;
    UInt32  groupID;
    UInt8   adminFlags;
    UInt8   ownerFlags;
    UInt16  fileMode;
    union {
        UInt32  iNodeNum;
        UInt32  linkCount;
        UInt32  rawDevice;
    } special;
};
typedef struct HFSPlusBSDInfo HFSPlusBSDInfo;

struct __attribute__((__packed__)) Rect {
    SInt16              top;
    SInt16              left;
    SInt16              bottom;
    SInt16              right;
};
typedef struct Rect   Rect;

struct __attribute__((__packed__)) Point {
    SInt16              v;
    SInt16              h;
};
typedef struct Point  Point;

struct __attribute__((__packed__)) FolderInfo {
    Rect      windowBounds;       /* The position and dimension of the */
    /* folder's window */
    UInt16    finderFlags;
    Point     location;           /* Folder's location in the parent */
    /* folder. If set to {0, 0}, the Finder */
    /* will place the item automatically */
    UInt16    reservedField;
};
typedef struct FolderInfo  FolderInfo;

struct __attribute__((__packed__)) ExtendedFolderInfo {
    Point     scrollPosition;     /* Scroll position (for icon views) */
    SInt32    reserved1;
    UInt16    extendedFinderFlags;
    SInt16    reserved2;
    SInt32    putAwayFolderID;
};
typedef struct ExtendedFolderInfo   ExtendedFolderInfo;

struct __attribute__((__packed__)) HFSPlusCatalogFolder {
    SInt16              recordType;
    UInt16              flags;
    UInt32              valence;
    HFSCatalogNodeID    folderID;
    UInt32              createDate;
    UInt32              contentModDate;
    UInt32              attributeModDate;
    UInt32              accessDate;
    UInt32              backupDate;
    HFSPlusBSDInfo      permissions;
    FolderInfo          userInfo;
    ExtendedFolderInfo  finderInfo;
    UInt32              textEncoding;
    UInt32              reserved;
};
typedef struct HFSPlusCatalogFolder HFSPlusCatalogFolder;

void PrintCatalogKey(HFSPlusCatalogKey key);
void PrintCatalogFolder(HFSPlusCatalogFolder folder);