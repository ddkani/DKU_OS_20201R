#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

#define TRUE 1
#define FALSE 0

#define MAX_THREADS 100

/* 멀티스레딩 이론
스레드별로 동일한 객체를 접근할 때 CPU의 원리 (메모리가 아닌 레지스터에서 연산)
으로 인해 데이터의 atomic이 보장되지 않습니다.
이에 특정 데이터의 읽기,쓰기를 하나의 스레드에서만 하도록 mutex를 사용합니다.

pthread_cond_wait - cond 객체가 signal을 받을 때까지 실행을 일시정지합니다.
pthread_cond_signal - cond 객체에 signal을 보냅니다.

*/


// TODO - 스레드 동작 확인 / 값 확인 / 알파벳별 갯수 확인

typedef struct sharedobject 
{
    FILE *rfile; // 파일 읽기전용 객체
    int linenum;
    char* line;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int full; // 버퍼에 데이터가 있는가? (1 / 0 구분)
    int finished; // 파일을 전부 읽었는가?
    // TODO: 문자열별 통계 데이터 추가
} so_t;           


// 파일을 읽고, 버퍼에 저장합니다.
void *procedure_producer(void *arg) // void: 자료형을 정하지 않음 (알수없음)
{
    so_t *so = arg; // 타입 캐스팅
    int *ret = malloc(sizeof(int)); // pthread 반환값
    
    FILE *rfile = so->rfile;
    int line_count = 0;
    
    char *buf = NULL; // rfile에서 읽은 데이터의 공간을 가리키게 됨.
    size_t len = 0; // getdelim 의 버퍼크기
    ssize_t read = 0; // getdelim() 호출시마다 읽은 값
    
    unsigned int self = pthread_self(); // threadid

    printf("prod_%u :initialized\n", self);
    while (TRUE)
    {
        // 뮤텍스를 잠급니다. - consumer 에서 다 읽을때까지 기다립니다.
	    printf("prod_%u: try to acquire mutex\n", self);
        pthread_mutex_lock(&so->lock);       
	    printf("prod_%u: mutex acquired\n", self);

        // delimeter 전까지 rfile에서 읽는다. 
        read = getdelim(&buf, &len, '\n', rfile);
        printf("prod_%u: getdelim() file readed\n", self);
        if (read == -1)
        {
            // 파일의 끝에 다다랐습니다. 더 읽을 수 없습니다.
            so->full = TRUE;
            so->finished = TRUE;
            // 다 읽었음을 알리고 러프를 종료합니다.
            printf("prod_%u: file end: signal / unlock mutex\n", self);
            pthread_cond_signal(&so->cond);
            pthread_mutex_unlock(&so->lock);
            break;
        }
        // 버퍼를 다 읽을때까지 대기합니다.
        // 아직 버퍼를 consumer에서 처리하지 않았음.
        while (so->full == TRUE) 
        {
            printf("prod_%u: wait condition - FULL == FALSE -> TRUE\n", self);
            pthread_cond_wait(&so->cond, &so->lock);
        }
        so->full = TRUE;
        so->linenum = line_count;
        so->line = strndup(buf, read); // 읽은 데이터를 공유합니다.
        printf("prod_%u: read line %s\n", self, buf);
        printf("prod_%u: file readed / signal, unlock\n", self);
        line_count++;
        pthread_cond_signal(&so->cond);
        pthread_mutex_unlock(&so->lock);
    }
    free(buf); // getdelim 에서 동적 할당하였으므로 해제.
    *ret = line_count; // 읽은 줄 수를 저장하고 스레드를 종료합니다.
    pthread_exit(ret);
}


