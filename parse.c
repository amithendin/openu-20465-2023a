/*
 * parse.c
 *
 *  Created on: Mar 1, 2023
 *      Author: amit
 */
#include "parse.h"

List *tokenize(char *line) {
    List *tokens; /*list of tokens that resulted*/
    /*tmp storage and flags*/
    char *i, *tmp, tok[LINE_SIZE];
    int k, in_str;
    /*initialize variables*/
    tokens = new_list();
    in_str = k = 0;

    for (i = line; *i != '\0'; i++) {/*iterate over characters in the line*/
        if (*i == '"') {/*if we are entering or exiting string, save current token to the list and flip the in string flag*/
            if (k > 0) { /*if token len > 0 than we have a token to save*/
                tmp = (char *) malloc(k + 2);
                strncpy(tmp, tok, k);
                if (in_str) { /*if add " to the token so that we know it's a string token*/
                    tmp[k] = '\"';
                    tmp[k + 1] = '\0';
                } else {
                    tmp[k] = '\0';
                }
                l_push(tokens, tmp);
            }
            k = 0;/*zero token length because we are starting a new token*/
            in_str = !in_str;
        }

        if (!in_str) { /*if we are not in a string parse the characters normally as tokens*/
            if (*i == ',' || *i == ':' || *i == '(' || *i == ')') {/*if we hit a single character token, add it and the current token we accumulated to the list*/
                if (k > 0) {/*if token len > 0 than we have a token to save*/
                    tmp = (char *) malloc(k + 1);
                    strncpy(tmp, tok, k);
                    tmp[k] = '\0';
                    l_push(tokens, tmp);
                }
                k = 0;/*zero token length because we are starting a new token*/
                /*add single character token*/
                tmp = (char *) malloc(2);
                tmp[0] = *i;
                tmp[1] = '\0';
                l_push(tokens, tmp);

            } else if (isspace(*i)) {
                if (k > 0) {/*if token len > 0 than we have a token to save*/
                    tmp = (char *) malloc(k + 1);
                    strncpy(tmp, tok, k);
                    tmp[k] = '\0';
                    l_push(tokens, tmp);
                }
                k = 0;/*zero token length because we are starting a new token*/
            } else if (*i != '"') {
                tok[k++] = *i;/*if we just entered string, save the " so we can validate string*/
            }
        } else {/*else, we are in a string and we dont inspect the chars we just copy them as is to the string token until we hit a " char*/
            tok[k++] = *i;
        }

    }
    if (k > 0) {/*take remaining token if there is any*/
        tmp = (char *) malloc(k + 1);
        strncpy(tmp, tok, k);
        tmp[k] = '\0';
        l_push(tokens, tmp);
    }

    return tokens;
}

int get_opcode(char *opcode, unsigned long *len) {
    *len = 3; /*assume not stop opcode*/
    /*convert opcode to int using simple if-else-if logic*/
    if (strncmp(opcode, "mov", 3) == 0) {
        return 0;

    } else if (strncmp(opcode, "cmp", 3) == 0) {
        return 1;

    } else if (strncmp(opcode, "add", 3) == 0) {
        return 2;

    } else if (strncmp(opcode, "sub", 3) == 0) {
        return 3;

    } else if (strncmp(opcode, "not", 3) == 0) {
        return 4;

    } else if (strncmp(opcode, "clr", 3) == 0) {
        return 5;

    } else if (strncmp(opcode, "lea", 3) == 0) {
        return 6;

    } else if (strncmp(opcode, "inc", 3) == 0) {
        return 7;

    } else if (strncmp(opcode, "dec", 3) == 0) {
        return 8;

    } else if (strncmp(opcode, "jmp", 3) == 0) {
        return 9;

    } else if (strncmp(opcode, "bne", 3) == 0) {
        return 10;

    } else if (strncmp(opcode, "red", 3) == 0) {
        return 11;

    } else if (strncmp(opcode, "prn", 3) == 0) {
        return 12;

    } else if (strncmp(opcode, "jsr", 3) == 0) {
        return 13;

    } else if (strncmp(opcode, "rts", 3) == 0) {
        return 14;

    } else if (strncmp(opcode, "stop", 4) == 0) {
        *len = 4;
        return 15;

    } else {
        return -1;/*else we got an unrecognized opcode*/
    }
}

