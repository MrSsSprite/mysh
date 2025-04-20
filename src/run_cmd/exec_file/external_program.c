/*----------------------------- Private Includes -----------------------------*/
#include "external_program.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
/*--------------------------- Private Includes END ---------------------------*/

/*---------------------------- Exported Functions ----------------------------*/
void fork_exec_cmd(const char *cmd, int argc, char **argv)
{
   pid_t pid = fork();
   if (pid == 0)
   {
      execv(cmd, argv);
      perror("execv");
      exit(1);
   }
   else if (pid < 0)
      perror("fork");
   else
   {
      int status;
      waitpid(pid, &status, 0);
      if (!WIFEXITED(status))
         fputs("execv error\n", stderr);
   }
}
/*-------------------------- Exported Functions END --------------------------*/
