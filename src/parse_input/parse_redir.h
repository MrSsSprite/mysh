#ifndef PARSE_REDIR_H
#define PARSE_REDIR_H

/*----------------------------- Public Functions -----------------------------*/
#include "Vector.h"
/*--------------------------- Public Functions END ---------------------------*/

/*------------------------ Public Function Prototypes ------------------------*/
int parse_redirection(const char **in_iter, Vector str);
int redir_table_clear(void);
void redir_table_destroy(void);
/*---------------------- Public Function Prototypes END ----------------------*/

#endif
