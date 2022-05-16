#include <stdio.h>
#include <string.h>

#include "sbs.h"
#include "tinytest.h"

void test_sbsnewlen() {
    const char raw_data[] = {65, 65, 65, 0x0, 23, 32};
    sbs* data = SBSNEWLEN(raw_data, sizeof(raw_data), 512);
    ASSERT_EQUALS(sbslen(data), sizeof(raw_data));
    ASSERT_EQUALS(raw_data[4], data->str[4]);  // check it went pass null-term
}

void test_sbsnew() {
    sbs* text = SBSNEW("this is a test", 512);
    ASSERT_STRING_EQUALS(sbsstr(text), "this is a test");
    sbs* fail = SBSNEW("this is too long", 16);
    ASSERT_EQUALS(NULL, fail);
    sbs* canfit = SBSNEW("this is just right", 19);
    ASSERT_STRING_EQUALS(sbsstr(canfit), "this is just right");
}

void test_sbsempty() {
    sbs* text = SBSEMPTY(512);
    ASSERT_EQUALS(sbslen(text), 0);
    ASSERT_EQUALS(text->str[0], '\0');
    ASSERT_EQUALS(512, sbssize(text));
}

void test_sbsdup() {
    sbs* text = SBS512("this is test text");
    sbs* text2 = SBSDUP(text, 512);
    sbscat(text, " newer text");
    // ensure they are separate buffers
    ASSERT_STRING_EQUALS("this is test text", sbsstr(text2));
    sbs* fail = SBSDUP(text, 17);
    ASSERT_EQUALS(NULL, fail);
}

void test_sbscat() {
    sbs* text = SBS512("first");
    sbscat(text, "|second");
    ASSERT_STRING_EQUALS("first|second", sbsstr(text));
    sbs* small = SBSNEW("", 12);
    sbscat(small, "first");
    int err = sbscat(small, "this text is way too long");
    ASSERT("Buffer overflow check", err != 0);
    sbscat(small, " ok");
    ASSERT_STRING_EQUALS(sbsstr(small), "first ok");
}

void test_sbscatsbs() {
    sbs* text = SBS512("first");
    sbscat(text, "|second");
    ASSERT_STRING_EQUALS("first|second", sbsstr(text));
    sbs* combined = SBS512("zero|");
    sbscatsbs(combined, text);
    ASSERT_STRING_EQUALS(sbsstr(combined), "zero|first|second");
}

void test_sbscpy() {
    sbs* text = SBS512("text to replace");
    sbscpy(text, "newer text");
    ASSERT_STRING_EQUALS("newer text", sbsstr(text));
    sbs* fail = SBSEMPTY(4);
    int err = sbscpy(fail, "too long text");
    ASSERT("overflow copy", err != 0);
    sbscpy(fail, "ok");
    ASSERT_STRING_EQUALS("ok", sbsstr(fail));
}

void test_sbsfromlonglong() {
    sbs* nums;
    nums = SBSFROMLL(23);
    ASSERT_STRING_EQUALS("23", sbsstr(nums));
}

int main() {
    RUN(test_sbsnewlen);
    RUN(test_sbsnew);
    RUN(test_sbsempty);
    RUN(test_sbsdup);
    RUN(test_sbscat);
    RUN(test_sbscatsbs);
    RUN(test_sbscpy);
    RUN(test_sbsfromlonglong);
    return TEST_REPORT();
}
