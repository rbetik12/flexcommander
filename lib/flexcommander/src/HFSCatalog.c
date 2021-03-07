#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include <stdlib.h>
#include "HFSCatalog.h"

void PrintCatalogKey(HFSPlusCatalogKey key) {
    setlocale(LC_CTYPE, "");
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