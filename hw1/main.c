#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* malloc, strncpy, strcpy */
#include <unistd.h>
#include <sys/wait.h>

#include "global_config.h"
#include "functions.h"


/*

*****************
사용자 환경변수들과 입력 명령어, 정리된 명령어 (인자값 등)은
여러 함수에서 많이 사용되므로 전역변수로 두는 것이 더 작성에 용이하다고 판단하였음.

*/

char* user; // USER
char hostname[1024]; // from gethostname
char* workspace; // PWD in environment

char user_input[MAX_USER_INPUT_SIZE]; // user commandline!
char* command; // process name (to execute) -> 해당 경로들에서 파일을 찾습니다.

char** args; // arguments
int args_sz;

char** path_dirs; // paths
int path_dirs_sz;

extern char **environ; // 응용 프로그램의 전체 환경변수를 가져옵니다.


// 호스트 이름, 유저 정보를 가져옵니다.
void set_basic_environment()
{
  memset(hostname, '\0', 1024);
  gethostname(hostname, 1023);

  user = getenv("USER");
  workspace = getenv("PWD");
}


// 사용자로부터 명령어를 입력받고, 파싱합니다.
void receive_commandline()
{
    input:
    memset(user_input, '\0', MAX_USER_INPUT_SIZE);
    // format: username@hostname:DIR $
    printf("%s@%s:%s $ ", user, hostname, workspace);
    fgets(user_input, MAX_USER_INPUT_SIZE - 1, stdin);

    int user_input_sz = strlen(user_input) - 1;
    // 명령어가 아무것도 입력 안됨
    if (user_input_sz == 0) goto input; // 다시 돌아가서 반복

    user_input[user_input_sz] = '\0';

    // 공백으로 프로세스 실행 인자값을 파싱합니다.
    int input_sz = 0;
    char** results = split_str(user_input, " ", &input_sz);

    args_sz = input_sz;
    if (args_sz) args = results;

    command = results[0];

    #ifdef DEBUG
    printf("user_input: %s\n", user_input);
    printf("input size: %d\n", input_sz);
    printf("arguments size: %d\n", args_sz);
    printf("command: %s\n", command);
    #endif
}


// 프로그램이 위치하는 경로를 찾고, 실행합니다. 
void execute_command()
{
    char* executable = NULL;
    char buf[MAX_STR_LENGTH];
    memset(buf, '\0', MAX_STR_LENGTH);

    if (is_executable(command)) // 전체 경로로 입력된 경우
    {
       executable = command;
    }

    // 전체 PATH에서 사용자가 입력한 프로그램을 찾습니다.
    else
    {
      for (int i = 0; i < path_dirs_sz; i++)
      {
         char* path = path_dirs[i];
         memset(buf, '\0', strlen(buf));
         int path_sz = strlen(path);

         strncpy(buf, path, path_sz);
         buf[path_sz] = '/';
         strcat(buf, command);

         if (is_executable(buf))
         {
              executable = buf;
              break;
         }
      }
    }

    if (executable == NULL)
    {
      // 명령어를 찾지 못하였으므로 반환하고 종료합니다.
      printf("sish: command not found or not executable file: %s\n", command);
      return;
    }

    #ifdef DEBUG
    printf("executable path is = %s\n", executable);
    printf("exeuctable v = %p\n", executable);
    #endif

   int pid = fork();
   int status_exit = 0;

   if (pid == -1) // fork() 실패
   {
      perror("[X] Oops.. something went wrong while forking!!");
      exit(-1);
   }
   else if (pid == 0) // 자식 프로세스에서 사용자 명령 실행
   {
      // args[0] 번째 주소를 변경해줍니다.
      args[0] = executable;

      // execve => 0번째 인자값과 args[0]값이 동일해야함.
      execve(executable, args, environ);
   }
   else // 부모 프로세스에서 자식 프로세스가 완료되기를 기다림
   {
      wait(&status_exit);
   }
}


int main(int argc, char * argv[])
{
	path_dirs = split_str(getenv("PATH"), ":", &path_dirs_sz);
  set_basic_environment();

  while (1)
  {
    receive_commandline();
    if (strcmp(command, "quit") == 0)
    {
        printf("Bye.\n");
        return 0;
    }
    execute_command();
  }

	// PATH 경로 저장을 위해 동적 할당했던 메모리를 해제
	for (int i = 0; path_dirs[i] != 0; i++) free(path_dirs[i]);
	free(path_dirs);

	return 0;
}
