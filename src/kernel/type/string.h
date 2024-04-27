#ifndef __STRING_H
#define __STRING_H

#include "types.h"

#define STRINGIFICATE(X)    #X

int strlen(const char *s);

int strcmp(const char *s1, char *s2);
int strncmp(const char *s1, const char *s2, int c);
int strcpy(char *dst, const char *src);
void strcat(char *dest, const char *src);

void *memset(void *dst, char c, u32 n);
void *memcpy(void *dst, const void *src, u32 n);
int memcmp(u8 *s1, u8 *s2, u32 n);

int isspace(char c);
int isalpha(char c);
char upper(char c);
char lower(char c);

void itoa(char *buf, int base, int d);
char *strstr(const char *in, const char *str);

//int snprintf(char* str, size_t size, const char* fmt, int num, const char* strarg);

#endif