#ifndef __STRING__H
#define __STRING__H

#include <stddef.h>  // For size_t
#include <stdint.h>

// Function prototypes
size_t strlen(const char *str);
char *strstr(const char *haystack, const char *needle);
int strcmp(const char *str_1, const char *str_2);
char *strcpy(char *dest, const char *src);
char *strncpy(char * restrict s1, const char * restrict s2, size_t n);
char *strcat(char *dest, const char *src);
int strncmp(const char *s1, const char *s2, register size_t n);

void *memcpy(void *dest, const void *src, size_t n) ;
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

#endif // __STRING__H
