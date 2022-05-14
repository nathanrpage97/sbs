#include "sbs.h"

#include <string.h>

sbs sbsnewlen(const void *init, size_t initlen, char buffer[], size_t buffer_size)
{
    sbs s;
    s.str = buffer;
    s.size = buffer_size;
    memcpy(s.str, init, initlen);
    s.str[initlen] = '\0';
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
    s->str[s->len] = '\0';
    return 0;
}

void sbsupdatelen(sbs *s)
{
    s->len = strlen(s->str);
}

void sbsclear(sbs *s)
{
    s->len = 0;
    s->str[0] = '\0';
}

int sbscatlen(sbs *s, const void *t, size_t len)
{
    if (len >= sbssizerem(*s))
    {
        return -1;
    }
    memcpy(sbsstrend(*s), t, len);
    s->len += len;
    s->str[s->len] = '\0';
    return 0;
}

int sbscat(sbs *s, const char *t)
{
    return sbscatlen(s, t, strlen(t));
}