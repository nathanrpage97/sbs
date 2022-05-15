#include "sbs.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define SBS_NULL_TERMINATE(s) (s)->str[(s)->len] = '\0'

#define SBS_CHECK(func)           \
    do {                          \
        int err = (func);         \
        if (err != 0) return err; \
    } while (0);

// create a shallow copy to allow updates
static void sbscpyshl(sbs *src, sbs *dst) {
    dst->size = src->size;
    dst->len = src->len;
    dst->str = src->str;
}

sbs *sbsnewlen(sbs *s, const void *init, size_t initlen, char buffer[],
               size_t buffer_size) {
    if (initlen >= buffer_size) {
        return NULL;
    }
    s->str = buffer;
    s->size = buffer_size;
    s->len = initlen;
    memcpy(s->str, init, initlen);
    SBS_NULL_TERMINATE(s);
    return s;
}

sbs *sbsnew(sbs *s, const char *init, char buffer[], size_t buffer_size) {
    return sbsnewlen(s, init, strlen(init), buffer, buffer_size);
}

sbs *sbsempty(sbs *s, char *buffer, size_t buffer_size) {
    // this will never fail
    sbsnewlen(s, "", 0, buffer, buffer_size);
    return s;
}

sbs *sbsdup(sbs *s, sbs *d, char buffer[], size_t buffer_size) {
    return sbsnewlen(d, s->str, s->len, buffer, buffer_size);
}

int sbsresize(sbs *s, char buffer[], size_t buffer_size) {
    if (buffer_size <= s->len) {
        return -1;
    }
    memcpy(buffer, s->str, s->len);
    s->size = buffer_size;
    s->str = buffer;
    SBS_NULL_TERMINATE(s);
    return 0;
}

void sbsupdatelen(sbs *s) { s->len = strlen(s->str); }

void sbsclear(sbs *s) {
    s->len = 0;
    SBS_NULL_TERMINATE(s);
}

int sbscatlen(sbs *s, const void *t, size_t len) {
    if (len >= sbsavail(s)) {
        return -1;
    }
    memcpy(sbsend(s), t, len);
    s->len += len;
    SBS_NULL_TERMINATE(s);
    return 0;
}

int sbscat(sbs *s, const char *t) { return sbscatlen(s, t, strlen(t)); }

int sbscatsbs(sbs *s, const sbs *t) { return sbscatlen(s, t->str, t->len); }

int sbscpylen(sbs *s, const char *t, size_t len) {
    if (len >= s->size) {
        return -1;
    }
    s->len = len;
    memcpy(s->str, t, len);
    SBS_NULL_TERMINATE(s);
    return 0;
}

int sbscpy(sbs *s, const char *t) { return sbscpylen(s, t, strlen(t)); }

/* Like sbscatprintf() but gets va_list instead of being variadic. */
int sbscatvprintf(sbs *s, const char *fmt, va_list ap) {
    size_t bufsize = sbsavail(s);
    int n = vsnprintf(sbsend(s), bufsize, fmt, ap);
    if (n >= bufsize) {
        SBS_NULL_TERMINATE(s);
        return -1;
    }
    s->len += n;
    SBS_NULL_TERMINATE(s);
    return 0;
}

int sbscatprintf(sbs *s, const char *fmt, ...) {
    va_list ap;
    int t;
    va_start(ap, fmt);
    t = sbscatvprintf(s, fmt, ap);
    va_end(ap);
    return t;
}

void sbstrim(sbs *s, const char *cset) {
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = s->str;
    ep = end = s->str + sbslen(s) - 1;
    while (sp <= end && strchr(cset, *sp)) sp++;
    while (ep > sp && strchr(cset, *ep)) ep--;
    len = (sp > ep) ? 0 : ((ep - sp) + 1);
    if (s->str != sp) memmove(s, sp, len);
    s->len = len;
    SBS_NULL_TERMINATE(s);
}

