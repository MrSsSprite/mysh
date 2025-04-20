#ifndef BUILTIN_H
#define BUILTIN_H

/*----------------------------- Public Includes ------------------------------*/
#include <stddef.h>
/*--------------------------- Public Includes END ----------------------------*/

/*------------------------------ Public Structs ------------------------------*/
struct fn_map
{
   const char *name;
   int (*fn)(int argc, char **argv);
};
/*---------------------------- Public Structs END ----------------------------*/

/*---------------------------- Exported Variables ----------------------------*/
extern const struct fn_map builtin_fn_map[];
extern const size_t builtin_fn_map_sz;
/*-------------------------- Exported Variables END --------------------------*/

/*------------------------ Public Function Prototypes ------------------------*/
const struct fn_map *builtin_fn_find(const char *fn_name) ;
char *find_exe_full_path(const char *cmd);
void builtin_cleanup(void);
/*---------------------- Public Function Prototypes END ----------------------*/

#endif
