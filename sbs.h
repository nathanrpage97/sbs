#ifndef __SBS_H
#define __SBS_H

#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct {
    char *str;
    size_t len;
    size_t size;
} sbs;

// Properties
static inline char *sbsstr(sbs *s) { return s->str; }
static inline size_t sbslen(const sbs *s) { return s->len; }

static inline size_t sbssize(const sbs *s) { return s->len; }
static inline size_t sbssizerem(const sbs *s) { return s->size - s->len; }
static inline char *sbsstrend(const sbs *s) { return s->str + s->len; }

// creation
sbs *sbsnewlen(sbs *s, const void *init, size_t initlen, char buffer[],
               size_t buffer_size);
#define SBSNEWLEN(init, initlen, size) \
    sbsnewlen(&(sbs){}, init, initlen, (char[size]){0}, size)
sbs *sbsnew(sbs *s, const char *init, char buffer[], size_t buffer_size);
#define SBSNEW(init, size) sbsnew(&(sbs){0}, init, (char[size]){0}, size)
sbs *sbsempty(sbs *s, char buffer[], size_t buffer_size);
#define SBSEMPTY(size) sbsempty(&(sbs){0}, (char[size]){0}, size)

// fill
sbs *sbscatlen(sbs *s, const void *t, size_t len);
sbs *sbscat(sbs *s, const char *t);
sbs *sbscatsbs(sbs *s, const sbs *t);
sbs *sbscpylen(sbs *s, const char *t, size_t len);
sbs *sbscpy(sbs *s, const char *t);
sbs *sbscpysbs(const sbs *s, sbs *d);

// format
sbs *sbscatvprintf(sbs *s, const char *fmt, va_list ap);
#ifdef __GNUC__
sbs *sbscatprintf(sbs *s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
sbs *sbscatprintf(sbs *s, const char *fmt, ...);
#endif
sbs *sbscatfmt(sbs *s, char const *fmt, ...);

// modify
void sbstrim(sbs *s, const char *cset);
void sbsrange(sbs *s, ssize_t start, ssize_t end);
void sbsclear(sbs *s);
int sbscmp(const sbs *s1, const sbs *s2);
void sbstolower(sbs *s);
void sbstoupper(sbs *s);
sbs *sbsfromlonglong(sbs *s, long long value);
void sbsmapchars(sbs *s, const char *from, const char *to, size_t setlen);

// join
sbs *sbsjoin(sbs *s, char **argv, int argc, const char *sep);
sbs *sbsjoinsbs(sbs *s, sbs argv[], int argc, const char *sep, size_t seplen);

// escape hatch
sbs *sbsresize(sbs *s, char buffer[], size_t buffer_size);
#define SBSRESIZE(s, size) sbsresize(s, (char[size]){}, size)
static inline void sbssetlen(sbs *s, size_t len) {
    if (s->len >= s->size) return;
    s->len = len;
}
void sbsupdatelen(sbs *s);

#endif
