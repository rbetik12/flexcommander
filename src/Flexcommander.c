#include <Flexcommander.h>

int main() {
    FlexCommanderFS fs;
    FlexOpen("/home/vitaliy/itmo/spo/flexcommander/res/fs/hfs.img", &fs);
    return 0;
}