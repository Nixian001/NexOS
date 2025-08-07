#include <stdint.h>
#include <stdbool.h>
#include "mem.h"

char* bin;

void util_init() {
    bin = (char*) mem_alloc(32 * sizeof(char));
}

void util_quit() {
    mem_free(bin);
}

int string_length(char s[]) {
    int i = 0;
    while (s[i] != '\0') ++i;
    return i;
}

void append(char s[], char n) {
    int len = string_length(s);
    s[len] = n;
    s[len + 1] = '\0';
}

bool backspace(char buffer[]) {
    int len = string_length(buffer);
    if (len > 0) {
        buffer[len - 1] = '\0';
        return true;
    } else {
        return false;
    }
}

void reverse(char s[]) {
    int c, i, j;
    for (i = 0, j = string_length(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void int_to_string(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) str[i++] = '-';
    str[i] = '\0';

    reverse(str);
}

/* K&R
 * Returns <0 if s1<s2, 0 if s1==s2, >0 if s1>s2 */
int compare_string(char s1[], char s2[]) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

char* dec_to_bin(int n, int len) {
    int i = 0;

    for (i = 0; i < len; i++) {
        bin[i] = '0';
    }

    i = 0;

    while (n > 0) {
        int bit = n % 2;
        bin[i++] = '0' + bit;
        n /= 2;
    }

    // Reverse the binary string
    reverse(bin);
    return bin;
}
