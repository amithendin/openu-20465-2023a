/*
 * validate.c
 *
 *  Created on: Mar 2, 2023
 *      Author: amit
 */
#include "validate.h"

int is_valid_opcode(char *str, int *opcode, byte *optype) {
    unsigned long opcode_len;
    int op;

    op = get_opcode(str, &opcode_len);
    if (op == -1) {
        return 0;
    }

    if (op == 6 || (op >= 0 && op <= 3)) {
        *optype = OPTYPE_BIN;
    } else if (op == 4 || op == 5 || op == 7
               || op == 8 || op == 11 || op == 12) {
        *optype = OPTYPE_UNI;
    } else if (op == 9 || op == 10 || op == 13) {
        *optype = OPTYPE_JMP;
    } else {
        *optype = OPTYPE_INS;
    }

    *opcode = op;

    return 1;
}

int is_valid_imm(char *str, char *err) {
    long num = 0;
    unsigned long i, j, len;
    len = strlen(str);

    j = 0;
    if (str[0] == '#') {
        j = 1;
    }

    if (!is_num(str[j])) {
        sprintf(err, "numbers must start with digit or + or -");
        return 0;
    }
    i = str[j] == '-' ? j + 1 : j;

    for (; i < len && str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') {
            sprintf(err, "numbers can only contain digits");
            return 0;
        }

        num *= 10;
        num += str[i];
    }

    if (num >> (ASM_WORD_SIZE - 3)) {/*sub 2 for ERA and 1 for sign*/
        sprintf(err, "number too large for %d bits", ASM_WORD_SIZE - 2);
        return 0;
    }

    return 1;
}

int is_valid_reg(char *str, char *err) {
    if (str[0] != 'r') {
        sprintf(err, "registers must begin with the letter 'r'");
        return 0;
    }

    if (str[1] < '0' || str[1] > '7' || strlen(str) > 2) {
        sprintf(err, "no such register");
        return 0;
    }

    return 1;
}

int is_valid_label(char *str, char *err) {
    unsigned int i, len;

    len = strlen(str);
    if (len > LABEL_SIZE || len == 0) {
        sprintf(err, "labels must be no more than 30 characters long and no less than 1");
        return 0;
    }

    if (!is_alpha(str[0])) {
        sprintf(err, "labels must start with letter");
        return 0;
    }

    for (i = 1; i < len; i++) {
        if (!is_alpha_num(str[i])) {
            sprintf(err, "labels can only contain alphanumeric characters");
            return 0;
        }
    }

    return 1;
}

void validate_code(const char *code_file_path, char *err) {
    char line[LINE_SIZE + 10];
    FILE *in_file;
    List *tokens;
    char tok_err[ERR_SIZE];
    Node *curr;
    unsigned int line_num, line_len, i;

    in_file = fopen(code_file_path, "r");
    if (in_file == NULL) {
        sprintf(err, "(0) failed to open file %s", code_file_path);
        return;
    }
    line_num = 1;
    memset(tok_err, '\0', ERR_SIZE);

    while (fgets(line, sizeof(line), in_file)) {
        line_len = strlen(line);

        if (line_len > LINE_SIZE) {
            sprintf(err, "line %d code (0): exeeded maximum line size of %d characters", line_num, LINE_SIZE);
            return;
        }

        if (line[0] == ';') {/*comment line*/
            continue;
        }
        for (i = 0; i < line_len; i++) {
            if (!isspace(line[i])) {
                break;
            }
        }
        if (i >= line_len) { /*whitespace line*/
            continue;
        }

        tokens = tokenize(line);
        validate_tokens(tokens, tok_err);
        curr = tokens->tail;
        while (curr) {
            free(curr->data);
            curr = curr->prev;
        }
        free_list(tokens);

        if (strlen(tok_err) > 0) {
            sprintf(err, "line %d code %s", line_num, tok_err);
            return;
        }
        line_num++;

    }

    fclose(in_file);
}

