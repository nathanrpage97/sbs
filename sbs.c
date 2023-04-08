#include "sbs.h"

#include <ctype.h>
#ifndef SBS_NO_FORMAT
#include <stdarg.h>
#include <stdio.h>
#endif

#include <string.h>

#define SBS_NULLTERM(h) (h)->str[(h)->len] = '\0'

#define SBS_CATCH(func, catch) \
  do {                         \
    int err = (func);          \
    if (err != 0) {            \
      catch;                   \
    }                          \
  } while (0);

sbs sbsnewlen(const void *init, size_t initlen, char buffer[],
              size_t buffer_size) {
  if (initlen >= buffer_size - SBS_HEADERSIZE) {
    return NULL;
  }
  sbshdr *h = (sbshdr *)(buffer);
  h->str = buffer + SBS_HEADERSIZE;
  h->size = buffer_size - SBS_HEADERSIZE;
  h->len = initlen;
  memcpy(h->str, init, initlen);
  SBS_NULLTERM(h);
  return h->str;
}

sbs sbsnew(const char *init, char buffer[], size_t buffer_size) {
  return sbsnewlen(init, strlen(init), buffer, buffer_size);
}

sbs sbsempty(char *buffer, size_t buffer_size) {
  // this will never fail
  return sbsnewlen("", 0, buffer, buffer_size);
}

sbs sbsdup(sbs s, char buffer[], size_t buffer_size) {
  sbshdr *h = to_sbshdr(s);
  return sbsnewlen(h->str, h->len, buffer, buffer_size);
}

void sbsupdatelen(sbs str) {
  sbshdr *h = to_sbshdr(str);
  h->len = strlen(h->str);
}

void sbsclear(sbs str) {
  sbshdr *h = to_sbshdr(str);
  h->len = 0;
  SBS_NULLTERM(h);
}

int sbscatlen(sbs str, const void *t, size_t len) {
  sbshdr *h = to_sbshdr(str);
  if (len >= sbsavail(str)) {
    return -1;
  }
  memcpy(sbsend(str), t, len);
  h->len += len;
  SBS_NULLTERM(h);
  return 0;
}

int sbscat(sbs s, const char *t) { return sbscatlen(s, t, strlen(t)); }

int sbscatsbs(sbs s, const sbs tstr) {
  sbshdr *t = to_sbshdr(tstr);
  return sbscatlen(s, t->str, t->len);
}

int sbscpylen(sbs str, const char *t, size_t len) {
  sbshdr *h = to_sbshdr(str);
  if (len >= h->size) {
    return -1;
  }
  h->len = len;
  memcpy(h->str, t, len);
  SBS_NULLTERM(h);
  return 0;
}

int sbscpy(sbs s, const char *t) { return sbscpylen(s, t, strlen(t)); }

#ifndef SBS_NO_FORMAT
/* Like sbscatprintf() but gets va_list instead of being variadic. */
int sbscatvprintf(sbs str, const char *fmt, va_list ap) {
  sbshdr *h = to_sbshdr(str);
  size_t bufsize = sbsavail(str);
  int n = vsnprintf(sbsend(str), bufsize, fmt, ap);
  if (n >= bufsize) {
    SBS_NULLTERM(h);
    return -1;
  }
  h->len += n;
  SBS_NULLTERM(h);
  return 0;
}

int sbscatprintf(sbs s, const char *fmt, ...) {
  va_list ap;
  int t;
  va_start(ap, fmt);
  t = sbscatvprintf(s, fmt, ap);
  va_end(ap);
  return t;
}
#endif

void sbstrim(sbs str, const char *cset) {
  sbshdr *h = to_sbshdr(str);
  char *start, *end, *sp, *ep;
  size_t len;

  sp = start = h->str;
  ep = end = h->str + sbslen(str) - 1;
  while (sp <= end && strchr(cset, *sp)) sp++;
  while (ep > sp && strchr(cset, *ep)) ep--;
  len = (sp > ep) ? 0 : ((ep - sp) + 1);
  if (h->str != sp) memmove(h->str, sp, len);
  h->len = len;
  SBS_NULLTERM(h);
}

void sbsrange(sbs str, ssize_t start, ssize_t end) {
  sbshdr *h = to_sbshdr(str);
  size_t newlen, len = sbslen(str);

  if (len == 0) return;
  if (start < 0) {
    start = (ssize_t)len + start;
    if (start < 0) start = 0;
  }
  if (end < 0) {
    end = (ssize_t)len + end;
    if (end < 0) end = 0;
  }
  newlen = (start > end) ? 0 : (end - start) + 1;
  if (newlen != 0) {
    if (start >= (ssize_t)len) {
      newlen = 0;
    } else if (end >= (ssize_t)len) {
      end = (ssize_t)len - 1;
      newlen = (start > end) ? 0 : (end - start) + 1;
    }
  } else {
    start = 0;
  }
  if (start && newlen) memmove(h->str, h->str + start, newlen);
  h->len = newlen;
  SBS_NULLTERM(h);
}

/* Apply tolower() to every character of the sbs string 's'. */
void sbstolower(sbs str) {
  sbshdr *h = to_sbshdr(str);
  for (size_t j = 0; j < sbslen(str); j++) h->str[j] = (char)tolower(h->str[j]);
}

/* Apply tolower() to every character of the sbs string 's'. */
void sbstoupper(sbs str) {
  sbshdr *h = to_sbshdr(str);
  for (size_t j = 0; j < sbslen(str); j++) h->str[j] = (char)toupper(h->str[j]);
}

