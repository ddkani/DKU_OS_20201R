
# 과제 요약

동기화를 통한 멀티 스레드 프로그래밍.
스레드는 실행 단위. 실행 컨텍스트가 있음. 메모리 주소 공간은 프로세스에서 스레드에게 공유되므로 스레드간의 메모리 액세스를 명확하게 분리 / 보호 할 수 없음.

예제 코드
- 여러 스레드 사용을 위한 기본 코드 있음. 파일을 읽고 화면에 쓰려고 시도함.
- 기본스레드(프로세스) / 생산자-파일을읽고 공유버퍼에 데이터넣기 / 소비자-버퍼에서 읽고 화면에 표시
- 하지만 스레드가 서로 독립하게 실행하므로 메모리 문제로 충돌나거나, 서로 무한 대기하게 됨
- 스레드에서 올바른 값을 가지려면, 여러 데이터가 한 데이터의 일관성을 유지해야 함.
- 동기화 하는 스레드 간의 실행을 신중하게 제어한다.

pthread 동기화 pthread_mutex_lock() / pthread_mutex_unlock() , 공유되는 조건 변수는 pthread_cond_wait() / pthread_cond_signal()

요구사항
- 각각 1개의 consumer, producer가 작동해야 함
- 여러 consumer를 지원
- consumer가 화면에 표시한 텍스트 문자의 갯수 수집 / 출력하여 통계 작성
- 프로세스 인자값으로 예제 파일, consumer, producer 갯수 지정

https://mobile-os.dankook.ac.kr/data/FreeBSD9-orig.tar 또는 /org/FreeBSD9-orig.tar 를 예제로 활용,
htop으로 프로세스, 스레드 실행 시간 확인.

make파일을 작성하고, 프로젝트 소개, 중요한 구현의 세부 사항, 프로그램(코드)의 구조 포함



# ORIGINAL CONTENTS:
# 2020_os_hw2
HW2: Multi-threaded word count

**Due date: May 9th**

The second homework is about multi-thread programming with some synchronization.
Thread is a unit of execution; a thread has execution context, 
    which includes the registers, stack.
Note that address space (memory) is shared among threads in a process, 
    so there is no clear separation and protection for memory access among threads.

The example code includes some primitive code for multiple threads usage.
It basically tries to read a file and print it out on the screen.
It consists of three threads: main thread for admin job 
    second thread serves as a producer: reads lines from a file, and put the line string on the shared buffer
    third thread serves as a consumer:  get strings from the shared buffer, and print the line out on the screen

Unfortunately, the code is not working because threads runs independently from others.
the result is that different threads access invalid memory, and have wrong value, and crash or waiting for terminated thread infinitely.
To make it working, you have to touch the code so that the threads have correct value.

To have correct values in threads, you need to keep consistency for data touched by multiple threads.
To keeping consistency, you should carefully control the execution among threads, which is called as synchronization.

pthread_mutex_lock()/pthread_mutex_unlock are the functions for pthreads synchorinization.
For condition variable, you may need to look up functions such as pthread_cond_wait()/pthread_cond_signal().

The goals from HW2 are 

1. correct the code for prod_cons.c so that it works with 1 producer and 1 consumer

2. enhance it to support multiple consumers.

3. Make consumer(s) to gather some statistics of the given text in the file. 
Basically, count the number of each alphabet character in the line.
char_stat.c can be a hint for gathering statistics.
At the end of execution, you should print out the statistics of the entire text.
Beat the fastest execution, maximizing the concurrency!

To run a program, you may give filename to read and # of producers and # of consumers.
In case of single producer, 2 consumers, reading 'sample file'; you may need to execute your program by
./prod_cons ./sample 1 2 

You can download some example input source code from the link: [https://mobile-os.dankook.ac.kr/data/FreeBSD9-orig.tar] or  
you can use /opt/FreeBSD9-orig.tar from our server.

Please make some document so that I can follow to build/compile and run the code.
It would be better if the document includes some introduction and some important implementation details or your program structure.

htop is a program that shows threads execution in the system.

Measure & compare of execution time for different # of threads

Happy hacking!
Seehwan
