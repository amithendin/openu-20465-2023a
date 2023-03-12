/*
 * validate.c
 *
 *  Created on: Mar 2, 2023
 *      Author: amit
 */
#include "validate.h"

/**
 * Check if given token string constitutes a valid opcode and
 * givens it's opcode integer and optype byte
 *
 * @param str token string to check
 * @param opcode a reference to an integer to put the opcode in
 * @param optype a reference to a byte to put the optype in
 * @return 1 if string is a valid opcode, 0 otherwise
 */
int is_valid_opcode(char *str, int *opcode, byte *optype) {
    int op;
    /*find the integer opcode*/
    op = get_opcode(str);
    if (op == -1) {
        return 0;/*if integer opcode was not found then opcode string is invalid*/
    }
    /*figure out which type of opcode we got according to the language spec and set it*/
    if (op == 6 || (op >= 0 && op <= 3)) {
        *optype = OPTYPE_BIN;/*binary operand op*/
    } else if (op == 4 || op == 5 || op == 7
               || op == 8 || op == 11 || op == 12) {
        *optype = OPTYPE_UNI;/*unary operand op*/
    } else if (op == 9 || op == 10 || op == 13) {
        *optype = OPTYPE_JMP;/*jump type op*/
    } else {
        *optype = OPTYPE_INS;/*0 operand ops like ret or stop*/
    }
    /*set opcode*/
    *opcode = op;
    /*if we didn't return then the value is correct*/
    return 1;
}

/**
 * Check if given token string constitutes a valid immediate value
 *
 * @param str token string to check
 * @param err an empty buffer to put the error in. if token string is valid immediate buffer will remain empty
 * @param no_prefix if immediate value requires # before it
 * @return 1 if string is a valid opcode, 0 otherwise
 */
int is_valid_imm(char *str, char *err, byte no_prefix) {
    long num = 0;/*number we discover from the string*/
    unsigned long i, j, len;/*counters*/
    len = strlen(str);
    /*set the char to start iterating from, we do this because immediate values in the data section are without the # prefix*/
    j = 0;
    if (!no_prefix) {
        if(str[0] != '#') {
            sprintf(err, "immediate value must begin with #");
            return 0;
        }
        j = 1;
    }
    /*make sure first char is valid*/
    if (!is_num(str[j])) {
        sprintf(err, "numbers must start with digit or + or -");
        return 0;
    }
    i = (str[j] == '-' || str[j] == '+') ? j + 1 : j;/*if first char is a sign, start from the next one*/

    for (; i < len && str[i] != '\0'; i++) {/*iterate over all chars, validate that they are all digits, and convert the string to a number*/
        if (str[i] < '0' || str[i] > '9') {
            sprintf(err, "numbers can only contain digits");
            return 0;
        }

        num *= 10;
        num += str[i];
    }
    /*after we converted the string to an integer, we must check that said integer fits within our word
     * as specified, we get 12 bits to store a data word and we are storing signed numbers so that leaves
     * us just 11 bits for the actual value, therefore we shift all the bits in our number 11 bits to the right
     * and if the resulting number isn't zero than we know we got an immediate value that is too big for our words*/
    if (num >> (ASM_WORD_SIZE - 3)) {/*sub 2 for ERA and 1 for sign*/
        sprintf(err, "number too large for %d bits", ASM_WORD_SIZE - 2);
        return 0;
    }
    /*if we didn't return then the value is correct*/
    return 1;
}

/**
 * Check if given token string constitutes a valid register
 *
 * @param str token string to check
 * @param err an empty buffer to put the error in. if token string is valid register buffer will remain empty
 * @return 1 if string is a valid register, 0 otherwise
 */
int is_valid_reg(char *str, char *err) {
    if (str[0] != 'r') {/*all registers must start with r*/
        sprintf(err, "registers must begin with the letter 'r'");
        return 0;
    }
    /*we have 8 registers to work with numbered 0 through 7*/
    if (str[1] < '0' || str[1] > '7' || strlen(str) > 2) {
        sprintf(err, "no such register");
        return 0;
    }
    /*if we didn't return then the value is correct*/
    return 1;
}

