#include "sbs.h"
#include <stdio.h>
int main()
{
    char buffer[512];
    sbs text = SBSEMPTY(buffer);
    sbscatfmt(&text, "testing %i %i %s\n", 23, 33, "test");
    sbscatprintf(&text, "%d %20d", 232, 1101101);
    printf("%s\n", sbsstr(text));
    return 0;
}
