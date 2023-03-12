/*
 * validate.h
 *
 *  Created on: Mar 2, 2023
 *      Author: amit
 */

#ifndef VALIDATE_H
#define VALIDATE_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "util.h"
#include "hashtable.h"
#include "list.h"
#include "parse.h"

#define ERR_SIZE 500 /*size of the string containing the error message*/

/**
 * Checks that all tokens in given list constitute a valid assembly line.
 * If they don't, meaning there is a syntax error, fill error string pointer
 * with error description. If error string remains empty of execution then tokens
 * list is valid
 *
 * @param tokens a list of tokens that makes up one line in the assembly language
 * @param err a string pointer in which to fill the description of the error in the tokens
 */
void validate_tokens(List *tokens, char *err);

/**
 * Checks that *!marco expanded!* code file has valid syntax.
 * If there is a syntax error, print error description and set is_valid flag to 0.
 *
 * @param code_file_path path to the .am *!marco expanded!* code file
 * @param is_valid a byte reference to put the is_valid flag in
 */
void validate_code(const char *code_file_path, byte *is_valid);

#endif /* VALIDATE_H_ */