/**
 * Check if given token string constitutes a valid label
 *
 * @param str token string to check
 * @param err an empty buffer to put the error in. if token string is valid label buffer will remain empty
 * @return 1 if string is a valid label, 0 otherwise
 */
int is_valid_label(char *str, char *err) {
    unsigned int i, len;
    len = strlen(str);
    /*language spec of label*/
    if (len > LABEL_SIZE || len == 0) {
        sprintf(err, "labels must be no more than 30 characters long and no less than 1");
        return 0;
    }
    /*language spec of label*/
    if (!is_alpha(str[0])) {
        sprintf(err, "labels must start with letter");
        return 0;
    }
    /*language spec of label*/
    for (i = 1; i < len; i++) {
        if (!is_alpha_num(str[i])) {
            sprintf(err, "labels can only contain alphanumeric characters");
            return 0;
        }
    }
    /*if we didn't return then the value is correct*/
    return 1;
}

void validate_code(const char *code_file_path, byte *is_valid) {
    /*flags, counter and tmp storage*/
    char line[LINE_SIZE + 10];
    FILE *in_file;
    List *tokens;
    char tok_err[ERR_SIZE];
    Node *curr;
    unsigned int line_num, line_len, i;

    *is_valid = 1; /*assume code file is correct*/

    /*open source code file in read mode*/
    in_file = fopen(code_file_path, "r");
    if (in_file == NULL) {
        printf("error code (0) failed to open file %s", code_file_path);
        *is_valid = 0;
        return;
    }
    line_num = 1;/*start counting lines from 1*/

    while (fgets(line, sizeof(line), in_file)) {/*iterate over all lines*/
        memset(tok_err, '\0', ERR_SIZE);/*reset error message buffer*/
        line_len = strlen(line);/*for readability and ease of use*/

        if (line_len > LINE_SIZE) { /*check for line length*/
            printf("line %d error code (99): exeeded maximum line size of %d characters", line_num, LINE_SIZE);
            *is_valid = 0;
            continue;
        }

        if (line[0] == ';') {/*comment line*/
            continue;
        }
        for (i = 0; i < line_len; i++) {/*check that entire line is whitespace*/
            if (!isspace(line[i])) {
                break;
            }
        }
        if (i >= line_len) { /*whitespace line*/
            continue;
        }

        tokens = tokenize(line);/*convert the line to language tokens*/
        validate_tokens(tokens, tok_err);/*get error code from line tokens, if there is any*/
        curr = tokens->tail;/*free tokens list because after checking for errors we have nothing to do with it*/
        while (curr) {
            free(curr->data);/*free token string*/
            curr = curr->prev;
        }
        free_list(tokens);/*free list data*/

        if (strlen(tok_err) > 0) {
            /*if we found an error in the line, print the error and mark file as in correct by
             * setting is_valid = 0*/
            printf("line %d error code %s\n", line_num, tok_err);
            *is_valid = 0;
        }
        line_num++;/*count lines to report which line if the offending line*/
    }
    /*close the code file*/
    fclose(in_file);
}

