#ifndef __INTTYPES_H
#define __INTTYPES_H

typedef signed char		s8;
typedef unsigned char		u8;
typedef short			s16;
typedef unsigned short 	u16;
typedef int			s32;
typedef unsigned int 		u32;
typedef long long		s64;
typedef unsigned long long	u64;

typedef s8	int8_t;
typedef u8	uint8_t;
typedef s16	int16_t;
typedef u16	uint16_t;
typedef s32	int32_t;
typedef u32	uint32_t;
typedef s64	int64_t;
typedef u64	uint64_t;

typedef unsigned int	uint_t;
typedef unsigned long	ulong_t;
typedef unsigned int	size_t;
typedef int	 	ssize_t;
typedef unsigned long	uintptr_t;
typedef long		intptr_t;

#define S8_C(x)  x
#define U8_C(x)  x ## U
#define S16_C(x) x
#define U16_C(x) x ## U
#define S32_C(x) x
#define U32_C(x) x ## U
#define S64_C(x) x ## LL
#define U64_C(x) x ## ULL


#endif	/* __INTTYPES_H */
