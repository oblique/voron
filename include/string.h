#ifndef __STRING_H
#define __STRING_H

#include <inttypes.h>

void *memset(void *s, int c, size_t n);
void *memcpy(void *s1, const void *s2, size_t n);
void *memmove(void *dest, const void *src, size_t count);
int memcmp(const void *s1, const void *s2, size_t n);

#endif	/* __STRING_H */
