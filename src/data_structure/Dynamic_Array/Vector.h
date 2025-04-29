#ifndef __VECTOR_H
#define __VECTOR_H

/*----------------------------- Public Includes ------------------------------*/
#include <stddef.h>
/*--------------------------- Public Includes END ----------------------------*/

/*------------------------------ Public Defines ------------------------------*/
#define vector_init(elem_type) vector_init_elem_sz(sizeof(elem_type))
/*---------------------------- Public Defines END ----------------------------*/

/*----------------------------- Public Typedefs ------------------------------*/
typedef struct vector *Vector;
/*--------------------------- Public Typedefs END ----------------------------*/

/*------------------------ Public Function Prototypes ------------------------*/
struct vector *vector_init_elem_sz(size_t elem_sz);
void vector_destroy(struct vector *this);

size_t vector_size(struct vector *this);
size_t vector_capacity(struct vector *this);

int vector_push(struct vector *this, const void *item);
int vector_pop(struct vector *this);
int vector_insert(struct vector *this, void *restrict dest,
                  const void *restrict first, size_t n);
int vector_erase(struct vector *this, void *first, size_t n);
int vector_resize(struct vector *this, size_t new_size);

void *vector_begin(struct vector *this);
void *vector_end(struct vector *this);
/*---------------------- Public Function Prototypes END ----------------------*/

#endif
