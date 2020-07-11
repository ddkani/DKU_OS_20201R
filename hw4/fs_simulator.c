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
    printf("DKU2020OS - Simple file system implementation\n");
    printf("Haechan Yang - 32187345\n\n");
    init_system("./disk.img");

    char cmd[100];
    memset(cmd, 0, 100);

    print_root_fstree();
    
    while (1)
    {
        printf("> ");
        scanf("%s", cmd);
        if (strcmp("exit", cmd) == 0)
        {
            printf("bye\n");
            return 0;
        }

        KFILE* f =  k_fopen(cmd, "");
        char* buf = (char*)malloc(100);

        size_t size = k_fread(buf, 100, f);
        printf("read file %s) size: %lu content: %s\n\n", cmd, size, buf);

        memset(cmd, 0, 100);        
    }


}