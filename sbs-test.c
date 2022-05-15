#include "sbs.h"

#include <stdio.h>

int main() {
    sbs* text = SBSEMPTY(512);
    sbscatfmt(text, "testing %i %i %s\n", 23, 33, "test");
    sbscatprintf(text, "%d %20d", 232, 1101101);
    printf("%s\n", sbsstr(text));

    sbs* text2 = SBSNEW("cool", 512);
    printf("%s", sbsstr(text2));
    return 0;
}
