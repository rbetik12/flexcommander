#pragma once

long long FlexRead (void * buffer, size_t structSize, size_t amount, FILE * file);
int FlexFSeek(FILE *file, long int offset, int mode);