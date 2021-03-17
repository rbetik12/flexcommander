#include <Flexcommander.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../lib/flexcommander/src/copy/Copy.h"

#define CURRENT_DIR_STRING_LENGTH 200
#define COMMAND_MAX_LENGTH CURRENT_DIR_STRING_LENGTH

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
        FlexCommanderFS fs;
        if (FlexOpen(argv[2], &fs)) {
            fprintf(stderr, "Error!\n");
            exit(EXIT_FAILURE);
        }

        printf("Welcome to the club, buddy!\n");

        char *currentDir = calloc(CURRENT_DIR_STRING_LENGTH, sizeof(char));
        while (1) {
            char str[COMMAND_MAX_LENGTH] = {0};
            StripString(currentDir);
            fputs(currentDir, stdout);
            fputs(">", stdout);
            fflush(stdout);
            fgets(str, sizeof(str), stdin);
            if (strcmp("exit\n", str) == 0) {
                printf("Bye!\n");
                break;
            }
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
            } else {
                printf("Unknown command!\n");
            }
        }
        free(currentDir);
    } else {
        fputs("Unknown key!\n", stderr);
        exit(EXIT_FAILURE);
    }

    return 0;
}