void sbsrange(sbs *s, ssize_t start, ssize_t end) {
    size_t newlen, len = sbslen(s);

    if (len == 0) return;
    if (start < 0) {
        start = len + start;
        if (start < 0) start = 0;
    }
    if (end < 0) {
        end = len + end;
        if (end < 0) end = 0;
    }
    newlen = (start > end) ? 0 : (end - start) + 1;
    if (newlen != 0) {
        if (start >= (ssize_t)len) {
            newlen = 0;
        } else if (end >= (ssize_t)len) {
            end = len - 1;
            newlen = (start > end) ? 0 : (end - start) + 1;
        }
    } else {
        start = 0;
    }
    if (start && newlen) memmove(s->str, s->str + start, newlen);
    s->len = newlen;
    SBS_NULL_TERMINATE(s);
}

/* Apply tolower() to every character of the sbs string 's'. */
void sbstolower(sbs *s) {
    for (size_t j = 0; j < sbslen(s); j++) s->str[j] = tolower(s->str[j]);
}

/* Apply tolower() to every character of the sbs string 's'. */
void sbstoupper(sbs *s) {
    for (size_t j = 0; j < sbslen(s); j++) s->str[j] = toupper(s->str[j]);
}

int sbscmp(const sbs *s1, const sbs *s2) {
    size_t l1, l2, minlen;
    int cmp;

    l1 = sbslen(s1);
    l2 = sbslen(s2);
    minlen = (l1 < l2) ? l1 : l2;
    cmp = memcmp(s1->str, s2->str, minlen);
    if (cmp == 0) return l1 > l2 ? 1 : (l1 < l2 ? -1 : 0);
    return cmp;
}

/* Modify the string substituting all the occurrences of the set of
 * characters specified in the 'from' string to the corresponding character
 * in the 'to' array.
 *
 * For instance: sbsmapchars(mystring, "ho", "01", 2)
 * will have the effect of turning the string "hello" into "0ell1".
 *
 * The function returns the sbs string pointer, that is always the same
 * as the input pointer since no resize is needed. */
void sbsmapchars(sbs *s, const char *from, const char *to, size_t setlen) {
    size_t j, i, l = sbslen(s);

    for (j = 0; j < l; j++) {
        for (i = 0; i < setlen; i++) {
            if (s->str[j] == from[i]) {
                s->str[j] = to[i];
                break;
            }
        }
    }
}

/* Helper for sbscatlonglong() doing the actual number -> string
 * conversion. 's' must point to a string with room for at least
 * SBS_LLSTR_SIZE bytes.
 *
 * The function returns the length of the null-terminated string
 * representation stored at 's'. */
