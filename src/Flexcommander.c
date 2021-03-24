#include <Flexcommander.h>
#include <string.h>
#include <stdlib.h>

#define CURRENT_DIR_STRING_LENGTH 200
#define COMMAND_MAX_LENGTH CURRENT_DIR_STRING_LENGTH

#define DEBUG

void StripString(char *string);

void ParseRelativePath(const char* path, const char* currentDir) {
    StripString(path);
    if (path[0] == '.') {
        const size_t currentDirLength = strlen(currentDir);
        const size_t pathLength = strlen(path);
        char* currentPathBuffer = calloc(COMMAND_MAX_LENGTH, 1);

        memcpy(currentPathBuffer, path + 1, pathLength + 1);
        memset(path, 0, COMMAND_MAX_LENGTH);
        memcpy(path, currentDir, currentDirLength);
        memcpy(path + currentDirLength, currentPathBuffer, pathLength);
        free(currentPathBuffer);
    }
}

void StripString(char *string) {
    uint64_t index = 0;
    while (string[index] != '\0') {
        if (string[index] == '\n') {
            string[index] = '\0';
        }
        index++;
    }
}

int main(int argc, char **argv) {
#ifndef DEBUG
    if (argc < 2) {
        fputs("flexcom [-l -i] <dev path>\n", stderr);
        fputs("-l list all the available devices\n", stderr);
        fputs("-i interactive mode\n", stderr);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "-l") == 0) {
        FlexCommanderProbeInfo info;
        Init(&info);
        ProbeDevices(&info);
        IterateDevices(&info);
    } else if (strcmp(argv[1], "-i") == 0) {
#endif
        FlexCommanderFS fs;
#ifdef DEBUG
        if (FlexOpen("hfs.img", &fs)) {
            fprintf(stderr, "Error!\n");
            exit(EXIT_FAILURE);
        }
#else
        if (FlexOpen(argv[2], &fs)) {
            fprintf(stderr, "Error!\n");
            exit(EXIT_FAILURE);
        }
#endif

        printf("Welcome to the club, buddy!\n");

        char *currentDir = calloc(CURRENT_DIR_STRING_LENGTH, sizeof(char));
        char* str = calloc(COMMAND_MAX_LENGTH, 1);

        currentDir[0] = '/';

        while (1) {
            StripString(currentDir);
            fputs(currentDir, stdout);
            fputs(">", stdout);
            fflush(stdout);
            fgets(str, COMMAND_MAX_LENGTH, stdin);
            if (strcmp("exit\n", str) == 0) {
                printf("Bye!\n");
                break;
            }

            //I assume that all commands will be 2 characters length
            ParseRelativePath(str + 3, currentDir);

            if (str[0] == 'l' && str[1] == 's') {
                if (str[3] == '.') {
                    FlexListDirContent(currentDir, &fs);
                } else {
                    FlexListDirContent(str + 3, &fs);
                }
            } else if (str[0] == 'c' && str[1] == 'd') {
                if (FlexSetCurrentDir(str + 3, &fs)) {
                    fprintf(stderr, "Path doesn't exist!\n");
                } else {
                    memset(currentDir, 0, CURRENT_DIR_STRING_LENGTH);
                    memcpy(currentDir, str + 3, CURRENT_DIR_STRING_LENGTH - 3);
                }
            } else if (str[0] == 'c' && str[1] == 'p') {
                FlexCopy(str + 3, currentDir, &fs);
            }
            else {
                printf("Unknown command!\n");
            }
            memset(str, 0, COMMAND_MAX_LENGTH);
        }
        free(currentDir);
        free(str);
#ifndef DEBUG
    } else {
        fputs("Unknown key!\n", stderr);
        exit(EXIT_FAILURE);
    }
#endif

    return 0;
}