#include <stdlib.h>
#include "tinypng.h"

int main(int argc, char *argv[])
{
	const int w = 2000;
	const int h = 2000;
	uint8_t* p;
	uint8_t* d = (uint8_t*)malloc(w * h * 4);
	if (!d)
		return -1;
	
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			p = d + (i * w + j) * 4;
			p[0] = 255 * i / h;
			p[1] = 0;
			p[2] = 0;
			p[3] = 255 * j / w;
		}
	}
	
	png_rgba_save(w, h, (uint8_t*)d, "out.png");
	free(d);
	return 0;
}