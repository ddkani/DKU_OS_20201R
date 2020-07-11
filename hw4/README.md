# 32187345 양해찬 - 가상 파일시스템 시뮬레이션

시뮬레이션 구현 - 과제 옵션의 'C' 단계를 구현했습니다.
- 파일 리스트 출력 / 디렉터리 구조를 저장하는 구조체 구현

▶ make: 'fs_simulator' is up to date. 오류가 발생하면 ‘make clean’ 후 재실행합니다.

```
live2skull@live2skull:~/hw4_original$ ./fs_simulator
DKU2020OS - Simple file system implementation
Haechan Yang - 32187345

list directory of /
. 0
.. 3264
file_1 37
file_2 40
file_3 40
file_4 38
file_5 39
file_6 37
file_7 37
file_8 39
file_9 38
file_10 40
file_11 40
file_12 40
file_13 40
file_14 39
...
file_28 39
file_29 40
file_30 41
> /file_13
read file /file_13) size: 40 content: file 13 2041852688 528010622 1296581201


>

```

----

과제 원본 글 

# 2020_os_hw4
Simple File System

please refer 
hw4_simple_file_system.pdf file

Similar to hw#3,

option|choices|To-do list
|------|---|-----------|
a.| Simple |List files in the root directory 
b.| Complicated |Simple + open/read/close file_1 and print out the contents in the file_1
c.| Complex |Complicated + (option1 OR option2 OR option3 OR option4)

-. option 1
  * add some new data structure for quickly finding directory entry / inode
  
-. option 2
  * add buffer cache to quickly obtain files' metadata
  
-. option 3
  * add copy-on-write when two users open the file
  
-. option 4
  * implement filesystem with VM (hw#3 programming assignment).
