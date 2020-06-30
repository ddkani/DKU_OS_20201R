#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "kernel.h"
#include "fs.h"

/*
프로그램이 실행되면, 파일 시스템을 로드합니다.

*/

int main(int argc, char** argv)
{
    init_system("./disk.img");

    KFILE* f =  k_fopen("/file_14", "");
    char* buf = (char*)malloc(100);

    size_t size = k_fread(buf, 100, f);
    printf("%lu %s\n", size, buf);
}