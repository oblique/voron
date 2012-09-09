#ifndef __STRING_H
#define __STRING_H

static inline void *memset(void *s, int c, size_t n) {
	unsigned char *us = s;

	while (n) {
		*us = c;
		us++;
		n--;
	}

	return s;
}

static inline void *memcpy(void *s1, const void *s2, size_t n) {
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

static inline int memcmp(const void *s1, const void *s2, size_t n) {
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

#endif	/* __STRING_H */
