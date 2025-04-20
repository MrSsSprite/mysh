/*----------------------------- Private Includes -----------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "builtin.h"
#include "external_program.h"
#include "parse_cli_input.h"
/*--------------------------- Private Includes END ---------------------------*/

/*----------------------------- Private Defines ------------------------------*/
#define MIN_ARGV_SZ 32
#define RECOMMANDED_INPUTBUF_SIZE   (64u)
/*--------------------------- Private Defines END ----------------------------*/

/*---------------------------- Private Variables -----------------------------*/
static char *input_buf = NULL;
static size_t input_buf_sz = 0u;
/*-------------------------- Private Variables END ---------------------------*/

/*----------------------- Private Function Prototypes ------------------------*/
static int input_request(void);
static void main_cleanup(void);
/*--------------------- Private Function Prototypes END ----------------------*/


int main(void)
{
   int err_code;
   const struct fn_map *fn_ptr;
   const char *exec_path;

   while ((err_code = input_request()))
   {
      if (err_code < 0)
      {
         fputs("malloc error\n", stderr);
         exit(1);
      }
      err_code = parse_cmd_args(input_buf);
      switch (err_code)
      {
      case 0:
         break;
      case 1:
         fputs("malloc error\n", stderr);
         exit(1);
      case 2:
         continue;
      default:
         fputs("Designed Error: This place should not be reached\n", stderr);
         exit(2);
      }

      if ((fn_ptr = builtin_fn_find(mysh_argv[0])))
         fn_ptr->fn(mysh_argc, mysh_argv);
      else if ((exec_path = find_exe_full_path(mysh_argv[0])))
         fork_exec_cmd(exec_path, mysh_argc, mysh_argv);
      else
         printf("%s: command not found\n", mysh_argv[0]);
   }

   main_cleanup();
   return 0;
}


/*---------------------------- Private Functions -----------------------------*/
static int input_request(void)
{
   char ch;
   size_t i = 0u;

   fputs("$ ", stdout);
   while (1)
   {
      ch = getchar();
      if (ch == '\n')
         ch = '\0';
      else if (ch == EOF)
      {
         if (i == 0)
            return 0;
         else
            ch = '\0';
      }
      if (i == input_buf_sz)
      {
         size_t new_sz = input_buf_sz ? input_buf_sz * 2u :
                                        RECOMMANDED_INPUTBUF_SIZE;
         void *tmp = realloc(input_buf, new_sz);
         if (tmp)
         {
            input_buf = tmp;
            input_buf_sz = new_sz;
         }
         else
            return -1;
      }

      input_buf[i++] = ch;
      if (ch == '\0') return 1;
   }
}

static void main_cleanup(void)
{
   builtin_cleanup();
   free(mysh_argv);
   mysh_argv_sz = 0u;
   free(input_buf);
   input_buf_sz = 0u;
}
/*-------------------------- Private Functions END ---------------------------*/
