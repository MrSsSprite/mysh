#ifndef PARSE_FNS_H
#define PARSE_FNS_H

/*----------------------------- Public Includes ------------------------------*/
#include "Vector.h"
/*--------------------------- Public Includes END ----------------------------*/

/*------------------------ Public Function Prototypes ------------------------*/
int parse_envar(const char **in_iter, Vector str);
int parse_dquote(const char **in_iter, Vector str);
int parse_token(const char **in_iter, Vector str);
/*---------------------- Public Function Prototypes END ----------------------*/

#endif
