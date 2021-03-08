#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include <stdlib.h>
#include "HFSCatalog.h"

void PrintCatalogKey(HFSPlusCatalogKey key) {
    setlocale(LC_CTYPE, "");
    printf("Key info:\n");
    printf("Key length: %d\n", key.keyLength);
    printf("Parent ID: %d\n", key.parentID);
    printf("Node name length: %d\n", key.nodeName.length);
    printf("Node name: ");
    for (int i = 0; i < key.nodeName.length; i++) {
        printf("%lc", key.nodeName.unicode[i]);
    }
    printf("\n");
}

void PrintCatalogFolder(HFSPlusCatalogFolder folder) {
    printf("Folder info:\n");
    printf("Valence: %d\n", folder.valence);
    printf("Folder ID: %d\n", folder.folderID);
}

void PrintCatalogFile(HFSPlusCatalogFile file) {
    printf("File info:\n");
    printf("File ID: %d\n", file.fileID);
}

void PrintCatalogThread(HFSPlusCatalogThread thread) {
    printf("Catalog thread info:\n");
    printf("Thread type: ");
    switch (thread.recordType) {
        case FolderThreadRecord:
            printf("folder\n");
            break;
        case FileThreadRecord:
            printf("file\n");
            break;
        default:
            printf("unknown\n");
            break;
    }
    printf("Parent ID: %d\n", thread.parentID);
    printf("Thread name length: %d\n", thread.nodeName.length);
    printf("Node name: ");
    for (int i = 0; i < thread.nodeName.length; i++) {
        printf("%lc", thread.nodeName.unicode[i]);
    }
    printf("\n");
}