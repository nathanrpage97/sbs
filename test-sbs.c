#include <stdio.h>
#include <string.h>

#include "sbs.h"
#include "tinytest.h"

void test_sbsnew() {
    sbs* text = SBSNEW("this is a test", 512);
    ASSERT_STRING_EQUALS(sbsstr(text), "this is a test");
    sbs* fail = SBSNEW("this is too long", 16);
    ASSERT_EQUALS(NULL, fail);
    sbs* canfit = SBSNEW("this is just right", 19);
    ASSERT_STRING_EQUALS(sbsstr(canfit), "this is just right");
}

void test_sbsnewlen() {
    const char raw_data[] = {65, 65, 65, 0x0, 23, 32};
    sbs* data = SBSNEWLEN(raw_data, sizeof(raw_data), 512);
    ASSERT_EQUALS(sbslen(data), sizeof(raw_data));
    ASSERT_EQUALS(raw_data[4], data->str[4]);  // check it went pass null-term
}

void test_sbsempty() {
    sbs* text = SBSEMPTY(512);
    ASSERT_EQUALS(sbslen(text), 0);
    ASSERT_EQUALS(text->str[0], '\0');
    ASSERT_EQUALS(sbssize(text), 512);
}

// int main() {
//     sbs* text = SBS512("");
//     sbscatfmt(text, "testing %i %i %s\n", 23, 33, "test");
//     sbscatprintf(text, "%d %20d", 232, 1101101);
//     printf("%s\n", sbsstr(text));

//     sbs* text2 = SBS512("cool");
//     printf("text2: %s\n", sbsstr(text2));
//     sbs* text3 = SBSDUP(text2, 512);
//     sbscat(text3, " more text");
//     printf("text3: %s\n", sbsstr(text3));
//     printf("text2: %s\n", sbsstr(text2));
//     return 0;
// }

int main() {
    RUN(test_sbsnewlen);
    RUN(test_sbsnew);
    RUN(test_sbsempty);
    return TEST_REPORT();
}
