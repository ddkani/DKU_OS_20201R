#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

 // 공유 변수 접근 제한을 위한 mutex 
pthread_mutex_t lock;
// 스레드에서 공유하고자 하는 전역 변수
int global_counter = 0;

typedef struct sharedobject {
    FILE *rfile;
    int linenum;
    char *line;
    pthread_mutex_t lock;
    pthread_cond_t  cv;
    int full;
} so_t;

// 여러개의 버퍼를 만들어야 한다.

// 여러명의 consumer 가 동시에 동작
/*
c_index
lock(), c_index++, unlock()

p_index

*/

// pthread_mutex_lock : 오버헤드가 큰 작업
// lock을 하지 못하면 busy-waiting이 아닌 sleep하게 된다.

// 같은 mutex를 가진 함수끼리는 뮤텍스를 잡는 순간 critical section에 들어갑니다.

// pthread_cond_wait? =>  cond & cond_var
// 컨디션 체크 시에도 동기화가 필요합니다.

/*
버퍼가 차 있다면, 버퍼에서 문자를 읽어서 stdout에 씁니다.
*/
static void * thread_consumer()
{

    so_t *so = arg;
    int *ret = malloc(sizeof(int));
    int i = 0;
    int len;
    char* line;

    while (1) {
        pthread_mutex_lock(&so->lock);

        // consumer waits until buffer is full
        while (so->full == 0) { // 버퍼가 찰 때까지 기다립니다.
            pthread_cond_wait(&so->cv, &so->lock);
        }

        //  now, buffer is full
        // mutex lock is hold * critical section

        line = so -> line;
        if (line == NULL) {
            pthread_mutex_unlock(&so->lock);
            break;
        }
        len = strlen(line);
        printf("Cons_%x: [%02d:%02d} %s",
                (unsigned int)pthread_self(), i, so->linenum, line);
        
        // consume completed
        so->full = 0;
        pthread_cond_signal(&so->cv);
        pthread_mutex_unlock(&so->lock);
    }

    printf("Cons: %d lines\n", i);
    *ret = i;





    pthread_mutex_lock(&lock);
    while(cond == false) // 버퍼가 비어 있다면 - 들어가지 않는다.
    {
        pthread_cond_wait(&cond_var, &lock); // producer작업이 완료되었음을 알릴 때까지 기다린다.
        // sleeps until somebody wakes up
    }
    // 버퍼가 채워졌으며, 버퍼가 차 있다.
    // get the lock
    // do the job
    // ??
    pthread_cond_signal(&cond_var_);
    pthread_mutex_unlock(&lock);
}

/*
파일을 읽고, 공유 버퍼에 데이터를 넣습니다.
*/
static void * thread_producer(void *arg)
{   
    so_t *so = arg;
    int *ret = malloc(sizeof(int));
    char *line = NULL;
    int i = 0;
    size_t len = 0;
    ssize_t read = 0;
    
    while (1) 
    {
        read = getdelim(&line, &len, '\n', rfile);
        // for the safety in the following if-case 
        pthread_mutex_lock(&so->lock);


        // if buffer is empty
        // place the line on the shared buffer

        // pthread_mutex_lock(&so->lock);

        // 버퍼가 비어있는 상태
        while (so -> full == 1) {
            // 컨디션이 맞는 경우에만 뮤텍스를 잡고 대기한다.
             pthread_mutex_cond_wait(&so->cv, &so->lock);
        }

         if (read == -1)  { // end of file?
            so -> full = 1;
            so -> line = NULL;
            pthread_cond_signal(&so->cv);
            pthread_mutex_unlock(&so->lock);
            break;
        }

        // 버퍼가 차있는 상태
        so -> linenum = i;
        so -> line = strdup(line);
        i++;

        // producer의 작업이 끝났으니, 퍼커가 차있음을 알린다.
        so->full = 1;
        pthread_cond_signal(&so->cv);
        pthread_mutex_unlock(&so->lock);
    }
    

    pthread_mutex_lock(&lock);
    // do the job
    // signal another waiting thread
    pthread_cond_signal(&cond_var);
    pthread_mutex_unlock(&lock);
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
        print_usage(); exit(-1);
    }

    share->rfile = rfile;
    share->line = NULL;

    pthread_mutex_init(&share->lock, NULL);
    pthread_mutex_init(&share->cv, NULL);

    printf("%s %d %d \n", read_file_path, consumer_count, producer_count);

    pthread_mutex_init(&lock, NULL);
}