#include "sbs.h"

#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#define SBS_NULL_TERMINATE(s) (s).str[(s).len] = '\0'

sbs sbsnewlen(const void *init, size_t initlen, char buffer[], size_t buffer_size)
{
    sbs s;
    s.str = buffer;
    s.size = buffer_size;
    s.len = initlen;
    memcpy(s.str, init, initlen);
    SBS_NULL_TERMINATE(s);
    return s;
}

sbs sbsnew(const char *init, char buffer[], size_t buffer_size)
{
    return sbsnewlen(init, strlen(init), buffer, buffer_size);
}

sbs sbsempty(char *buffer, size_t buffer_size)
{
    return sbsnewlen("", 0, buffer, buffer_size);
}

sbs sbsdup(const sbs *s, char buffer[], size_t buffer_size)
{
    return sbsnewlen(s->str, s->len, buffer, buffer_size);
}

int sbsmove(sbs *s, char buffer[], size_t buffer_size)
{
    if (buffer_size <= s->len)
    {
        return -1;
    }
    memcpy(buffer, s->str, s->len);
    s->size = buffer_size;
    s->str = buffer;
    SBS_NULL_TERMINATE(*s);
    return 0;
}

void sbsupdatelen(sbs *s)
{
    s->len = strlen(s->str);
}

void sbsclear(sbs *s)
{
    s->len = 0;
    SBS_NULL_TERMINATE(*s);
}

int sbscatlen(sbs *s, const void *t, size_t len)
{
    if (len >= sbssizerem(*s))
    {
        return -1;
    }
    memcpy(sbsstrend(*s), t, len);
    s->len += len;
    SBS_NULL_TERMINATE(*s);
    return 0;
}

int sbscat(sbs *s, const char *t)
{
    return sbscatlen(s, t, strlen(t));
}

int sbscatsbs(sbs *s, const sbs *t)
{
    return sbscatlen(s, t->str, t->len);
}

int sbscpylen(sbs *s, const char *t, size_t len)
{
    if (len >= s->size)
    {
        return -1;
    }
    s->len = len;
    memcpy(s->str, t, len);
    SBS_NULL_TERMINATE(*s);
    return 0;
}

int sbscpy(sbs *s, const char *t)
{
    return sbscpylen(s, t, strlen(t));
}

/* Like sdscatprintf() but gets va_list instead of being variadic. */
int sbscatbufvprintf(sbs *s, char buffer[], size_t buffer_size, const char *fmt, va_list ap)
{
    va_list cpy;
    char *buf = buffer;
    size_t buflen = buffer_size;

    buf[buflen - 2] = '\0';
    va_copy(cpy, ap);
    vsnprintf(buf, buflen, fmt, cpy);
    va_end(cpy);
    if (buf[buflen - 2] != '\0')
    {
        return -1;
    }
    /* Finally concat the obtained string to the SDS string and return it. */
    return sbscat(s, buf);
}

/* Like sdscatprintf() but gets va_list instead of being variadic. */
int sbscatvprintf(sbs *s, const char *fmt, va_list ap)
{
    char staticbuf[SBS_PRINTF_FMT_SIZE];
    return sbscatbufvprintf(s, staticbuf, SBS_PRINTF_FMT_SIZE, fmt, ap);
}

int sbscatprintf(sbs *s, const char *fmt, ...)
{
    va_list ap;
    int t;
    va_start(ap, fmt);
    t = sbscatvprintf(s, fmt, ap);
    va_end(ap);
    return t;
}

int sbscatbufprintf(sbs *s, char buffer[], size_t buffer_size, const char *fmt, ...)
{
    va_list ap;
    int t;
    va_start(ap, fmt);
    t = sbscatbufvprintf(s, buffer, buffer_size, fmt, ap);
    va_end(ap);
    return t;
}

void sbstrim(sbs *s, const char *cset)
{
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = s->str;
    ep = end = s->str + sbslen(*s) - 1;
    while (sp <= end && strchr(cset, *sp))
        sp++;
    while (ep > sp && strchr(cset, *ep))
        ep--;
    len = (sp > ep) ? 0 : ((ep - sp) + 1);
    if (s->str != sp)
        memmove(s, sp, len);
    s->len = len;
    SBS_NULL_TERMINATE(*s);
}

