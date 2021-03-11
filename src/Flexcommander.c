#include <Flexcommander.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv) {
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
    }
    else if (strcmp(argv[1], "-i") == 0) {
        FlexCommanderFS fs;
        if (FlexOpen(argv[2], &fs)) {
            fprintf(stderr, "Error!\n");
            exit(EXIT_FAILURE);
        }

        printf("Welcome to the club, buddy!\n");

        while (1) {
            char str[200] = {0};
            fputs(">", stdout);
            fflush(stdout);
            fgets(str, sizeof(str), stdin);
            if (strcmp("exit\n", str) == 0) {
                printf("Bye!\n");
                break;
            }
            if (str[0] == 'l' && str[1] == 's') {
                FlexListDirContent(str + 3, &fs);
            }
            else {
                printf("Unknown command!\n");
            }
        }
    }
    else {
        fputs("Unknown key!\n", stderr);
        exit(EXIT_FAILURE);
    }

    return 0;
}