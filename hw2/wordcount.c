#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

/*
단국대학교 컴퓨터공학과 HW2 멀티스레드 
32187345 양해찬
*/

#define TRUE 1
#define FALSE 0

#define ASCII_SIZE	256
#define MAX_STRING_LENGTH 30

char *sep = "{}()[],;\" \n\t^"; // 문자열 나누기용 데이터

typedef struct sharedobject
{
    FILE *rfile;
    pthread_mutex_t lock;
    pthread_cond_t cond_prod; // procedure_producer condition
    pthread_cond_t cond_cons; // procedure_consumer condition

    char* buf; // 읽은 버퍼 데이터입니다.
    int linenum; // 읽어준 줄 번호입니다. 1부터 시작합니다.
    int full; // 버퍼 데이터 준비 여부
    int eof; // 데이터를 다 읽었는지 여부

    int* stat; // 문자열 통계 - 줄별 입력 문자열 길이
    int* stat2; // 문자열 통계 - ASCII 문자열 별 사용 빈도
} so_t;

void *procedure_producer(void *arg)
{
    so_t *so = arg;
    unsigned int self = (unsigned int)pthread_self();
    int *pthread_ret = malloc(sizeof(int)); // 스택 영역에 있다면 밀릴 수 있다.

    char* buf = NULL;
    ssize_t read = 0; // getdelim() 에 의해 읽힌 데이터 크기
    size_t length = 0; // getdelim() 버퍼 크기

    printf("prod_%u | init\n", self);
    while (TRUE)
    {
        pthread_mutex_lock(&so->lock);
        
        // 다른 스레드에서 파일을 전부 읽고, 버퍼에 남은 데이터가 없음.
        if (so->full == FALSE, so->eof == TRUE)
        {
            // 작업이 종료되었습니다.
            printf("prod_%u | 스레드 종료\n", self);
            pthread_mutex_unlock(&so->lock);
            break;
        }

        while (so->full)
        {
            // 파일에 끝에 다다랐지만, 버퍼가 차 있으므로 모든 스레드를 동작합니다.
            if (so->eof)
            {
                    pthread_cond_signal(&so->cond_cons);
                    pthread_cond_signal(&so->cond_prod);
                    break;
            }
            // 신호를 잡을 때까지 대기
            pthread_cond_wait(&so->cond_prod,&so->lock);
        }

        // 데이터를 읽고 저장한 후 consumer에 신호를 보내고, 뮤텍스를 해제합니다.
        read = getdelim(&buf, &length, '\n', so->rfile);
        if (read == -1)
        {
            // 파일의 끝을 만났습니다.
            so->eof = TRUE;
            so->buf = NULL;
            pthread_cond_signal(&so->cond_cons);
            pthread_mutex_unlock(&so->lock);
            break;
        }
        so->linenum++;
        so->buf = strndup(buf, read);
        so->full = TRUE;
        pthread_cond_signal(&so->cond_cons);
        pthread_mutex_unlock(&so->lock);
    }
}


void *procedure_consumer(void *arg)
{
    so_t *so = arg;
    unsigned int self = (unsigned int)pthread_self();
    int *pthread_ret = malloc(sizeof(int)); // 스택 영역에 있다면 밀릴 수 있다.

    int linenum = 0; // 현재 스레드에서 읽은 줄의 수
    int length = 0; // 현재 스레드에서 읽은 자료 크기

    char* buf = NULL; // producer에서 받은 데이터
    char* brka = NULL;
    char* substr = NULL;

    printf("cond_%u | init\n", self);
    while (TRUE)
    {
        pthread_mutex_lock(&so->lock);
        if (so->full == FALSE && so->eof)
        {
            printf("cond_%u | 스레드 종료\n", self);
            pthread_mutex_unlock(&so->lock);
            break;
        }
        
        while(so->full == FALSE)
        {
            if (so->eof)
            {
                // 파일의 끝에 다다랐음. 스레드를 종료합니다.
                pthread_cond_signal(&so->cond_prod);
                pthread_cond_signal(&so->cond_cons);
                break;
            }
            pthread_cond_wait(&so->cond_cons, &so->lock);
        }
        buf = so->buf;
        if (buf == NULL) 
        { 
            pthread_cond_signal(&so->cond_prod);
            pthread_mutex_unlock(&so->lock);
            break;
        }
        printf("cond_%u | 데이터 읽음 : %s\n", self, buf);
        linenum++;
        length += strlen(buf);


        char* cptr = buf;
        // 받은 데이터를 문자별로 통계를 냅니다.
        for (substr = strtok_r(cptr, sep, &brka); substr; substr = strtok_r(NULL, sep, &brka))
        {
            int _length = strlen(substr);
			printf("length (%s): %d\n", substr, (int)_length);

			cptr = cptr + _length + 1;
			if (_length >= 30) _length = 30;
			so->stat[_length-1]++; // 줄별 길이
            printf("%d\n", *cptr);{
			// if (*cptr == '\0') break; 

		    cptr = substr;
            for (int i = 0 ; i < _length ; i++)
                if (*cptr < 256 && *cptr > 1) {
                    so->stat2[*cptr]++; // 문자열 빈도
                }
                cptr++;
		    }
        }

        so->full = FALSE;
        free(buf); // 동적 할당 & 복사한 데이터를 받았으므로 해제합니다.
        pthread_cond_signal(&so->cond_prod);
        pthread_mutex_unlock(&so->lock);
    }
    *pthread_ret = linenum;
    pthread_exit(pthread_ret);
}