void sbsrange(sbs *s, ssize_t start, ssize_t end)
{
    size_t newlen, len = sbslen(*s);

    if (len == 0)
        return;
    if (start < 0)
    {
        start = len + start;
        if (start < 0)
            start = 0;
    }
    if (end < 0)
    {
        end = len + end;
        if (end < 0)
            end = 0;
    }
    newlen = (start > end) ? 0 : (end - start) + 1;
    if (newlen != 0)
    {
        if (start >= (ssize_t)len)
        {
            newlen = 0;
        }
        else if (end >= (ssize_t)len)
        {
            end = len - 1;
            newlen = (start > end) ? 0 : (end - start) + 1;
        }
    }
    else
    {
        start = 0;
    }
    if (start && newlen)
        memmove(s->str, s->str + start, newlen);
    s->len = newlen;
    SBS_NULL_TERMINATE(*s);
}

/* Apply tolower() to every character of the sds string 's'. */
void sbstolower(sbs *s)
{
    for (size_t j = 0; j < sbslen(*s); j++)
        s->str[j] = tolower(s->str[j]);
}

/* Apply tolower() to every character of the sds string 's'. */
void sbstoupper(sbs *s)
{
    for (size_t j = 0; j < sbslen(*s); j++)
        s->str[j] = toupper(s->str[j]);
}

int sbscmp(const sbs *s1, const sbs *s2)
{
    size_t l1, l2, minlen;
    int cmp;

    l1 = sbslen(*s1);
    l2 = sbslen(*s2);
    minlen = (l1 < l2) ? l1 : l2;
    cmp = memcmp(s1->str, s2->str, minlen);
    if (cmp == 0)
        return l1 > l2 ? 1 : (l1 < l2 ? -1 : 0);
    return cmp;
}

/* Modify the string substituting all the occurrences of the set of
 * characters specified in the 'from' string to the corresponding character
 * in the 'to' array.
 *
 * For instance: sdsmapchars(mystring, "ho", "01", 2)
 * will have the effect of turning the string "hello" into "0ell1".
 *
 * The function returns the sds string pointer, that is always the same
 * as the input pointer since no resize is needed. */
void sbsmapchars(sbs *s, const char *from, const char *to, size_t setlen)
{
    size_t j, i, l = sbslen(*s);

    for (j = 0; j < l; j++)
    {
        for (i = 0; i < setlen; i++)
        {
            if (s->str[j] == from[i])
            {
                s->str[j] = to[i];
                break;
            }
        }
    }
}

/* Helper for sdscatlonglong() doing the actual number -> string
 * conversion. 's' must point to a string with room for at least
 * SBS_LLSTR_SIZE bytes.
 *
 * The function returns the length of the null-terminated string
 * representation stored at 's'. */
#define SBS_LLSTR_SIZE 21
static int sbsll2str(char *s, long long value)
{
    char *p, aux;
    unsigned long long v;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    v = (value < 0) ? -value : value;
    p = s;
    do
    {
        *p++ = '0' + (v % 10);
        v /= 10;
    } while (v);
    if (value < 0)
        *p++ = '-';

    /* Compute length and add null term. */
    l = p - s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while (s < p)
    {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/* Identical sbsll2str(), but for unsigned long long type. */
static int sbsull2str(char *s, unsigned long long v)
{
    char *p, aux;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    p = s;
    do
    {
        *p++ = '0' + (v % 10);
        v /= 10;
    } while (v);

    /* Compute length and add null term. */
    l = p - s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while (s < p)
    {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/* Create an sds string from a long long value. It is much faster than:
 *
 * sdscatprintf(sdsempty(),"%lld\n", value);
 */
int sbsfromlonglong(sbs *s, long long value)
{
    char buf[SBS_LLSTR_SIZE];
    int len = sbsll2str(buf, value);
    return sbscpylen(s, buf, len);
}

/* Join an array of C strings using the specified separator (also a C string).
 * Returns the result as an sds string. */
int sbsjoin(sbs *s, char **argv, int argc, const char *sep)
{
    int j;

    for (j = 0; j < argc; j++)
    {
        int err = sbscat(s, argv[j]);
        if (err != 0)
        {
            return err;
        }
        if (j != argc - 1)
        {
            err = sbscat(s, sep);
            if (err != 0)
            {
                return err;
            }
        }
    }
    return 0;
}

int sbsjoinsbs(sbs *s, sbs argv[], int argc, const char *sep, size_t seplen)
{
    int j;

    for (j = 0; j < argc; j++)
    {
        int err = sbscatsbs(s, &argv[j]);
        if (err != 0)
        {
            return err;
        }
        if (j != argc - 1)
        {
            err = sbscatlen(s, sep, seplen);
            if (err != 0)
            {
                return err;
            }
        }
    }
    return 0;
}