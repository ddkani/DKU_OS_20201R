#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define COFFEE 100000
#define THREADS 10

int coffee_supply = COFFEE;
int coffee_sell = 0;

pthread_mutex_t lock;

void *employee(void *arg)
{  
    unsigned int tid = (unsigned int)pthread_self;
    printf("%u employee init\n", tid);
    while (1)
    {
        pthread_mutex_lock(&lock);
        if (!coffee_supply)
        {   
            pthread_mutex_unlock(&lock);
            break; // 실행이 끝났으니 뮤택스를 unlock 하고 종료합니다.
        }
        coffee_supply--; // 커피를 만들어서
        coffee_sell++; // 팔았습니다.

        pthread_mutex_unlock(&lock); // 다른 스레드를 실행합니다.
    } 
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
        int* pthread_ret;
        pthread_t threads[THREADS];
        pthread_mutex_init(&lock, NULL);

        for (int i = 0; i < THREADS; i++)
            pthread_create(&threads[i], NULL, employee, NULL);
        for (int i = 0; i < THREADS; i++)
            pthread_join(threads[i], (void **)&pthread_ret);

        printf("sell %d\n", coffee_sell);

        pthread_mutex_destroy(&lock);
        return 0;
}

