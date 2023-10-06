#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>

typedef int8_t  i8; 
typedef int16_t i16; 
typedef int32_t i32; 
typedef int64_t i64; 

typedef uint8_t  u8; 
typedef uint16_t u16; 
typedef uint32_t u32; 
typedef uint64_t u64; 

typedef float f32;
typedef double f64;

#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))
#define ASSERT(condition) if(!(condition)) { *(int *)0 = 0;} 

#define KB(value) ((value)*1024LL)
#define MB(value) (KB(value)*1024LL)
#define GB(value) (MB(value)*1024LL)
#define TB(value) (GB(value)*1024LL)

#define IS_POWER_OF_TWO(expr) (((expr) & (expr - 1)) == 0)

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MAX3(a, b, c) MAX(MAX(a, b), c)
#define MIN3(a, b, c) MIN(MIN(a, b), c)

#define ABS(a) ((a) >= 0 ? (a) : (-(a)))
#define CLAMP(value, min, max) MAX(MIN(value, max), min)

#endif
