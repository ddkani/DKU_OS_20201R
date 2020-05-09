#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>

int global_counter = 0;
pthread_mutex_t lock;

// thread_inc
static void * thread_inc(void *arg)
{
	int i;
	for (int i = 0 ; i < 100000000 ; i++) {
		// 공유 변수에 접근을 위해서 mutex lock을 이용하고 있다.
		// lock을 가지고 있는 동안은 공유 변수에 접근하여도 된다.
		pthread_mutex_lock(&lock);
		global_counter++;
		pthread_mutex_unlock(&lock);
	}
	// thread exit
	pthread_exit(0);
}

// thread_dec
static void * thread_dec(void *arg)
{
	int i;
	for (int i = 0 ; i < 100000000 ; i++) {
		pthread_mutex_lock(&lock);
		global_counter--;
		pthread_mutex_unlock(&lock);
	}
	// thread exit
	pthread_exit(0);
}


/*
gettimeofday / settimeofday => 프로그램 내에서 실행시간 구하기

man function_name 으로 시스템에 일치하는 기본적인 함수 정보를 확인할 수 있음.

reverse-i-search?
*/


int main(int argc, char *argv[])
{

	pthread_t thr[4];
	void *res;

	// mutex init
	// 동기화 lock 변수를 생성합니다.
	pthread_mutex_init(&lock, NULL);

	// print global_counter;
	printf("global_counter: %d\n", global_counter);

	void *res;
	struct timeval start, end;

	

	//create two threads
	pthread_create(&thr[0], NULL, thread_inc, NULL);
	pthread_create(&thr[1], NULL, thread_dec, NULL);

	pthread_join(thr[0], &res);
	pthread_join(thr[1], &res);

	// gettimeofday for getting start time
	gettimeofday(&start)

	// print global_counter;
	// 실행된 시간까지 구합니다.
	printf("global_counter: %d %ld %d\n", global_counter, end.tv_sec - start.tv_sec, end_tv.usec - start.tv_usec);


	global_counter = 0;
	// print global_counter;
	printf("reset counter\nglobal_counter: %d\n", global_counter);

	//create two threads
	pthread_create(&thr[2], NULL, thread_inc, NULL);
	pthread_create(&thr[3], NULL, thread_inc, NULL);

	pthread_join(thr[2], &res);
	pthread_join(thr[3], &res);

	// print global_counter;
	printf("global_counter: %d\n", global_counter);

	return 0;
}
