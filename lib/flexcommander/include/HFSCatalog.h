#pragma once

#include "HFSPlus.h"

struct HFSPlusCatalogKey {
    UInt16              keyLength;
    HFSCatalogNodeID    parentID;
    HFSUniStr255        nodeName;
};
typedef struct HFSPlusCatalogKey HFSPlusCatalogKey;

enum HFSDataRecordType {
    kHFSPlusFolderRecord        = 0x0001,
    kHFSPlusFileRecord          = 0x0002,
    kHFSPlusFolderThreadRecord  = 0x0003,
    kHFSPlusFileThreadRecord    = 0x0004
};

struct HFSPlusBSDInfo {
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

struct Rect {
    SInt16              top;
    SInt16              left;
    SInt16              bottom;
    SInt16              right;
};
typedef struct Rect   Rect;

struct Point {
    SInt16              v;
    SInt16              h;
};
typedef struct Point  Point;

struct FolderInfo {
    Rect      windowBounds;       /* The position and dimension of the */
    /* folder's window */
    UInt16    finderFlags;
    Point     location;           /* Folder's location in the parent */
    /* folder. If set to {0, 0}, the Finder */
    /* will place the item automatically */
    UInt16    reservedField;
};
typedef struct FolderInfo  FolderInfo;

struct ExtendedFolderInfo {
    Point     scrollPosition;     /* Scroll position (for icon views) */
    SInt32    reserved1;
    UInt16    extendedFinderFlags;
    SInt16    reserved2;
    SInt32    putAwayFolderID;
};
typedef struct ExtendedFolderInfo   ExtendedFolderInfo;

struct HFSPlusCatalogFolder {
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