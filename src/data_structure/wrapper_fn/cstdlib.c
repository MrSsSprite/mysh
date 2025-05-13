#include "cstdlib.h"
#include "string.h"


/*----------------------------- Public Variables -----------------------------*/
size_t _wrapper_n = 0;
/*--------------------------- Public Variables END ---------------------------*/


/*----------------------------- Public Functions -----------------------------*/
int _wpr_memcmp_count(const void *lhs, const void *rhs)
{
   return memcmp(lhs, rhs, _wrapper_n);
}
/*--------------------------- Public Functions END ---------------------------*/
