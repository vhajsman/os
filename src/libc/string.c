#include "string.h"
#include "types.h"
#include "memory/memory.h"

void *memset(void *dst, char c, u32 n) {
    char *temp = dst;
    for (; n != 0; n--) 
        *temp++ = c;

    return dst;
}

void *memcpy(void *dst, const void *src, u32 n) {
    char *ret = dst;
    char *p = dst;
    const char *q = src;

    while (n--)
        *p++ = *q++;
        
    return ret;
}

int memcmp(u8 *s1, u8 *s2, u32 n) {
    while (n--) {
        if (*s1 != *s2)
            return 0;

        s1++;
        s2++;
    }

    return 1;
}

char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == (char) c)
            return (char*) str;

        str++;
    }

    return NULL;
}


int strlen(const char *s) {
    int len = 0;
    while (*s++)
        len++;

    return len;
}

int strcmp(const char *s1, char *s2) {
    int i = 0;

    while ((s1[i] == s2[i])) {
        if (s2[i++] == 0)
            return 0;
    }

    return 1;
}

int strncmp(const char *s1, const char *s2, int c) {
    int result = 0;

    while (c) {
        result = *s1 - *s2++;

        if ((result != 0) || (*s1++ == 0))
            break;
        
        c--;
    }

    return result;
}

int strcpy(char *dst, const char *src) {
    int i = 0;
    while ((*dst++ = *src++) != 0)
        i++;

    return i;
}

int strncpy(char* dst, const char* src, int N) {
    int i = 0;
    while((*dst++ = *src++) != 0 && i < N)
        i++;

    return i;
}