void validate_tokens(List *tokens, char *err) {
    byte type, optype, need_comma;
    int opcode;
    Node *curr;
    char *tmp, sub_err[ERR_SIZE / 2];
    curr = tokens->tail;

    need_comma = type = optype = opcode = 0;

    if (curr->prev != NULL && strcmp((char *) curr->prev->data, ":") == 0) {/*label definition*/
        tmp = (char *) curr->data;
        if (!is_valid_label(tmp, sub_err)) {
            sprintf(err, "(1): invalid label name \"%s\" at definition, %s", tmp, sub_err);
            return;
        }
        curr = curr->prev->prev;/*skip to next token after the 2 tokens needed to make definitions*/
        type |= SYM_DEF;
    }

    if (curr == NULL) {
        sprintf(err, "(2): expected instruction or data after label definition");
        return;
    }
    tmp = (char *) curr->data;
    if (strcmp(tmp, ".extern") == 0) {/*.extern definition*/
        curr = curr->prev;
        if (curr == NULL) {
            sprintf(err, "(3): .extern must be followed by space and then label");
            return;
        }
        tmp = (char *) curr->data;
        type |= SYM_EXT;

        if (!is_valid_label(tmp, sub_err)) {
            sprintf(err, "(4): invalid label name \"%s\" at external declaration, %s", tmp, sub_err);
            return;
        }

        if (curr->prev != NULL) {
            sprintf(err,
                    "(5): too many arguments after external declaration. extern must be preceded by exactly one valid label name");
            return;
        }

    } else if (strcmp(tmp, ".entry") == 0) {/*.entry defintion*/
        curr = curr->prev;
        if (curr == NULL) {
            sprintf(err, "(3): .entry must be followed by space and then label");
            return;
        }
        tmp = (char *) curr->data;
        type |= SYM_EXT;

        if (!is_valid_label(tmp, sub_err)) {
            sprintf(err, "(4): invalid label name \"%s\" at entry declaration, %s", tmp, sub_err);
            return;
        }

        if (curr->prev != NULL) {
            sprintf(err,
                    "(5): too many arguments after entry declaration. entry must be preceded by exactly one valid label name");
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

        if (tmp[0] != '"' || tmp[strlen(tmp) - 1] != '"') {
            sprintf(err, "(9): invalid string declaration, strings must begin and end with exactly 1 '\"' character");
            return;
        }

        curr = curr->prev;

        if (curr != NULL) {
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
        while (curr != NULL) {
            tmp = (char *) curr->data;
            if (need_comma) {
                if (tmp[0] != ',') {
                    sprintf(err, "(53): all numbers in .data declaration must be separated by ',' character");
                    return;
                }
            } else {
                if (!is_valid_imm(tmp, sub_err)) {
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

        if (optype == OPTYPE_INS) {
            if (curr != NULL) {
                sprintf(err, "(14): too many argument for opcode %d, expected 0 arguments.", opcode);
                return;
            }

        } else if (optype == OPTYPE_UNI) {
            if (curr == NULL) {
                sprintf(err, "(15): too few argument for opcode %d, expected 1 argument.", opcode);
                return;
            }
            tmp = (char *) curr->data;

            if (tmp[0] == '#') {
                if (opcode == 12) { /*is prn opcode*/
                    if (!is_valid_imm(tmp, sub_err)) {
                        sprintf(err, "(16): invalid immediate value \"%s\", %s", tmp, sub_err);
                        return;
                    }

                } else {
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

            curr = curr->prev;

            if (curr != NULL) {
                sprintf(err, "(20): too many arguments for opcode %d", opcode);
                return;
            }

        } else if (optype == OPTYPE_BIN) {
            if (curr == NULL) {
                sprintf(err, "(21): too few argument for opcode %d, expected 2 arguments.", opcode);
                return;
            }
            tmp = (char *) curr->data;

            if (tmp[0] == '#') {
                if (opcode == 6) {/*is lea opcode*/
                    sprintf(err, "(22): source operand for opcode 6 must be label");
                    return;
                } else {
                    if (!is_valid_imm(tmp, sub_err)) {
                        sprintf(err, "(23): invalid immediate value \"%s\", %s", tmp, sub_err);
                        return;
                    }
                }

            } else if (tmp[0] == 'r') {
                if (opcode == 6) {
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

            curr = curr->prev;
            if (curr == NULL) {
                sprintf(err, "(42): missing 2nd operand");
                return;
            }
            tmp = (char *) curr->data;


            if (tmp[0] == '#') {
                if (opcode == 1) {/*is cmp opcode*/
                    if (!is_valid_imm(tmp, sub_err)) {
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

            curr = curr->prev;

            if (curr != NULL) {
                sprintf(err, "(32): too many arguments for opcode %d", opcode);
                return;
            }

        } else if (optype == OPTYPE_JMP) {
            if (curr == NULL) {
                sprintf(err, "(33): too few argument for opcode %d, expected 1 argument.", opcode);
                return;
            }
            tmp = (char *) curr->data;

            if (tmp[0] == '#') {
                if (!is_valid_imm(tmp, sub_err)) {
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

            /*check for arguments*/
            curr = curr->prev;
            if (curr == NULL) {/*no params stop checking for errors*/
                return;
            }

            tmp = (char *) curr->data;/*then we have params*/
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

            if (tmp[0] == '#') {
                if (!is_valid_imm(tmp, sub_err)) {
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

            curr = curr->prev;

            if (curr == NULL) {
                sprintf(err, "(44): missing jump parameter after ','");
                return;
            }
            tmp = (char *) curr->data;

            if (tmp[0] == '#') {
                if (!is_valid_imm(tmp, sub_err)) {
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
        }
    }
}

