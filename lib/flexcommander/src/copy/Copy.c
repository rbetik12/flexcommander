#include <zconf.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include "Copy.h"

void MakePath(char* dir) {
    char cwd[512];
    if (dir[0] == '.') {
        getcwd(cwd, sizeof(cwd));
    }

    char tmp[1024];
    char* p = NULL;
    size_t len;

    if (dir[0] == '.') {
        snprintf(tmp, sizeof(tmp), "%s%s", cwd, dir + 1);
    }
    else {
        snprintf(tmp, sizeof(tmp), "%s", dir);
    }
    printf("%s\n", tmp);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    mkdir(tmp, S_IRWXU);
}