void strcat(char *dest, const char *src) {
    char *end = (char *)dest + strlen(dest);

    memcpy((void *)end, (void *)src, strlen(src));

    end = end + strlen(src);
    *end = '\0';
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

int isspace(char c) {
    return  c == ' '  || 
            c == '\t' || 
            c == '\n' || 
            c == '\v' || 
            c == '\f' || 
            c == '\r';
}

int isalpha(char c) {
    return (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')));
}

char upper(char c) {
    if ((c >= 'a') && (c <= 'z'))
        return (c - 32);

    return c;
}

char lower(char c) {
    if ((c >= 'A') && (c <= 'Z'))
        return (c + 32);

    return c;
}

void itoa(char *buf, int base, int d) {
    char *p = buf;
    char *p1, *p2;

    unsigned long ud = d;
    int divisor = 10;

    
    if (base == 'd' && d < 0) {
        *p++ = '-';
        buf++;
        ud = -d;
    } else if (base == 'x') {
        divisor = 16;
    }

    do {
        int remainder = ud % divisor;
        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    } while (ud /= divisor);



    /* Terminate BUF. */
    *p = 0;


    /* Reverse BUF. */
    p1 = buf;
    p2 = p - 1;

    while (p1 < p2) {
        char tmp = *p1;

        *p1 = *p2;
        *p2 = tmp;
        
        p1++;
        p2--;
    }
}

char *strstr(const char *in, const char *str) {
    char c = *str++;
    u32 len;

    if (!c)
        return (char *)in;

    len = strlen(str);

    do {
        char sc;

        do {
            sc = *in++;

            if (!sc)
                return (char*) 0;
        } while (sc != c);
    } while (strncmp(in, str, len) != 0);

    return (char*) (in - 1);
}

/*
int snprintf(char* str, size_t size, const char* fmt, int num, const char* strarg) {
    int w = 0;
    int r = 0;

    size_t r = size;


    while(*fmt != '\0') {
        if(*fmt == '%' && *(fmt + 1) != '\0') {
            fmt ++;

            switch(*fmt) {
                case 'd':
                    int digits = 0;
                    int n = num;

                    while(n != 0) {
                        n /= 10;
                        digits ++;
                    }

                    if(num < 0)
                        digits ++;

                    if(w + digits < size) {
                        char temp[12];
                        int i = digits;

                        if(num < 0) {
                            temp[0] = '-';
                            num = -num;
                        }

                        while(i > 0) {
                            temp[i - 1] = num % 10 + '0';
                            num /= 10;

                            i --;
                        }

                        for(int j = 0; j < digits; j ++) 
                            str[w + j] = temp[j];

                        
                        w += digits;
                        r -= digits;
                    } else {
                        // Insufficient space in the buffer
                        break;
                    }

                    break;
                
                case 's':
                    int len = 0;
                    const char* s = strarg;

                    while(*s != '\0') {
                        len ++;
                        s ++;
                    }

                    if(w + len < size) {
                        for(int i = 0; i < len; i ++) 
                            str[w + i] = strarg[i];
                        
                        w += len;
                        r -= len;
                    }

                    break;
                

                default:
                    break;
            }
        } else {
            if(r > 1) {
                *str ++ = *fmt;
                w ++;
                r --;
            } else {
                break;
            }
        }

        fmt ++;
    }

    if(r > 0) {
        *str = '\0';
    } else {
        str[size - 1] = '\0';
    }

    return w;
}
*/

char* strtok(char* str, char* delm) {
    static char* current = NULL;

    if (str != NULL)
        current = str;
    if (current == NULL)
        return NULL;

    while (*current && strchr(delm, *current)) 
        current++;

    if (*current == '\0') {
        current = NULL;
        return NULL;
    }

    char* start = current;

    while (*current && !strchr(delm, *current))
        current++;

    if (*current) {
        *current = '\0';
        current++;
    }

    return start;

// It:
//     W[i] = 0;
//     current = i + 1;
//     
//     return W;
}

char* strdup(const char* source) {
    if (source == NULL)
        return NULL;

    int len = strlen(source) + 1;
    char* dest = (char*) kmalloc(len);

    if (dest == NULL)
        return NULL;

    memcpy(dest, source, len);
    return dest;
}

char* strsep(char** stringp, const char* delim) {
    int c, sc;
    char *s, *token;
    const char *spanp;

    if((s = *stringp) == NULL)
        return (NULL);

    for(token = s; ;) {
        c = *s++;
        spanp = delim;

        do {
            if((sc = *spanp++) == c) {
                if(c == 0) {
                    s = NULL;
                } else {
                    s[-1] = 0;
                }

                *stringp = s;

                return token;
            }
        } while(sc != 0);
    }
}

linkedlist_t* string_split(const char* src, const char* delim, unsigned int* tokenCount) {
    linkedlist_t* ret_list = linkedlist_create();

    char* s = strdup(src);
    char *token, *rest = s;

    while((token = strsep(&rest, delim)) != NULL) {
        if(!strcmp(token, "."))
            continue;
        
        if(!strcmp(token, "..")) {
            if(linkedlist_size(ret_list) > 0)
                linkedlist_pop(ret_list);
            
            continue;
        }

        linkedlist_push(ret_list, strdup(token));

        if(tokenCount)
            (*tokenCount)++;
    }

    free(s);

    return ret_list;
}

char* linkedlist_toString(linkedlist_t* list, const char* delim) {
    int len = 0, ret_len = 256;
    char* ret = kmalloc(256);
    
    memset(ret, 0, 256);

    while(linkedlist_size(list) > 0) {
        char* temp = linkedlist_pop(list)->val;
        int temp_len = strlen(temp);

        if(len + temp_len /*+ 1 + 1*/ + 2 > ret_len) {
            ret_len = ret_len * 2;
            ret = krealloc(ret, ret_len);
            len = len + temp_len + 1;
        }

        strcat(ret, delim);
        strcat(ret, temp);
    }

    return ret;
}

int atoi(const char *str) {
    if (str == NULL)
        return 0;

    int result = 0;
    int sign = 1;
    size_t i = 0;

    while (str[i] == ' '  || str[i] == '\t' || str[i] == '\n' || 
           str[i] == '\r' || str[i] == '\v' || str[i] == '\f') {
            i++;
    }

    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }

    while (is_digit(str[i])) {
        int digit = str[i] - '0';

        result = result * 10 + digit;
        i++;
    }

    return sign * result;
}
