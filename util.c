/*
 * util.c
 *
 *  Created on: Jan 22, 2023
 *      Author: amit
 */
#include "util.h"

int is_num(char c) {
    return c == '-' || c == '+' || (c >= '0' && c <= '9');/*assume digit characters are neighbouring in order in encoding*/
}

int is_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');/*assume letter characters are neighbouring in order in encoding*/
}

int is_alpha_num(char c) {
    return is_alpha(c) || is_num(c); /*use previously defined functions*/
}

char *trim_left(char *str) {
    int i;
    /*scan string from start*/
    for (i = 0; str[i] != '\0'; i++) {
        if (!isspace(str[i])) {
            break;/*if found, stop scanning*/
        }
    }
    return str + i;/*return pointer + alphanumeric offset*/
}

char *trim_right(char *str, int len) {
    int i;
    /*scan string from the end*/
    for (i = len - 1; i >= 0; i--) {
        /*if found alphanumeric character, place a null termiator infront of it and stop looking*/
        if (!isspace(str[i])) {
            str[i + 1] = '\0';
            break;
        }
    }
    return str;
}

char *trim(char *str) {
    /*use previously defined functions*/
    int len = strlen(str);
    trim_right(str, len);
    return trim_left(str);
}

int str_to_int(char *str) {
    int num, i, len;
    num = 0;
    len = strlen(str);
    for (i = 0; i < len; i++) {
        /*for each digit character, add it to number and increase number's magnitude*/
        if (str[i] >= '0' && str[i] <= '9') {
            num *= 10;
            num += str[i] - '0';
        }
    }
    if (*str == '-') {/*negate number if we started with a negtive sign*/
        num *= -1;
    }
    return num;
}

FILE *open_file_append(const char *path) {
    /*open file in write mode to clear it's contents*/
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        printf("Error opening file: %s\n", path);
        return NULL;
    }
    fclose(file);/*close file and reopen in append mode to append line to it*/
    file = fopen(path, "a");
    return file;
}
