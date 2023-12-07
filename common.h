/*
 * Common functions
 */
#ifndef _COMMON_H
#define _COMMON_H
#include <string.h>

#define max(a,b) (a>=b?a:b)
#define min(a,b) (a<=b?a:b)

//Math part
unsigned long long simple_fact(unsigned char i)
{
    if (i <= 1)
        return 1;
    else
        return i * simple_fact(i - 1);
}
long long part_fact(int m, int n)
{
    long long o = 1;
    for (int i = m; i <= n; o *= i++);
    return o;
}
unsigned long long combine(unsigned int n, unsigned int m)
{
    if (m > n)
        return 0;
    if (m + m > n)
        m = n - m;
    return part_fact(n - m + 1, n) / part_fact(1, m);
}
_Bool is_prime(unsigned long long n)
{
    unsigned char r;
    unsigned long i;
    switch (n)
    {
    case 0:case 1:case 4:return 0;
    case 2:case 3:case 5:return 1;
    default:
        r = n % 6;
        if (r != 1 && r != 5)return 0;
        for (i = 5; (unsigned long long)i * i <= n; i += 6)
        {
            if (n % i == 0 || n % (i + 2) == 0)
                return 0;
        }
        return 1;
    }
}
unsigned long long fibonacci(unsigned short n)
{
    unsigned long long a = 0, b = 1, c = 1;
    for (unsigned short i = 2; i <= n; i++)
    {
        c = a + b;
        a = b;
        b = c;
    }
    return c;
}
long long int_power(long long m, unsigned char n)
{
    long long result = 1;
    while (n != 0)
    {
        if (n & 1)result *= m;
        m *= m;
        n >>= 1;
    }
    return result;
}
unsigned char int_digits(long long x)
{
    unsigned char d = 0;
    while (x != 0)
    {
        x /= 10;
        d++;
    }
    return d;
}
int int_reverse(int number)
{
    int n = 0;
    while (number)
    {
        n = n * 10 + number % 10;
        number /= 10;
    }
    return n;
}
//Common
void int_insert_sort(int* a, int n)
{
    for (int* i = a + 1, *j, x; i - a < n; i++)
    {
        if (*i < *(i - 1))
        {
            for (j = i, x = *i; j > a && x < *(j - 1); j--)
                *j = *(j - 1);
            *j = x;
        }
    }
}
char* strrev(char* str)
{
    size_t i = strlen(str);
    char t, * j;
    for (j = str; j - str < i / 2; j++)
        t = *j, * j = *((str + i) - (j - str) - 1), * ((str + i) - (j - str) - 1) = t;
    return str;
}
ptrdiff_t findstr(char* str, char* x)
{
    size_t i = strlen(str), n = strlen(x);
    char t, * j;
    _Bool f;
    if (i < n)
        return -1;
    else if (!n)
        return 0;
    for (j = str; j - str <= i - n; j++)
    {
        t = *(j + n), * (j + n) = 0;
        f = !strcmp(j, x);
        *(j + n) = t;
        if (f)
            return j - str;
    }
    return -1;
}
/*
char* strrep(char* str, char* x, char* y)
{
    unsigned i = strlen(str), n = strlen(x), m;
    char t, * j;
    for (j = str; j - str < i; j++)
        if ((m = findstr(str, x)) >= 0)
        {
            t = *(str + m + n);
            strcpy(str + m, y);
            *(str + m + n) = t;
            strrep(str + m + n, x, y);
        }
    return str;
}
*/
char* strfilter(char* p, char* c, _Bool w)
{
    char* i = p, * j = p, * n; //i为当前检测的字符位置，j为当前写入的字符位置
    _Bool f;
    while (*i)
    {
        f = !w;
        for (n = c; n - c < strlen(c); n++)
        {
            if (*i == *n)
            {
                f = w;
                break;
            }
        }
        if (f)
            *j++ = *i;
        i++;
    }
    *j = 0;
    return p;
}
char* strzip(char *p)
{
    char* i = p, * j = p; //i为当前检测的字符位置，j为当前写入的字符位置
    while (*i)
    {
        if (*i != *(i + 1))
            *++j = *(i + 1);
        i++;
    }
    return p;
}

#endif