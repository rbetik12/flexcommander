#include <Flexcommander.h>

int main() {
    FlexCommanderFS fs;
    FlexOpen("/home/vitaliy/itmo/spo/lab1/res/fs/hfs.img", &fs);
    return 0;
}