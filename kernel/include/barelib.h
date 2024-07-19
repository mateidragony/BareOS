/*
 *  This header contains typedefs for common utility types and functions
 *  used by the kernel.
 */

#ifndef H_BARELIB
#define H_BARELIB

#define interrupt void __attribute__ ((interrupt ("machine")))

#define NULL 0x0

#define va_copy(dst, src)       __builtin_va_copy(dst, src)    /*                                      */
#define va_start(last, va)      __builtin_va_start(last, va)   /*  GCC compiler builtins for variable  */
#define va_arg(va, type)        __builtin_va_arg(va, type)     /*  length arguments                    */
#define va_end(va)              __builtin_va_end(va)           /*                                      */

typedef unsigned char     byte;       /*                                            */
typedef int               int32;      /*  Defines some clearer aliases for common   */
typedef short             int16;      /*  data types to make bit-width unambiguous  */
typedef unsigned int      uint32;     /*                                            */
typedef unsigned short    uint16;     /*                                            */
typedef long int          int64;      /*                                            */
typedef unsigned long int uint64;     /*                                            */
typedef __builtin_va_list va_list;    /*                                            */

#define EXIT_SUCCESS ((byte)0)
#define EXIT_FAILURE ((byte)1)

typedef unsigned char   bool;
typedef unsigned long   size_t;

#define true ((bool)1)
#define false ((bool)0)

#define DEBUG_MALLOC 1
#define DEBUG_TRIE   2
#define DEBUG_SLEEP  4
#define DEBUG_FILE   8

#define RUNNING_AG_TESTS 1

int32 raise_syscall(uint32);          /*  Ask the operating system to run a low level system function  */

void uart_init(void);

extern uint32 boot_complete;

void* memset(void* s, int c, int n);
void* memcpy(void* dst, const void* src, int n);

#endif // barelib.h