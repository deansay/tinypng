#include <stdlib.h>
#include <stdio.h>
#include "tinypng.h"

static uint32_t crc(uint8_t *buf, int len)
{
	static int crc_table_computed = 0;
	static uint32_t crc_table[256];
	uint32_t c, n, k;
	if (!crc_table_computed)
	{
		for (n = 0; n < 256; n++)
		{
			c = n;
			for (k = 0; k < 8; k++)
				c = (c & 1) ? 0xedb88320L ^ (c >> 1) : c >> 1;
			crc_table[n] = c;
		}
		crc_table_computed = 1;
	}
	c = 0xffffffffL;
	for (n = 0; n < len; n++)
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
	return c ^ 0xffffffffL;
}


static uint8_t* w_bytes(uint8_t * dst, uint8_t * buf, size_t size)
{
	memcpy(dst, buf, size);
	return dst + size;
}

static uint8_t* w_beu32(uint8_t * dst, uint32_t u32)
{
	dst[0] = (u32 >> 24) & 0xff;
	dst[1] = (u32 >> 16) & 0xff;
	dst[2] = (u32 >> 8) & 0xff;
	dst[3] = (u32 >> 0) & 0xff;
	return dst + 4;
}

static uint8_t* w_zlen(uint8_t * dst, uint32_t u32)
{
	dst[0] = u32 & 0xff;
	dst[1] = (u32 >> 8) & 0xff;

	u32 = ~u32;
	dst[2] = u32 & 0xff;
	dst[3] = (u32 >> 8) & 0xff;
	return dst + 4;
}

static void adler32(int32_t *a, int32_t *b, uint8_t *data, size_t len)
{
	while (len != 0)
	{
		*a = (*a + *data++) % 65521;
		*b = (*b + *a) % 65521;
		len--;
	}
}

int png_rgba_create(int w, int h, uint8_t* buf, uint8_t **out, size_t * size)
{
	uint8_t hdr[] = {
		0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
		0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x08, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00 };
	uint8_t ihdr[] = {
		0x00, 0x00, 0x00, 0x00, 0x49, 0x44, 0x41, 0x54 };
	uint8_t iend[] = { 
		0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 
		0xAE, 0x42, 0x60, 0x82 };

	size_t step = (65535 - 20) / (w * 4 + 9);
	uint8_t* start, *end, *adl,*p,*tmp;
	int y, i, ma;
	int lrow = (w * 4 + 1);
	int a = 1;
	int b = 0;

	if ((!buf) || (!out) || (!size))
		return -1;
	tmp = (uint8_t *)malloc(w * h * 4 + h * 20 + 1024);
	if (!tmp)
		return -1;
	p = tmp + sizeof(hdr);

	w_bytes(tmp, hdr, sizeof(hdr));
	w_beu32(tmp + 16, w);
	w_beu32(tmp + 20, h);
	w_beu32(tmp + 29, crc(tmp + 12, 17));
	for (y = 0; y < h; y += step)
	{
		ma = y + step;

		if (ma > h - 1)
			ma = h;
		start = p + 4;
		p = w_bytes(p, ihdr, sizeof(ihdr));
		if (y == 0)
		{
			*p++ = 0x78;
			*p++ = 0x9c;
		}

		for (i = y; i < ma; i++)
		{
			*p++ = i == h - 1 ? 0x01 : 0x00;
			p = w_zlen(p, lrow);
			adl = p;
			*p++ = 0;
			p = w_bytes(p, buf + i * w * 4, w * 4);
			adler32(&a, &b, adl, p - adl);
		}
		if (ma == h)
			p = w_beu32(p, (b << 16) | a);
		end = p;
		p = w_beu32(p, crc(start, end - start));
		w_beu32(start - 4, end - start - 4);
	}
	
	p = w_bytes(p, iend, sizeof(iend));
	*out = tmp;
	*size = p - tmp;
	return 0;
}

void png_rgba_free(uint8_t* out)
{
	if (out)
		free(out);
}

int png_rgba_save(int w, int h, uint8_t* buf, const char * name)
{
	uint8_t * out = NULL;
	size_t size = 0;
	FILE * fp = NULL;
	int ret;

	ret = png_rgba_create(w, h, buf, &out, &size);
	if (ret != 0)
		return -1;
	fp = fopen(name, "wb+");
	ret = (fp && (size == fwrite(out, 1, size, fp)));
	png_rgba_free(out);
	return ret ? 0 : -1;
}

 