// 버퍼에서 자료를 가져옵니다.
void *procedure_consumer(void *arg)
{
    so_t *so = arg; // 타입 캐스팅
    int *ret = malloc(sizeof(int)); // pthread 반환값
    
    FILE *rfile = so->rfile;
    int line_count = 0; // 읽은 줄 갯수
    int char_count = 0; // 읽은 문자열 갯수

    char *buf = NULL; // rfile에서 읽은 데이터의 공간을 가리키게 됨.
    size_t len = 0; // getdelim 의 버퍼크기
    ssize_t read = 0; // getdelim() 호출시마다 읽은 값
    unsigned int self = (unsigned int) pthread_self();
    printf("cond_%u :initialized\n", self);
    while (TRUE)
    {
	    
	    printf("cond_%u: try to acquire mutex\n", self);
        // 뮤텍스를 잠급니다.
        pthread_mutex_lock(&so->lock); 
	    printf("cond_%u: mutex acquired\n", self);
        // 버퍼를 아직 가져오지 못했다면, 버퍼가 찰 때까지 대기합니다.

        while(so->full == FALSE)
        {
            printf("cond_%u: wait condition - FULL == TRUE -> FALSE\n", self);
            pthread_cond_wait(&so->cond, &so->lock);
        }

        buf = so->line; // 읽은 데이터를 가져옵니다.
        line_count++;
        if (so->finished)
        {
            so->full = FALSE;
            printf("cond_%u: is_finished : signal & unlock mutex\n", self);
            pthread_cond_signal(&so->cond);
            pthread_mutex_unlock(&so->lock);
        }
        char_count = strlen(buf); // 없어도 무관 - TODO: 알파벳별 추가
        line_count++;
        printf("cond_%u: read line %s\n", self, buf);
        free(so->line); // getdelimeter 동적할당 버퍼 해제
        printf("cond_%u: buffer read : signal & unlock mutex\n", self);
        pthread_cond_signal(&so->cond);
        pthread_mutex_unlock(&so->lock);
    }
    *ret = line_count;
    pthread_exit(ret);
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

    char* read_file_path = argv[1];
    int consumer_count = atoi(argv[2]);
    int producer_count = atoi(argv[3]);

    if (consumer_count == 0 || producer_count == 0)
    {
        print_usage();
       	// exit(-1);
    }

    // 파일을 읽기 모드로 엽니다.
    FILE* rfile;
    rfile = fopen(read_file_path, "rb");
    if (rfile == NULL)
    {
        printf("파일을 열 수 없습니다.");
        exit(-1);
    }

    // share 오브젝트를 생성합니다.
    // 모든 producer, consumer 스레드에서 공유하게 됩니다.
    so_t *share = malloc(sizeof(so_t));
    share->rfile = rfile;
    share->line = NULL;
    share->full = FALSE;
    share->finished = FALSE;

    // critical section 에서 사용될 객체를 생성합니다.
    // 구조체포인터-> 객체 | pthread_..._init에서 포인터받으므로 & 붙여야합니다.
    pthread_cond_init(&share->cond, NULL); // NULL : 기본 설정 사용
    pthread_mutex_init(&share->lock, NULL);
    
    printf("pthread mutex / condition initialized\n");

    // 각 스레드에 대한 정보를 담고 있는 구조체입니다.
    pthread_t producers[MAX_THREADS];
    pthread_t consumers[MAX_THREADS];

    for (int i = 0; i < producer_count; i++)
    {
        pthread_create(&producers[i], NULL, procedure_producer, share);
    }

    for (int i = 0; i < consumer_count; i++)
    {
        pthread_create(&consumers[i], NULL, procedure_consumer, share);
    }
    printf("pthread producer / consumer initialized\n");

    int exitcode;  // 스레드 
    int *ret; // 스레드 반환값

    // 스레드가 종료 될 때까지 대기합니다.
    for (int i = 0; i < consumer_count; i++)
    {
        exitcode = pthread_join(consumers[i], (void**) &ret); // ret에 실행결과를 저장합니다. 
        printf("main(): consumer_%d joined / return %d", i, *ret);
    }
    for (int i = 0; i < producer_count; i++)
    {
        exitcode = pthread_join(producers[i], (void**) &ret); // ret에 실행결과를 저장합니다. 
        printf("main(): producer_%d joined / return %d", i, *ret);
    }

    // 사용된 공유 객체를 해제합니다.
    free(share);
}

