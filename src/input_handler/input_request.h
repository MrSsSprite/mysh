#ifndef INPUT_REQUEST_H
#define INPUT_REQUEST_H

/*----------------------------- Public Includes ------------------------------*/
#include <stddef.h>
/*--------------------------- Public Includes END ----------------------------*/

/*----------------------------- Public Variables -----------------------------*/
extern char *input_buf;
extern size_t input_buf_sz;
/*--------------------------- Public Variables END ---------------------------*/

/*------------------------ Public Function Prototypes ------------------------*/
int input_request_init(void);
int input_request(void);
/*---------------------- Public Function Prototypes END ----------------------*/

#endif
