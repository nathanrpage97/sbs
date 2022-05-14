#ifndef __SBS_H
#define __SBS_H

#include <sys/types.h>
#include <stdarg.h>
#include <stdint.h>

typedef struct
{
    char *str;
    size_t len;
    size_t size;
} sbs;

sbs sbsnewlen(const void *init, size_t initlen, char buffer[], size_t buffer_size);
#define SBSNEWLEN(init, initlen, buffer) sbsnewlen(init, initlen, buffer, sizeof(buffer))
sbs sbsnew(const char *init, char buffer[], size_t buffer_size);
#define SBSNEW(init, buffer) sbsnew(s, init, buffer, sizeof(buffer))
sbs sbsempty(char *buffer, size_t buffer_size);
#define SBSEMPTY(buffer) sbsempty(buffer, sizeof(buffer))
sbs sbsdup(const sbs *s, char buffer[], size_t buffer_size);
#define SBSDUP(s, buffer) sbsdup(s, buffer, sizeof(buffer))

// sbs properties
static inline char *sbsstr(sbs s)
{
    return s.str;
}
static inline size_t sbslen(sbs s)
{
    return s.len;
}

static inline size_t sbssize(sbs s)
{
    return s.len;
}
static inline size_t sbssizerem(sbs s)
{
    return s.size - s.len;
}
static inline char *sbsstrend(sbs s)
{
    return s.str + s.len;
}

// void sdsfree(sds s);
// sds sdsgrowzero(sds s, size_t len);
int sbsmove(sbs *s, char buffer[], size_t buffer_size);
#define SBSMOVE(s, buffer) sbsmove(s, buffer, sizeof(buffer_size))
int sbscatlen(sbs *s, const void *t, size_t len);
int sbscat(sbs *s, const char *t);
int sbscatsbs(sbs *s, const sbs *t);
int sbscpylen(sbs *s, const char *t, size_t len);
int sbscpy(sbs *s, const char *t);

int sbscatvprintf(sbs *s, const char *fmt, va_list ap);
#ifdef __GNUC__
int sbscatprintf(sbs *s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
int sbscatprintf(sbs *s, const char *fmt, ...);
#endif

int sbscatfmt(sbs *s, char const *fmt, ...);
void sbstrim(sbs *s, const char *cset);
void sbsrange(sbs *s, ssize_t start, ssize_t end);
void sbsupdatelen(sbs *s);
void sbsclear(sbs *s);
int sbscmp(const sbs *s1, const sbs *s2);
// sds *sdssplitlen(const char *s, ssize_t len, const char *sep, int seplen, int *count);
// void sdsfreesplitres(sds *tokens, int count);
void sbstolower(sbs *s);
void sbstoupper(sbs *s);
int sbsfromlonglong(sbs *s, long long value);
int sbscatrepr(sbs *s, const char *p, size_t len);
// sds *sdssplitargs(const char *line, int *argc);
int sbsmapchars(sbs *s, const char *from, const char *to, size_t setlen);
int sbsjoin(sbs *s, char **argv, int argc, char *sep);
int sbsjoinsbs(sbs *s, sbs *argv, int argc, const char *sep, size_t seplen);

/* Low level functions exposed to the user API */
// sds sdsMakeRoomFor(sds s, size_t addlen);
// void sdsIncrLen(sds s, ssize_t incr);
// sds sdsRemoveFreeSpace(sds s);
// size_t sdsAllocSize(sds s);
// void *sdsAllocPtr(sds s);

/* Export the allocator used by SDS to the program using SDS.
 * Sometimes the program SDS is linked to, may use a different set of
 * allocators, but may want to allocate or free things that SDS will
 * respectively free or allocate. */
// void *sds_malloc(size_t size);
// void *sds_realloc(void *ptr, size_t size);
// void sds_free(void *ptr);

#endif