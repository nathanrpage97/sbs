#ifndef SBS_H
#define SBS_H

#ifndef SBS_NO_FORMAT
#include <stdarg.h>
#endif
#include <stdint.h>
#include <sys/types.h>

typedef char *sbs;

typedef struct {
  size_t len;
  size_t size;
  char *str;
} sbshdr;

#define SBS_HEADERSIZE (sizeof(sbshdr))

#define to_sbshdr(s) (sbshdr *)((s)-SBS_HEADERSIZE)

// Properties

static inline size_t sbslen(const sbs s) {
  sbshdr *h = to_sbshdr(s);
  return h->len;
}

static inline size_t sbssize(const sbs s) {
  sbshdr *h = to_sbshdr(s);
  return h->size;
}
static inline size_t sbsavail(const sbs s) {
  sbshdr *h = to_sbshdr(s);
  return h->size - h->len;
}
static inline char *sbsend(const sbs s) {
  sbshdr *h = to_sbshdr(s);
  return h->str + h->len;
}

// Initialization
sbs sbsnewlen(const void *init, size_t initlen, char buffer[],
              size_t buffer_size);
#define SBSNEWLEN(init, initlen, size) \
  sbsnewlen(init, initlen, (char[size]){0}, size)
sbs sbsnew(const char *init, char buffer[], size_t buffer_size);
#define SBSNEW(init, size) \
  sbsnew(init, (char[(size) + SBS_HEADERSIZE]){0}, (size) + SBS_HEADERSIZE)
sbs sbsempty(char buffer[], size_t buffer_size);
#define SBSEMPTY(size) \
  sbsempty((char[(size) + SBS_HEADERSIZE]){0}, (size) + SBS_HEADERSIZE)
sbs sbsdup(sbs s, char buffer[], size_t buffer_size);
#define SBSDUP(s, size) \
  sbsdup(s, (char[(size) + SBS_HEADERSIZE]){0}, (size) + SBS_HEADERSIZE)
sbs sbsfromlonglong(char buffer[64], long long value);
#define SBSFROMLL(value) sbsfromlonglong((char[64]){0}, value)

// add some convenience creators
#define SBS64(init) SBSNEW(init, 128)
#define SBS128(init) SBSNEW(init, 128)
#define SBS256(init) SBSNEW(init, 256)
#define SBS512(init) SBSNEW(init, 512)
#define SBS1024(init) SBSNEW(init, 1024)
#define SBS2048(init) SBSNEW(init, 2048)

// fill
int sbscatlen(sbs s, const void *t, size_t len);
int sbscat(sbs s, const char *t);
int sbscatsbs(sbs s, const sbs t);
int sbscpylen(sbs s, const char *t, size_t len);
int sbscpy(sbs s, const char *t);

// format
#ifndef SBS_NO_FORMAT
int sbscatvprintf(sbs s, const char *fmt, va_list ap);
#ifdef __GNUC__
int sbscatprintf(sbs s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
int sbscatprintf(sbs s, const char *fmt, ...);
#endif
int sbscatfmt(sbs s, char const *fmt, ...);
int sbscatrepr(sbs s, const char *p, size_t len);
#endif

// modify
void sbstrim(sbs s, const char *cset);
void sbsrange(sbs s, ssize_t start, ssize_t end);
void sbsclear(sbs s);
int sbscmp(const sbs s1, const sbs s2);
void sbstolower(sbs s);
void sbstoupper(sbs s);
void sbsmapchars(sbs s, const char *from, const char *to, size_t setlen);

// join
int sbsjoin(sbs s, const char **argv, int argc, const char *sep);
int sbsjoinsbs(sbs s, const sbs argv[], int argc, const char *sep,
               size_t seplen);

// escape hatch
static inline void sbssetlen(sbs s, size_t len) {
  sbshdr *h = to_sbshdr(s);
  if (h->len >= h->size) return;
  h->len = len;
  h->str[h->len] = '\0';  // null terminate in case
}
void sbsupdatelen(sbs s);

#endif