void parse_tok(char *start, char *end, char *type_val) {
    int k;/*typeval string length*/

    k = 0;
    /*remove whitespace and separator tokens from the start of the string*/
    while (isspace(*end) && end > start) end--;
    while ((*start == ',' || *start == '(' || isspace(*start)) && start < end) start++;

    if (*end == '(') {/*label jump address*/
        type_val[k++] = 'l';
        for (; start < end; start++) {
            if (is_alpha_num(*start)) {
                type_val[k++] = *start;
            }
        }

    } else if (*start == 'r') {/*register*/
        type_val[k++] = 'r';
        for (; start < end; start++) {
            if (is_num(*start)) {
                type_val[k++] = *start;
            }
        }

    } else if (*start == '#') {/*immediate*/
        type_val[k++] = 'i';
        for (; start < end; start++) {
            if (is_alpha_num(*start) || *start == '-') {
                type_val[k++] = *start;
            }
        }

    } else { /*label*/
        type_val[k++] = 'l';
        for (; start < end; start++) {
            if (is_alpha_num(*start)) {
                type_val[k++] = *start;
            }
        }
    }

    type_val[k++] = '\0';/*make sure string is properly terminated*/
}

void parse_line(char *line, byte *type, char tokens[MAX_TOKENS_PER_LINE][MAX_TOKEN_LEN], int *num_tokens) {
    /*tmp storage and flags*/
    char type_val[LINE_SIZE], *tok, *i;
    int k;
    int token_len;

    *type = 0;
    *num_tokens = 0;
    k = 0;
    tok = strtok(line, " ");/*grab first word of the line*/
    tok = trim(tok);

    if (strlen(tok) == 0) {/*ignore whitespace line*/
        *type = 0;
        return;

    } else if (tok[0] == ';') {/*ignore comment line*/
        *type = 0;
        return;

    } else if (tok[strlen(tok) - 1] == ':') {/*label definition*/
        tok = trim(tok);/*copy label name we are defining*/
        strncpy(tokens[k++], tok, strlen(tok) - 1);
        tok = strtok(NULL, " ");
        *type |= SYM_DEF;/*turn on SYM_DEF bit int the type byte to mark token as label definition*/
    }

    if (strncmp(tok, ".extern", 7) == 0) {/*.extern definition*/
        *type |= SYM_EXT;/*turn on SYM_EXT bit int the type byte to mark token as external label*/
        tok = strtok(NULL, " ");
        tok = trim(tok);
        strcpy(tokens[k++], tok);/*copy token string*/
        *num_tokens = k;
        return;

    } else if (strncmp(tok, ".entry", 6) == 0) {/*.entry defintion*/
        *type |= SYM_ENT;/*turn on SYM_ENT bit int the type byte to mark token as entry label*/
        tok = strtok(NULL, " ");
        tok = trim(tok);
        strcpy(tokens[k++], tok);/*copy token string*/
        *num_tokens = k;
        return;

    } else if (strncmp(tok, ".string", 7) == 0) {/*.string data line*/
        *type |= SYM_STR;/*turn on SYM_STR bit int the type byte to mark token as string data*/
        tok = strtok(NULL, " ");
        tok = trim(tok);
        strncpy(tokens[k++], tok + 1, strlen(tok) - 2);/*copy token string without " chars*/

    } else if (strncmp(tok, ".data", 5) == 0) {/*.data data line*/
        *type |= SYM_DAT;/*turn on SYM_DAT bit int the type byte to mark token as data*/
        tok = strtok(NULL, "\"");
        token_len = 0;
        while (tok != NULL) {/*iterate following tokens and copy each 1 to tokens array*/
            for (i = tok; *tok != '\0'; tok++) {/*iterate word because there maybe more than 1 tokens in 1 word like 23,21,1,7,.. instead of  23, 21, 1, 7,..*/
                if (!isspace(*tok)) {
                    token_len += 1;
                }
                if (*tok == ',') {
                    /*parse data*/
                    strncpy(tokens[k++], i, token_len - 1);
                    i = tok + 1;
                    token_len = 0;
                }
            }
            if (i < tok) {
                strncpy(tokens[k++], i, tok - i);/*make sure not to leave last number behind*/
            }
            tok = strtok(NULL, " ");/*next token*/
        }

    } else {/*instruction line*/
        *type |= SYM_COD;/*turn on SYM_STR bit int the type byte to mark token as opcode*/
        strcpy(tokens[k++], tok);/*copy opcode token*/
        tok = strtok(NULL, " ");/*move to next word in string becuase opcode and operands must have space separating them*/
        if (tok != NULL) {/*iterate over all remaining words in the line and all tokens within each word to fully extract all the operands into individual tokens*/
            while (tok != NULL) {
                i = tok;

                for (; *i != '\0'; i++) {/*check for multiple tokens in same word*/
                    if (*i == '(' || *i == ',') {
                        parse_tok(tok, i, type_val);
                        if (strlen(trim(type_val)) > 1) {
                            strcpy(tokens[k++], type_val);/*if is valid operand, copy it*/
                        }
                        tok = i;
                    }
                }
                /*do check again to leave no operand behind*/
                parse_tok(tok, i, type_val);

                if (strlen(trim(type_val)) > 1) {
                    strcpy(tokens[k++], type_val);
                }

                tok = strtok(NULL, " ");
            }
        }
    }

    *num_tokens = k;/*set the number of tokens we found*/
}

