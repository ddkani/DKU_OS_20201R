#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* malloc, strncpy, strcpy */
#include <sys/stat.h> /* stat */

#include "functions.h"
#include "global_config.h"

void trim_str(char* src)
{
		// not implemented

		// src 데이터를 변환합니다.

		unsigned int sz_src = strlen(src);

		char* buf = (char*)malloc((sz_src + 1) * sizeof(char));
		strncpy(buf, src, sz_src);
		buf[sz_src] = '\0';

		memset(src, '\0', sz_src);

		/*
		1. start position 찾기 (처음부터 들어간다.)
		2. end position 찾기 (거꾸로 들어간다.)

		memory copy 한다. startpos ~ endpos => src

		*/

		free(buf);
}

int is_executable(char* path)
{
	 // 해당 경로의 파일이 존재하고, 실행 가능한지 확인합니다.
	 struct stat fstat_buf;

	 int result = stat(path, &fstat_buf);

	 #ifdef DEBUG
	 printf("stat %s : ret=%d\n", path, result);
	 printf("st_mode : %hu\n", fstat_buf.st_mode);
	 #endif

	 if (result == -1) return 0; // not exist

	 // 그룹이나 사용자가 해당 파일을 실행할 수 있는지 확인합니다.
	 return (fstat_buf.st_mode & S_IXUSR) || (fstat_buf.st_mode & S_IXGRP);

}

char** split_str(char* src, char* delimeter, int* count)
{
		unsigned int sz_src = strlen(src);

		/*
		strtok_r 함수는 src 데이터에 변형을 가합니다.
	  해당 데이터가 손상되기를 원치 않는다면 복사하여야 합니다.
		*/

		char* buf = (char*)malloc((sz_src + 1) * sizeof(char));
		strncpy(buf, src, sz_src);
		buf[sz_src] = '\0';

		/* TODO dynamic alloc : 128 => realloc or ... */
		char** result = (char**)malloc(SPLIT_LENGTH * sizeof(char**));
		memset(result, 0, SPLIT_LENGTH * sizeof(char**));

		char* ret_ptr;
		char* next_ptr;

		#ifdef DEBUG
		printf("result: %p\n", result);
		printf("buf: %p %s\n", buf, buf);
		printf("dlim: %p %s\n", delimeter, delimeter);
		#endif
		/*
			strtok_r
			src (char*): source
			delm (char*): delimeter
			save_ptr (char**): [ret] next pointer // next search result

			return: (char*) first search result
		*/

		/*
		???? 실행 중 오류 발생 : bus error (in OSX) | IDA64로 디버깅

		Stack trace:
			strtok_r
			_split_str
			_main
			...

	  원인 추적 중 오류가 발생한 asm은 다음과 같다.
		mov		byte ptr [r9], 0

		=> 원인 파악
		r9 레지스터의 값은 src로 제공한 값의 메모리 주소.
	  OSX __cstring 에 상수 문자열이 저장되는데, write 권한이 없어 발생한 오류로 보임.
		src 가 상수 문자열이 아닌 stack / heap 의 메모리 주소일 경우 에러가 발생하지 않는다.

		=> 참고 정보 : https://linux.die.net/man/3/strtok_r : Bugs 문단
		These functions cannot be used on constant strings.
		These functions modify their first argument.

	  따라서 문자열 상수가 저장된, 쓰기 권한이 존재하지 않는 값을 파싱할 수 없다.

		*/

		ret_ptr = strtok_r(buf, delimeter, &next_ptr);
		unsigned int idx = 0;

		while (ret_ptr)
		{
				unsigned int sz = strlen(ret_ptr);
				char* bf = (char*)malloc((sz + 1) * sizeof(char));
				strncpy(bf, ret_ptr, sz);
				bf[sz] = '\0';

				result[idx] = bf;

				#ifdef DEBUG
				/* save_ptr 의 데이터가 메모리 주소 확인을 위함. */
				printf("%p : %s\n", bf, bf);
				#endif

				ret_ptr = strtok_r(NULL, delimeter, &next_ptr);
				idx++;
		}

		if (count != NULL)
		{
			 *count = idx;
		}

		free(buf); // 결과데이터는 복사 되었으므로 기존 버퍼는 해제
		return result;
}