#define SBS_LLSTR_SIZE 21
static int sbsll2str(char *s, long long value) {
    char *p, aux;
    unsigned long long v;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    v = (value < 0) ? -value : value;
    p = s;
    do {
        *p++ = '0' + (v % 10);
        v /= 10;
    } while (v);
    if (value < 0) *p++ = '-';

    /* Compute length and add null term. */
    l = p - s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while (s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/* Identical sbsll2str(), but for unsigned long long type. */
static int sbsull2str(char *s, unsigned long long v) {
    char *p, aux;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    p = s;
    do {
        *p++ = '0' + (v % 10);
        v /= 10;
    } while (v);

    /* Compute length and add null term. */
    l = p - s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while (s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/* Create an sbs string from a long long value. It is much faster than:
 *
 * sbscatprintf(sbsempty(),"%lld\n", value);
 */
int sbsfromlonglong(sbs *s, long long value) {
    char buf[SBS_LLSTR_SIZE];
    int len = sbsll2str(buf, value);
    return sbscpylen(s, buf, len);
}

/* Join an array of C strings using the specified separator (also a C string).
 * Returns the result as an sbs string. */
int sbsjoin(sbs *s, const char **argv, int argc, const char *sep) {
    int j;
    sbs buf;
    sbscpyshl(s, &buf);

    for (j = 0; j < argc; j++) {
        SBS_CHECK(sbscat(&buf, argv[j]));
        if (j != argc - 1) {
            SBS_CHECK(sbscat(&buf, sep));
        }
    }
    sbscpyshl(&buf, s);
    return 0;
}

int sbsjoinsbs(sbs *s, const sbs argv[], int argc, const char *sep,
               size_t seplen) {
    int j;
    sbs buf;
    sbscpyshl(s, &buf);

    for (j = 0; j < argc; j++) {
        SBS_CHECK(sbscatsbs(&buf, &argv[j]));
        if (j != argc - 1) {
            SBS_CHECK(sbscatlen(&buf, sep, seplen));
        }
    }
    sbscpyshl(&buf, s);
    return 0;
}

static int sbscatvfmt(sbs *s, char const *fmt, va_list ap) {
    sbs buf_base;
    sbscpyshl(s, &buf_base);
    sbs *buf = &buf_base;
    const char *f = fmt;

    f = fmt; /* Next format specifier byte to process. */
    while (*f) {
        char next, *str;
        size_t l;
        long long num;
        unsigned long long unum;

        switch (*f) {
            case '%':
                next = *(f + 1);
                f++;
                switch (next) {
                    case 's': {
                        str = va_arg(ap, char *);
                        SBS_CHECK(sbscat(buf, str));
                        break;
                    }
                    case 'S': {
                        sbs *tmp_sbs = va_arg(ap, sbs *);
                        SBS_CHECK(sbscatsbs(buf, tmp_sbs));
                        break;
                    }
                    case 'i':
                    case 'I':
                        if (next == 'i')
                            num = va_arg(ap, int);
                        else
                            num = va_arg(ap, long long);
                        {
                            char numbuf[SBS_LLSTR_SIZE];
                            l = sbsll2str(numbuf, num);
                            SBS_CHECK(sbscatlen(buf, numbuf, l));
                            break;
                        }
                        break;
                    case 'u':
                    case 'U':
                        if (next == 'u')
                            unum = va_arg(ap, unsigned int);
                        else
                            unum = va_arg(ap, unsigned long long);
                        {
                            char numbuf[SBS_LLSTR_SIZE];
                            l = sbsull2str(numbuf, unum);
                            SBS_CHECK(sbscatlen(buf, numbuf, l));
                            break;
                        }
                        break;
                    default: { /* Handle %% and generally %<unknown>. */
                        SBS_CHECK(sbscatlen(buf, &next, 1));
                        break;
                    }
                }
                break;
            default: {
                SBS_CHECK(sbscatlen(buf, f, 1));
                break;
            }
        }
        f++;
    }
    va_end(ap);
    sbscpyshl(buf, s);
    return 0;
}

int sbscatfmt(sbs *s, char const *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int err = sbscatvfmt(s, fmt, ap);
    va_end(ap);
    SBS_NULL_TERMINATE(s);  // if it fails cleanup
    return err;
}

int sbscatrepr(sbs *sb, const char *p, size_t len) {
    sbs sdata;
    sbs *s = &sdata;
    sbscpyshl(sb, s);
    SBS_CHECK(sbscatlen(s, "\"", 1));
    while (len--) {
        switch (*p) {
            case '\\':
            case '"':
                SBS_CHECK(sbscatprintf(s, "\\%c", *p));
                break;
            case '\n':
                SBS_CHECK(sbscatlen(s, "\\n", 2));
                break;
            case '\r':
                SBS_CHECK(sbscatlen(s, "\\r", 2));
                break;
            case '\t':
                SBS_CHECK(sbscatlen(s, "\\t", 2));
                break;
            case '\a':
                SBS_CHECK(sbscatlen(s, "\\a", 2));
                break;
            case '\b':
                SBS_CHECK(sbscatlen(s, "\\b", 2));
                break;
            default:
                if (isprint(*p)) {
                    SBS_CHECK(sbscatprintf(s, "%c", *p));
                } else {
                    SBS_CHECK(sbscatprintf(s, "\\x%02x", (unsigned char)*p));
                }
                break;
        }
        p++;
    }
    SBS_CHECK(sbscatlen(s, "\"", 1));
    sbscpyshl(s, sb);
    return 0;
}
