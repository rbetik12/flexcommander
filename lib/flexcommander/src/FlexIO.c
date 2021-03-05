#include <stddef.h>
#include <stdio.h>
#include "FlexIO.h"

long long FlexRead (void * buffer, size_t structSize, size_t amount, FILE * file) {
    if (fread(buffer, structSize, amount, file) != amount) {
        if (feof(file)) {
            fprintf(stderr, "Unexpected EOF!\n");
        } else {
            fprintf(stderr, "Can't read!\n");
        }
        return -1;
    }
    return amount;
}

int FlexFSeek(FILE *file, long int offset, int mode) {
    if (fseek(file, offset, mode)) {
        fprintf(stderr, "Can't set 1024 bytes offset!\n");
        return -1;
    }
    return 0;
}