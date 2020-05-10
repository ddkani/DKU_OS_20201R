#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


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

typedef struct sharedobject { 
    pthread_mutex_t lock; // lock
	pthread_cond_t cond; // cond

	FILE *rfile;

	int linenum; // 현재 줄 번호
	int full; // 버퍼 존재 유무
    char *buf; // 읽은 데이터

    int finished; // 읽기 종료 여부
} so_t;


void *producer(void *arg) {
	so_t *so = arg;
    int *ret = malloc(sizeof(int)); // pthread 반환값
	FILE *rfile = so->rfile;
	    
    int linenum = 0;

	char *buf = NULL;
	size_t len = 0; // getdelim 내부 버퍼. 중요하지 않음.
	ssize_t read = 0;

    unsigned int self = (unsigned int)pthread_self();

	while (TRUE) {
        // 뮤텍스를 잠급니다. - consumer 에서 다 읽을때까지 기다립니다.
        printf("prod_%u: try to acquire mutex\n", self);
		pthread_mutex_lock(&so->lock);
        printf("prod_%u: mutex acquired\n", self);

		read = getdelim(&buf, &len, '\n', rfile);
		if (read == -1) 
        {
			// 파일의 끝에 다다랐습니다. 더 읽을 수 없습니다.
            so->full = TRUE;
            so->finished = TRUE;
            // 다 읽었음을 알리고 푸르

            printf("prod_%u: file end: signal / unlock mutex\n", self);
			pthread_cond_signal(&so->cond);
			pthread_mutex_unlock(&so->lock);
			break;
		}
		while(so->full == TRUE) {
		    pthread_cond_wait(&so -> cond, &so -> lock); 
        }
        // printf("read:%lu len: %ld slen: %d\n", read, len, strlen(buf));  // 데이터의 크기를 확인하고자 함.
        // strlen :: 문자열의 길이 (아스키 파일에서는 맞음)
        // read :: 읽은 전체 데이터의 길이

		so->linenum = linenum;
		so->buf = strdup(buf); 
		linenum++;
		so->full = TRUE;
		pthread_cond_signal(&so->cond);
		pthread_mutex_unlock(&so->lock);
	}
	free(buf);
	printf("Prod_%x: %d lines\n", (unsigned int)pthread_self(), linenum);
	*ret = linenum;
	pthread_exit(ret);
}

void *consumer(void *arg) {
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	int linenum = 0;
	int len;
	char *buf;

    unsigned int self = (unsigned int)pthread_self();

	while (TRUE) {
        printf("cond_%u: try to acquire mutex\n", self);
		pthread_mutex_lock(&so->lock);
        printf("cond_%u: mutex acquired\n", self);

        // 버퍼를 아직 가져오지 못했다면, 버퍼가 찰 때까지 대기합니다.
		while(so->full == FALSE) 
        {
            printf("cond_%u: wait condition - FULL == TRUE -> FALSE\n", self);
			pthread_cond_wait(&so -> cond, &so -> lock); 
		}
		buf = so->buf;

		if (so->finished == TRUE) 
        {
			if(buf != NULL)
            {
				printf("Cons_%x: [%02d:%02d] %s",
						(unsigned int)pthread_self(), linenum, so->linenum, buf);
				linenum++;
				so -> buf = NULL;
			}	
			so->full = FALSE;
			pthread_cond_signal(&so->cond);
			pthread_mutex_unlock(&so->lock);
			break;
		}

		len = strlen(buf);
		printf("Cons_%x: [%02d:%02d] %s",
			(unsigned int)pthread_self(), linenum, so->linenum, buf);
		free(so->buf);
		linenum++;
		
		so->full = FALSE;
 		pthread_cond_signal(&so->cond);
		pthread_mutex_unlock(&so->lock);
	}
    printf("cond_%u: buffer read : signal & unlock mutex\n", self);
	*ret = linenum;
	pthread_exit(ret);
}

void print_usage()
{
    printf("word_count - DKU2020OS HW2\n\n");
    printf("usage : word_count read_file_path consumer_count producer_count\n");
}

int main (int argc, char *argv[])
{

    if (argc != 4)
    {
        print_usage(); exit(-1);
    }
    char* read_file_path = argv[1];
    int consumer_count = atoi(argv[2]);
    int producer_count = atoi(argv[3]);

    // 파일을 읽기 모드로 엽니다.
    FILE* rfile;
    rfile = fopen(read_file_path, "rb");
    if (rfile == NULL)
    {
        printf("파일을 열 수 없습니다.");
        exit(-1);
    }


	so_t *share = malloc(sizeof(so_t));
	memset(share, 0, sizeof(so_t));
	share->rfile = rfile;
	share->buf = NULL;
	share->full = FALSE;
    share->finished =FALSE;

    // Critical section 에서 사용될 객체를 생성합니다.
	pthread_mutex_init(&share->lock, NULL);
	pthread_cond_init(&share->cond, NULL);

    // 각 스레드에 대한 정보를 담고 있는 구조체입니다.
    pthread_t producers[MAX_THREADS];
    pthread_t consumers[MAX_THREADS];

    // 스레드를 시작합니다.
	for (int i = 0 ; i < producer_count ; i++)
		pthread_create(&producers[i], NULL, producer, share);
	for (int i = 0 ; i < consumer_count ; i++)
		pthread_create(&consumers[i], NULL, consumer, share);
	printf("main continuing\n");

    int *pthread_ret;
    int pthread_exitcode;
    
    // 스레드가 종료할때까지 대기합니다.
    // pthread_join() : 스레드가 종료할때까지 대기
	for (int i = 0 ; i < consumer_count ; i++) {
		pthread_exitcode = pthread_join(consumers[i], (void **) &pthread_ret);
		printf("main: consumer_%d joined with %d\n", i, *pthread_ret);
	}
	for (int i = 0 ; i < producer_count ; i++) {
		pthread_exitcode = pthread_join(producers[i], (void **) &pthread_ret);
		printf("main: producer_%d joined with %d\n", i, *pthread_ret);
	}

    
	pthread_exit(NULL);
	exit(0);
}