int sbscmp(const sbs str1, const sbs str2) {
  sbshdr *h1 = to_sbshdr(str1);
  sbshdr *h2 = to_sbshdr(str2);
  size_t l1, l2, minlen;
  int cmp;

  l1 = sbslen(str1);
  l2 = sbslen(str2);
  minlen = (l1 < l2) ? l1 : l2;
  cmp = memcmp(h1->str, h2->str, minlen);
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
void sbsmapchars(sbs str, const char *from, const char *to, size_t setlen) {
  size_t j, i, l = sbslen(str);
  sbshdr *h = to_sbshdr(str);
  for (j = 0; j < l; j++) {
    for (i = 0; i < setlen; i++) {
      if (h->str[j] == from[i]) {
        h->str[j] = to[i];
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
#ifndef SBS_NO_FORMAT
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
#endif
/* Create an sbs string from a long long value. It is much faster than:
 *
 * sbscatprintf(sbsempty(),"%lld\n", value);
 */
sbs sbsfromlonglong(char buffer[64], long long value) {
  char buf[SBS_LLSTR_SIZE];
  int len = sbsll2str(buf, value);
  return sbsnewlen(buf, len, buffer, 64);
}

/* Join an array of C strings using the specified separator (also a C string).
 * Returns the result as an sbs string. */
int sbsjoin(sbs str, const char **argv, int argc, const char *sep) {
  sbshdr *h = to_sbshdr(str);
  int j;
  sbshdr backup = *h;

  for (j = 0; j < argc; j++) {
    SBS_CATCH(sbscat(str, argv[j]), goto fail);
    if (j != argc - 1) {
      SBS_CATCH(sbscat(str, sep), goto fail);
    }
  }
  return 0;
fail:
  *h = backup;
  SBS_NULLTERM(h);  // fix the buffer
  return -1;
}

int sbsjoinsbs(sbs str, const sbs argv[], int argc, const char *sep,
               size_t seplen) {
  int j;

  sbshdr *h = to_sbshdr(str);
  sbshdr backup = *h;

  for (j = 0; j < argc; j++) {
    SBS_CATCH(sbscatsbs(str, argv[j]), goto fail);
    if (j != argc - 1) {
      SBS_CATCH(sbscatlen(str, sep, seplen), goto fail);
    }
  }
  return 0;
fail:
  *h = backup;
  SBS_NULLTERM(h);  // fix the buffer
  return -1;
}
#ifndef SBS_NO_FORMAT
static int sbscatvfmt(sbs str, char const *fmt, va_list ap) {
  sbshdr *h = to_sbshdr(str);
  sbshdr backup = *h;
  const char *f = fmt;

  f = fmt; /* Next format specifier byte to process. */
  while (*f) {
    char next, *str2;
    size_t l;
    long long num;
    unsigned long long unum;

    switch (*f) {
      case '%':
        next = *(f + 1);
        f++;
        switch (next) {
          case 's': {
            str2 = va_arg(ap, char *);
            SBS_CATCH(sbscat(str, str2), goto fail);
            break;
          }
          case 'S': {
            sbs tmp_sbs = va_arg(ap, sbs);
            SBS_CATCH(sbscatsbs(str, tmp_sbs), goto fail);
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
              SBS_CATCH(sbscatlen(str, numbuf, l), goto fail);
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
              SBS_CATCH(sbscatlen(str, numbuf, l), goto fail);
              break;
            }
            break;
          default: { /* Handle %% and generally %<unknown>. */
            SBS_CATCH(sbscatlen(str, &next, 1), goto fail);
            break;
          }
        }
        break;
      default: {
        SBS_CATCH(sbscatlen(str, f, 1), goto fail);
        break;
      }
    }
    f++;
  }
  return 0;
fail:
  *h = backup;
  SBS_NULLTERM(h);  // fix the buffer
  return -1;
}

int sbscatfmt(sbs s, char const *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int err = sbscatvfmt(s, fmt, ap);
  va_end(ap);
  return err;
}

int sbscatrepr(sbs s, const char *p, size_t len) {
  sbshdr *h = to_sbshdr(s);
  sbshdr backup = *h;
  SBS_CATCH(sbscatlen(s, "\"", 1), goto fail);
  while (len--) {
    switch (*p) {
      case '\\':
      case '"':
        SBS_CATCH(sbscatprintf(s, "\\%c", *p), goto fail);
        break;
      case '\n':
        SBS_CATCH(sbscatlen(s, "\\n", 2), goto fail);
        break;
      case '\r':
        SBS_CATCH(sbscatlen(s, "\\r", 2), goto fail);
        break;
      case '\t':
        SBS_CATCH(sbscatlen(s, "\\t", 2), goto fail);
        break;
      case '\a':
        SBS_CATCH(sbscatlen(s, "\\a", 2), goto fail);
        break;
      case '\b':
        SBS_CATCH(sbscatlen(s, "\\b", 2), goto fail);
        break;
      default:
        if (isprint(*p)) {
          SBS_CATCH(sbscatprintf(s, "%c", *p), goto fail);
        } else {
          SBS_CATCH(sbscatprintf(s, "\\x%02x", (unsigned char)*p), goto fail);
        }
        break;
    }
    p++;
  }
  SBS_CATCH(sbscatlen(s, "\"", 1), goto fail);
  return 0;
fail:
  *h = backup;
  SBS_NULLTERM(h);  // fix string buffer
  return -1;
}
#endif
