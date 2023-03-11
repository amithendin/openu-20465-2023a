/*
 * util.h
 *
 *  Created on: Jan 22, 2023
 *      Author: amit
 */

#ifndef UTIL_H
#define UTIL_H

#define MAX_FILE_PATH 200 /*for storing file paths*/
#define LINE_SIZE 81 /*for reading files line by line*/
#define LABEL_SIZE 30
#define BASE_ADDRESS 100
#define MAX_TOKENS_PER_LINE 50
#define MAX_TOKEN_LEN 50
#define ASM_WORD_SIZE 14 /*assembler word size in bits*/
#define ZERO_CHAR '.'
#define ONE_CHAR '/'

#define SYM_COD 1 /*represents instruction token*/
#define SYM_DAT 2 /*represents data token*/
#define SYM_STR 4 /*represents string token*/
#define SYM_EXT 8 /*represents external token*/
#define SYM_ENT 16 /*represents entry token*/
#define SYM_DEF 32 /*represents label definition token*/

#define OPTYPE_INS 0 /*represents opcode with no operands*/
#define OPTYPE_UNI 1 /*represents opcode with 1 operand*/
#define OPTYPE_BIN 2 /*represents opcode with 2 operands*/
#define OPTYPE_JMP 4 /*represents jump opcode*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

/*shortcut when trying to save memory space*/
typedef unsigned char byte;

/**
 * Checks if character is a digit
 *
 * @param c a character
 * @return true is a character is numeric including positive and negative signs
 */
int is_num(char c);

/**
 * Checks if character is a alphabetic
 *
 * @param c a character
 * @return true is a character is alphabetic, otherwise returns false
 */
int is_alpha(char c);

/**
 * Checks if character is a alphanumeric
 *
 * @param c a character
 * @return true is a character is alphabetic or numeric, otherwise returns false
 */
int is_alpha_num(char c);

/**
 * Trims whitespace from the start of the string
 *
 * @param str a string pointer
 * @return the given pointer + offset to the first non-whistapce characters, to trim white spaces from the left
 */
char* trim_left(char *str);

/**
 * Trims whitespace from the end of the string
 *
 * @param str a string pointer
 * @param len string length
 * @return the given pointer, sets the null termiator at the last non-whistapce character and returns the given pointer, to trim white spaces from the right
 */
char* trim_right(char *str, int len);
/**
 * Trims whitespace from either ends of the string
 *
 * @param str a string pointer
 * @return the given pointer + offset to the first non-whistapce characters, also
 * sets the null termiator at the last non-whistapce character
 */
char* trim(char *str);

/**
 * Convertes a string to integer based on the characters in string (given they are digits)
 *
 * @param str a string pointer with or without - char in the beginning with only digits
 * @return an integer that is equal to the integer represented by the string charaters digits with respect to sign charater if present
 */
int str_to_int(char* str);
/**
 * Opens file in append mode but also clears the file's contents before
 *
 * @param path path to file to overwrite
 * @return file handle to write to
 */
FILE* open_file_append(const char* path);

#endif /* UTIL_H_ */
