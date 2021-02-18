#include <Flexcommander.h>

int main() {
    FlexCommanderFS fs;
    FlexGetDirElements("/mnt/temp", &fs);
    FlexCopyFile("/mnt/temp/kek.txt", "/home/vitaliy/kek.txt");
    return 0;
}