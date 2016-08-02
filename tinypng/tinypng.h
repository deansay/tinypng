/*
	tiny implementation for create rgba format PNG stream
	by scinyeah
	20160801
*/

#ifndef __TINY_PNG_H__
#define __TINY_PNG_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


int png_rgba_create(int w, int h, uint8_t* buf, uint8_t **out, size_t * size);
 
void png_rgba_free(uint8_t* out);

int png_rgba_save(int w, int h, uint8_t* buf, const char * name);


#ifdef __cplusplus
}
#endif

#endif