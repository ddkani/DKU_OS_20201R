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

// HW1 사용 코드 재활용

// const char src가 아닌 char src 여도 문제없음. 내부에서 복사하여 사용함.
char** split_str(const char* src, char* delimeter, int* count)
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

		ret_ptr = strtok_r(buf, delimeter, &next_ptr);
		unsigned int idx = 0;

		while (ret_ptr)
		{
				unsigned int sz = strlen(ret_ptr);
				char* bf = (char*)malloc((sz + 1) * sizeof(char));
				strncpy(bf, ret_ptr, sz);
				bf[sz] = '\0';

				result[idx] = bf;

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