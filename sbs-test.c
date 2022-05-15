#include "sbs.h"

#include <stdio.h>

int main() {
    sbs* text = SBSEMPTY(512);
    sbscatfmt(text, "testing %i %i %s\n", 23, 33, "test");
    sbscatprintf(text, "%d %20d", 232, 1101101);
    printf("%s\n", sbsstr(text));

    sbs* text2 = SBSNEW("cool", 512);
    printf("text2: %s\n", sbsstr(text2));
    sbs* text3 = SBSDUP(text2, 512);
    printf("text3: %s\n", sbsstr(text3));
    return 0;
}
