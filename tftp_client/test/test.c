//
// Created by yll20 on 2022/10/19.
//
#include "../library.h"
#include <time.h>
#include <stdlib.h>

int main(){
    set_remote_ip("127.0.0.1");
    get_file("test.jpg", "test.jpg", MODE_OCTET);
    char *remote_filename;
    // 随机生成文件名
    srand(time(NULL));
    remote_filename = malloc(10);
    for (int i = 0; i < 6; i++) {
        remote_filename[i] = rand() % 26 + 'a';
    }
    // 加上.txt
    remote_filename[6] = '.';
    remote_filename[7] = 't';
    remote_filename[8] = 'x';
    remote_filename[9] = 't';
    remote_filename[10] = '\0';
    put_file(remote_filename, "test.txt", MODE_OCTET);
    return 0;
}