void print_usage()
{
    printf("wordcount - DKU2020OS HW2 | 31739630\n\n");
    printf("usage : wordcount read_file_path consumer_count producer_count\n");
}


int main(int argc, char* argv[])
{
        if (argc != 4)
        {
            print_usage(); exit(-1);
        }

        FILE *rfile;
        rfile = fopen((char*)argv[1], "rb");

        int amount_prods = atoi(argv[2]);
        int amount_conds = atoi(argv[3]);

        so_t *so = malloc(sizeof(so_t));
        memset(so, 0, sizeof(so_t)); // 객체 메모리 초기화

        so->rfile = rfile;
        so->linenum = 0;
        so->full = FALSE;
        so->eof = FALSE;
        so->stat = malloc(sizeof(int) * MAX_STRING_LENGTH); // stat
        so->stat2 = malloc(sizeof(int) * ASCII_SIZE); // stat2
        pthread_mutex_init(&so->lock, NULL);
        pthread_cond_init(&so->cond_prod, NULL);
        pthread_cond_init(&so->cond_cons, NULL);
        
        pthread_t prod[100];
        pthread_t cons[100];

        for (int i = 0 ; i < amount_prods ; i++)
                pthread_create(&prod[i], NULL, procedure_producer, so);
        for (int i = 0 ; i < amount_conds ; i++)
                pthread_create(&cons[i], NULL, procedure_consumer, so);

        int pthread_exitcode;
        int* pthread_return;

        for (int i = 0 ; i < amount_conds ; i++) {
                pthread_exitcode = pthread_join(cons[i], (void **) &pthread_return);
                // printf("main: consumer_%d joined with %d\n", i, *ret);

        }
        for (int i = 0 ; i < amount_prods ; i++) {
                pthread_exitcode = pthread_join(prod[i], (void **) &pthread_return);
                // printf("main: producer_%d joined with %d\n", i, *ret);
        }

        // 전체 통계를 출력합니다.

        int sum = 0;

        for (int i = 0 ; i < 30 ; i++) {
		    sum += so->stat[i];
	    }

        printf("  #ch  freq \n");
	    for (int i = 0 ; i < 30 ; i++) {
		    int j = 0;
		    int num_star = so->stat[i]*80/sum;
		    printf("[%3d]: %4d \t", i+1, so->stat[i]);

		    for (j = 0 ; j < num_star ; j++) printf("*");
		    printf("\n");
	    }

        printf("       A        B        C        D        E        F        G        H        I        J        K        L        M        N        O        P        Q        R        S        T        U        V        W        X        Y        Z\n");
        printf("%8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d\n",
			so->stat2['A']+so->stat2['a'], so->stat2['B']+so->stat2['b'],  so->stat2['C']+so->stat2['c'],  so->stat2['D']+so->stat2['d'],  so->stat2['E']+so->stat2['e'],
			so->stat2['F']+so->stat2['f'], so->stat2['G']+so->stat2['g'],  so->stat2['H']+so->stat2['h'],  so->stat2['I']+so->stat2['i'],  so->stat2['J']+so->stat2['j'],
			so->stat2['K']+so->stat2['k'], so->stat2['L']+so->stat2['l'],  so->stat2['M']+so->stat2['m'],  so->stat2['N']+so->stat2['n'],  so->stat2['O']+so->stat2['o'],
			so->stat2['P']+so->stat2['p'], so->stat2['Q']+so->stat2['q'],  so->stat2['R']+so->stat2['r'],  so->stat2['S']+so->stat2['s'],  so->stat2['T']+so->stat2['t'],
			so->stat2['U']+so->stat2['u'], so->stat2['V']+so->stat2['v'],  so->stat2['W']+so->stat2['w'],  so->stat2['X']+so->stat2['x'],  so->stat2['Y']+so->stat2['y'],
			so->stat2['Z']+so->stat2['z']);


        free(so->stat);
        free(so->stat2);
        free(so);
        fclose(rfile);
        return 0;
}

