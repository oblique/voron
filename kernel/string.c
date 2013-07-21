#include <inttypes.h>
#include <string.h>

void *
memset(void *s, int c, size_t n)
{
	unsigned char *us = s;

	while (n) {
		*us = c;
		us++;
		n--;
	}

	return s;
}

void *
memcpy(void *s1, const void *s2, size_t n)
{
	unsigned char *us1 = s1;
	const unsigned char *us2 = s2;

	while (n) {
		*us1 = *us2;
		us1++;
		us2++;
		n--;
	}

	return s1;
}

void *
memmove(void *dest, const void *src, size_t count)
{
	char *tmp;
	const char *s;

	if (dest <= src) {
		tmp = dest;
		s = src;
		while (count--)
			*tmp++ = *s++;
	} else {
		tmp = dest;
		tmp += count;
		s = src;
		s += count;
		while (count--)
			*--tmp = *--s;
	}
	return dest;
}

int
memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *us1, *us2;

	us1 = s1;
	us2 = s2;

	while (n) {
		if (*us1 != *us2)
			return (*us1 - *us2 < 0) ? -1 : 1;
		us1++;
		us2++;
		n--;
	}

	return 0;
}