void validate_tokens(List *tokens, char *err) {
    /*flags, counters, and tmp storage*/
    byte type, optype, need_comma;
    int opcode;
    Node *curr;
    char *tmp, sub_err[ERR_SIZE / 2];
    curr = tokens->tail;

    need_comma = type = optype = opcode = 0;

    if (curr->prev != NULL && strcmp((char *) curr->prev->data, ":") == 0) {
        /*if we have label definition check that label name is valid*/
        tmp = (char *) curr->data;/*take token string*/
        if (!is_valid_label(tmp, sub_err)) {
            sprintf(err, "(1): invalid label name \"%s\" at definition, %s", tmp, sub_err);
            return;
        }
        curr = curr->prev->prev;/*skip to next token after the 2 tokens needed to make definitions*/
        type |= SYM_DEF;/*make line as label definition line*/
    }

    if (curr == NULL) {/*make sure we still have tokens*/
        sprintf(err, "(2): expected instruction or data after label definition");
        return;
    }
    tmp = (char *) curr->data;/*take token string*/
    if (strcmp(tmp, ".extern") == 0) {/*.extern definition*/
        curr = curr->prev;
        if (curr == NULL) {/*make sure we have next token*/
            sprintf(err, "(3): .extern must be followed by space and then label");
            return;
        }
        tmp = (char *) curr->data;/*take token string*/
        type |= SYM_EXT;

        if (!is_valid_label(tmp, sub_err)) {/*make sure external definition is of a valid label*/
            sprintf(err, "(4): invalid label name \"%s\" at external declaration, %s", tmp, sub_err);
            return;
        }

        if (curr->prev != NULL) { /*make sure we only have one token after .extern*/
            sprintf(err, "(5): too many arguments after external declaration. extern must be preceded by exactly one valid label name");
            return;
        }

    } else if (strcmp(tmp, ".entry") == 0) {/*.entry definition*/
        curr = curr->prev;
        if (curr == NULL) {/*make sure we have next token*/
            sprintf(err, "(3): .entry must be followed by space and then label");
            return;
        }
        tmp = (char *) curr->data;
        type |= SYM_ENT;/*mark as .entry definition*/

        if (!is_valid_label(tmp, sub_err)) {/*make sure entry definition is of a valid label*/
            sprintf(err, "(4): invalid label name \"%s\" at entry declaration, %s", tmp, sub_err);
            return;
        }

        if (curr->prev != NULL) { /*make sure we only have one token after .entry*/
            sprintf(err, "(5): too many arguments after entry declaration. entry must be preceded by exactly one valid label name");
            return;
        }

    } else if (strcmp(tmp, ".string") == 0) {/*.string data line*/
        curr = curr->prev;
        if (curr == NULL) {
            sprintf(err, "(8): .string must be followed by space and then a string");
            return;
        }

        type |= SYM_STR;
        tmp = (char *) curr->data;

        if (tmp[0] != '"' || tmp[strlen(tmp) - 1] != '"') { /*make sure we get a valid string in the token after .string token*/
            sprintf(err, "(9): invalid string declaration, strings must begin and end with exactly 1 '\"' character");
            return;
        }

        curr = curr->prev;

        if (curr != NULL) { /*make sure we only have a string after .string token*/
            sprintf(err,
                    "(10): too many arguments after string declaration. string must be preceded by exactly one valid string");
            return;
        }

    } else if (strcmp(tmp, ".data") == 0) {/*data line*/
        curr = curr->prev;
        if (curr == NULL) {
            sprintf(err, "(11): .data must be followed by space and then a comma separated sequence of valid integers");
            return;
        }

        type |= SYM_DAT;
        need_comma = 0;
        while (curr != NULL) { /*go over all tokens after .data tokens and make sure that they are valid immediate values*/
            tmp = (char *) curr->data;
            if (need_comma) { /*make sure there is a comma between each immediate value*/
                if (tmp[0] != ',') {
                    sprintf(err, "(53): all numbers in .data declaration must be separated by ',' character");
                    return;
                }
            } else {
                if (!is_valid_imm(tmp, sub_err,1)) {
                    sprintf(err, "(12): invalid number at .data declaration \"%s\", %s", tmp, sub_err);
                    return;
                }
            }
            curr = curr->prev;
            need_comma = !need_comma;
        }
        if (!need_comma) {
            sprintf(err, "(52): cannot end .data declaraion with ',' character");
            return;
        }

    } else {/*instruction line*/
        type |= SYM_COD;
        if (!is_valid_opcode(tmp, &opcode, &optype)) {
            sprintf(err, "(13): invalid opcode \"%s\"", tmp);
            return;
        }
        curr = curr->prev;

        if (optype == OPTYPE_INS) { /*make sure we dont have any tokens after a no operand instruction*/
            if (curr != NULL) {
                sprintf(err, "(14): too many argument for opcode %d, expected 0 arguments.", opcode);
                return;
            }

        } else if (optype == OPTYPE_UNI) { /*make sure we have 1 operand after unary instruction*/
            if (curr == NULL) {
                sprintf(err, "(15): too few argument for opcode %d, expected 1 argument.", opcode);
                return;
            }
            tmp = (char *) curr->data;
            /*validate unary instruction operand*/
            if (tmp[0] == '#') {
                if (opcode == 12) { /*is prn opcode we can have any operand type*/
                    if (!is_valid_imm(tmp, sub_err,0)) {/*verify that immediate is valid immediate*/
                        sprintf(err, "(16): invalid immediate value \"%s\", %s", tmp, sub_err);
                        return;
                    }

                } else {/*otherwise, all other unary instructions can only accept label or register as operand*/
                    sprintf(err, "(17): cannot pass immediate value to opcode %d", opcode);
                    return;
                }

            } else if (tmp[0] == 'r') {
                if (!is_valid_reg(tmp, sub_err)) {
                    sprintf(err, "(18): invalid register name \"%s\", %s", tmp, sub_err);
                    return;
                }

            } else {
                if (!is_valid_label(tmp, sub_err)) {
                    sprintf(err, "(19): invalid label \"%s\", %s", tmp, sub_err);
                    return;
                }
            }
            /*make sure we don't have more than 1 operand*/
            curr = curr->prev;
            if (curr != NULL) {
                sprintf(err, "(20): too many arguments for opcode %d", opcode);
                return;
            }

        } else if (optype == OPTYPE_BIN) {/*make sure we have 2 operands after binary instruction*/
            if (curr == NULL) {
                sprintf(err, "(21): too few argument for opcode %d, expected 2 arguments.", opcode);
                return;
            }
            tmp = (char *) curr->data;

            if (tmp[0] == '#') {
                if (opcode == 6) {/*if is lea opcode we cannot have immediate in the source operand*/
                    sprintf(err, "(22): source operand for opcode 6 must be label");
                    return;
                } else {
                    if (!is_valid_imm(tmp, sub_err,0)) {
                        sprintf(err, "(23): invalid immediate value \"%s\", %s", tmp, sub_err);
                        return;
                    }
                }

            } else if (tmp[0] == 'r') {
                if (opcode == 6) { /*if is lea opcode we cannot have register in the source operand*/
                    sprintf(err, "(24): source operand for opcode 6 must be label");
                    return;
                } else {
                    if (!is_valid_reg(tmp, sub_err)) {
                        sprintf(err, "(25): invalid register name \"%s\", %s", tmp, sub_err);
                        return;
                    }
                }

            } else {
                if (!is_valid_label(tmp, sub_err)) {
                    sprintf(err, "(26) : invalid label \"%s\", %s", tmp, sub_err);
                    return;
                }
            }
            /*check that we have a comma after 1st operand*/
            curr = curr->prev;
            if (curr == NULL) {
                sprintf(err, "(27): too few argument for opcode %d, expected 2 argument.", opcode);
                return;
            }
            tmp = (char *) curr->data;
            if (tmp[0] != ',') {
                sprintf(err, "(50): instruction operands must be separated by ',' character");
                return;
            }
            /*check second operand*/
            curr = curr->prev;
            if (curr == NULL) {
                sprintf(err, "(42): missing 2nd operand");
                return;
            }
            tmp = (char *) curr->data;

            if (tmp[0] == '#') {
                if (opcode == 1) {/*if is cmp opcode we can have nay type of operand in the destination operand*/
                    if (!is_valid_imm(tmp, sub_err,0)) {
                        sprintf(err, "(28): invalid immediate value \"%s\", %s", tmp, sub_err);
                        return;
                    }
                } else {
                    sprintf(err, "(29): cannot pass immediate as destination operand ");
                    return;
                }

            } else if (tmp[0] == 'r') {
                if (!is_valid_reg(tmp, sub_err)) {
                    sprintf(err, "(30): invalid register name \"%s\", %s", tmp, sub_err);
                    return;
                }

            } else {
                if (!is_valid_label(tmp, sub_err)) {
                    sprintf(err, "(31): invalid label \"%s\", %s", tmp, sub_err);
                    return;
                }
            }
            /*make sure we dont have more than 2 operands*/
            curr = curr->prev;
            if (curr != NULL) {
                sprintf(err, "(32): too many arguments for opcode %d", opcode);
                return;
            }

        } else if (optype == OPTYPE_JMP) {/*make sure we have at least 1 token after jump instruction*/
            if (curr == NULL) {
                sprintf(err, "(33): too few argument for opcode %d, expected 1 argument.", opcode);
                return;
            }
            tmp = (char *) curr->data;
            /*validate destination operand*/
            if (tmp[0] == '#') {
                if (!is_valid_imm(tmp, sub_err,0)) {
                    sprintf(err, "(34): invalid immediate value \"%s\", %s", tmp, sub_err);
                    return;
                }

            } else if (tmp[0] == 'r') {
                if (!is_valid_reg(tmp, sub_err)) {
                    sprintf(err, "(35): invalid register name \"%s\", %s", tmp, sub_err);
                    return;
                }

            } else {
                if (!is_valid_label(tmp, sub_err)) {
                    sprintf(err, "(36): invalid label \"%s\", %s", tmp, sub_err);
                    return;
                }
            }
            /*check for jump parameters*/
            curr = curr->prev;
            if (curr == NULL) {/*no params stop checking for errors*/
                return;
            }
            /*if we didn't return then we have params, check that tokens are in order
             * '(', 'param1', 'param2', ')'*/
            tmp = (char *) curr->data;
            if (tmp[0] != '(') {
                sprintf(err, "(37): jump parameters must be inside () brackets");
                return;
            }

            curr = curr->prev;
            if (curr == NULL) {
                sprintf(err, "(38): must add arguments after opening ( bracket.");
                return;
            }
            tmp = (char *) curr->data;
            /*validate 1st param*/
            if (tmp[0] == '#') {
                if (!is_valid_imm(tmp, sub_err,0)) {
                    sprintf(err, "(39): invalid immediate value \"%s\", %s", tmp, sub_err);
                    return;
                }

            } else if (tmp[0] == 'r') {
                if (!is_valid_reg(tmp, sub_err)) {
                    sprintf(err, "(40): invalid register name \"%s\", %s", tmp, sub_err);
                    return;
                }

            } else {
                if (!is_valid_label(tmp, sub_err)) {
                    sprintf(err, "(41): invalid label \"%s\", %s", tmp, sub_err);
                    return;
                }
            }

            /*make sure we have comma token*/
            curr = curr->prev;
            if (curr == NULL) {
                sprintf(err, "(42): missing 2nd jump parameter");
                return;
            }

            tmp = (char *) curr->data;
            if (tmp[0] != ',') {
                sprintf(err, "(43): jump parameters must be separated by ',' character");
                return;
            }

            /*validate 2nd param*/
            curr = curr->prev;
            if (curr == NULL) {
                sprintf(err, "(44): missing jump parameter after ','");
                return;
            }
            tmp = (char *) curr->data;

            if (tmp[0] == '#') {
                if (!is_valid_imm(tmp, sub_err,0)) {
                    sprintf(err, "(45): invalid immediate value \"%s\", %s", tmp, sub_err);
                    return;
                }

            } else if (tmp[0] == 'r') {
                if (!is_valid_reg(tmp, sub_err)) {
                    sprintf(err, "(46): invalid register name \"%s\", %s", tmp, sub_err);
                    return;
                }

            } else {
                if (!is_valid_label(tmp, sub_err)) {
                    sprintf(err, "(47): invalid label \"%s\", %s", tmp, sub_err);
                    return;
                }
            }
            /*make sure we have ) token*/
            curr = curr->prev;
            if (curr == NULL) {
                sprintf(err, "(48): must close jump parameters with ')' character");
                return;
            }
            tmp = (char *) curr->data;
            if (tmp[0] != ')') {
                sprintf(err, "(49): must close jump parameters with ')' character");
                return;
            }
            /*make sure we don't have any more tokens*/
            curr = curr->prev;
            if (curr != NULL) {
                sprintf(err, "(69): cannot have any arguments after jump paramters");
                return;
            }
        }
    }
}

