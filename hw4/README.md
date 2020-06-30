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
