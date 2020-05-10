#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

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

    int linenum = 0; // 현재 스레드에서 읽은 파일

    char* buf = NULL;
    ssize_t read = 0; // getdelim() 에 의해 읽힌 데이터 크기
    size_t length = 0; // getdelim() 버퍼 크기

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
        so->full = FALSE;
        pthread_cond_signal(&so->cond_prod);
        pthread_mutex_unlock(&so->lock);
    }
    *pthread_ret = linenum;
    pthread_exit(pthread_ret);
}

void print_usage()
{
    printf("word_count - DKU2020OS HW2\n\n");
    printf("usage : word_count read_file_path consumer_count producer_count\n");
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

        so_t *share = malloc(sizeof(so_t));
        memset(share, 0, sizeof(so_t)); // 객체 메모리 초기화

        share->rfile = rfile;
        share->linenum = 0;
        share->full = FALSE;
        share->eof = FALSE;
        pthread_mutex_init(&share->lock, NULL);
        pthread_cond_init(&share->cond_prod, NULL);
        pthread_cond_init(&share->cond_cons, NULL);
        
        pthread_t prod[100];
        pthread_t cons[100];

        for (int i = 0 ; i < amount_prods ; i++)
                pthread_create(&prod[i], NULL, procedure_producer, share);
        for (int i = 0 ; i < amount_conds ; i++)
                pthread_create(&cons[i], NULL, procedure_consumer, share);

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

        free(share);
        fclose(rfile);
        return 0;
}

