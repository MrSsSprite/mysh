/*----------------------------- Private Includes -----------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "builtin.h"
#include "external_program.h"
#include "parse_cli_input.h"
#include "redirection.h"
#include "input_request.h"
/*--------------------------- Private Includes END ---------------------------*/

/*----------------------------- Private Defines ------------------------------*/
#define MIN_ARGV_SZ 32
#define RECOMMANDED_INPUTBUF_SIZE   (64u)
/*--------------------------- Private Defines END ----------------------------*/

/*---------------------------- Private Variables -----------------------------*/
/*-------------------------- Private Variables END ---------------------------*/

/*----------------------- Private Function Prototypes ------------------------*/
static int main_init(void);
static void main_cleanup(void);
/*--------------------- Private Function Prototypes END ----------------------*/


int main(void)
{
   int err_code;
   const struct fn_map *fn_ptr;
   const char *exec_path;

   err_code = main_init();
   if (err_code)
   {
      fprintf(stderr, "Failed to initialize: err_code=%d\n", err_code);
      return -1;
   }

   while ((err_code = input_request()) == 0)
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
         exit(err_code);
      }

      err_code = redirect_vec(parsed_redir_table);
      if (err_code)
      {
         fputs("redirection error\n", stderr);
         return err_code;
      }

      if (mysh_argc == 0)
         continue;
      if ((fn_ptr = builtin_fn_find(mysh_argv[0])))
         fn_ptr->fn(mysh_argc, mysh_argv);
      else if ((exec_path = find_exe_full_path(mysh_argv[0])))
         fork_exec_cmd(exec_path, mysh_argc, mysh_argv);
      else
         printf("%s: command not found\n", mysh_argv[0]);

      err_code = redir_recover();
      if (err_code)
      {
         fputs("redirection recovery error\n", stderr);
         return err_code;
      }
   }

   main_cleanup();
   return 0;
}


/*---------------------------- Private Functions -----------------------------*/
static int main_init(void)
{
   if (redirect_init()) return 1;
   if (input_request_init()) return 2;
   return 0;
}

static void main_cleanup(void)
{
   builtin_cleanup();
   parse_cli_input_cleanup();
   redirect_cleanup();
   free(input_buf);
   input_buf_sz = 0u;
}
/*-------------------------- Private Functions END ---------------------------*/
