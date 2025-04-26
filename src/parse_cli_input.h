#ifndef PARSE_CLI_INPUT_H
#define PARSE_CLI_INPUT_H

/*----------------------------- Public Includes ------------------------------*/
#include <stddef.h>
/*--------------------------- Public Includes END ----------------------------*/

/*----------------------- Public Variable Declarations -----------------------*/
extern int mysh_argc;
extern char **mysh_argv;
extern size_t mysh_argv_sz;
/*--------------------- Public Variable Declarations END ---------------------*/

/*------------------------ Public Function Prototypes ------------------------*/
void parse_cli_input_cleanup(void);
int parse_cmd_args(const char *input);
/*---------------------- Public Function Prototypes END ----------------------*/

#endif
