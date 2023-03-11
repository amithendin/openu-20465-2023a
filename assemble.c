/*
 * assemble.c
 *
 *  Created on: Mar 2, 2023
 *      Author: amit
 */
#include "assemble.h"

void int_to_bin(long num, int bits, char *bin) {
    int k, i, neg;/*flag and iterators*/
    k = 0;/*start counting bits from 0*/
    neg = num < 0;/*set flag to 1 if num is negative*/
    if (neg) { /*make sure we work on positive numbers in this step*/
        num *= -1;
    }
    while (num > 0 &&
           k < bits) {/*keep diving by 2 and adding the remainder to encod integer into bits until integer reaches 0*/
        bin[k] = num % 2;/*place bits into provided bin array*/
        num /= 2;
        k++;/*count bits*/
    }
    if (neg) {/*if number was originally negative, preform two's complement convertion on the allotted bits*/
        for (i = 0; i < bits; i++) {
            bin[i] = !bin[i];/*flip bits*/
        }
        /*add 1*/
        bin[0] += 1;
        for (i = 0; i < bits - 1 && bin[i] > 1; i++) {
            bin[i] = 0;
            bin[i + 1] += 1;
        }
    }
}

int get_addr_type(char *operand) {/*operand is token from parse function*/
    if (operand[0] == 'i') return 0; /*immediate tokens start with i*/
    if (operand[0] == 'l') return 1; /*label tokens start with l*/
    if (operand[0] == 'r') return 3; /*register tokens start with r*/
    return -1; /*default*/
}

/**
 * Zero the array
 *
 * @param word
 */
void clear_word(char word[ASM_WORD_SIZE]) {
    int i;
    for (i = 0; i < ASM_WORD_SIZE; i++) {/*self-explanatory*/
        word[i] = 0;
    }
}

/**
 * Converts array of 1 and 0 bytes into their character representation and writes
 * those characters to file stream
 *
 * @param word an array of signed bytes that are 1 or 0 in their values
 * @param obj_file a file to write their character representation into
 */
void print_word(char word[ASM_WORD_SIZE], FILE *obj_file) {
    int i;
    for (i = ASM_WORD_SIZE - 1; i >= 0; i--) {
        fputc(word[i] ? ONE_CHAR : ZERO_CHAR, obj_file);
    }
}

/**
 * Shortcut for using print_word
 *
 * @param n integer to convert into bits character representation
 * @param obj_file fiel to write characters into
 */
void print_int_as_word(long n, FILE *obj_file) {
    char data_word[ASM_WORD_SIZE];
    clear_word(data_word); /*zero word bytes*/
    int_to_bin(n, ASM_WORD_SIZE, data_word);/*fill with integer binary rep bits*/
    print_word(data_word, obj_file);/*send bits to file*/
}

void instruction_to_bin(long ic, HashTable *labels, char *opcode, char *operands[3], int num_operands,
                        char bin[4][ASM_WORD_SIZE], int *num_words) {
    int bin_opcode, bin_operands[3], k, offset, word_offset; /*0 - opcode, 1 - num args, bin_opcode - opcode number, offset - offset in the operand array
 * word_offset - offset in the bin matrix*/
    /*tmp shorthand for readability*/
    char *operand;
    SymbolData *label_data;
    byte operands_type[3]; /*save important type info*/
    unsigned long opcode_len; /*just to use get_opcode function correctly*/

    bin_opcode = get_opcode(opcode, &opcode_len);

    for (k = 0;
         k < num_operands; k++) { /*convert operands to integer form so we can send them to int_to_bin for encoding*/
        operand = operands[k];/*for ease of use and readability*/
        /*handle each operand type accordingly*/
        if (operand[0] == 'r') {
            bin_operands[k] = str_to_int(operand + 1);

        } else if (operand[0] == 'i') {
            bin_operands[k] = str_to_int(trim(operand + 1));

        } else if (operand[0] == 'l') {
            label_data = (SymbolData *) ht_get(labels, trim(operand + 1));/*get label data*/

            if (label_data == NULL) {
                printf("error: no such label \"%s\"\n", operand + 1);
                return;
            }

            if (label_data->type &
                SYM_EXT) { /*if we found a external label reference in the code, write the address of it down*/
                l_push(label_data->other, (void *) ic);
            }

            bin_operands[k] = label_data->addr +
                              BASE_ADDRESS;/*set label address to be it's original address plus the address offset defined in the assignment*/
            operands_type[k] = label_data->type;

        } else {
            printf("error: unrecognized operand type \"%c\"\n", *operand);
            return;
        }
    }

    *num_words = 1; /*send number of words to 1 initiazly because that is the minimum*/
    int_to_bin(bin_opcode, 4, bin[0] + 6); /*convert opcode into bits*/

    offset = 0;/*assume no offset needed*/
    word_offset = 1;/*always have at least 1 offset in the bin matrix because of the opcode word*/

    if (num_operands == 3) { /*if 3 params then first one is always jump label*/
        int_to_bin(2, 2, bin[0] + 2);/*ERA bits of jump label in opcode word*/
        int_to_bin(2, 2, bin[1]);/*ERA bits of jump label word*/
        int_to_bin(bin_operands[0], ASM_WORD_SIZE - 2, bin[1] + 2);/*bits of label address in label word*/
        offset = 1;/*set operand offset to 1 because first operand is jump label*/
        word_offset = 2;/*set word offset to 2 because now we have 2 words occupied, 1 for the opcode and 1 for the jump label*/
        /*update the number of words we are using and set the addressing mode of both jump paramters in the param bits of the opcode word*/
        int_to_bin(get_addr_type(operands[offset + 1]), 2, bin[0] + ASM_WORD_SIZE - 4);
        int_to_bin(get_addr_type(operands[offset]), 2, bin[0] + ASM_WORD_SIZE - 2);
        *num_words = 4;
    }

    if (num_operands > 1) {

        /*instruction byte operand addressing bits*/
        if (num_operands ==
            2) {/*if we have exactly 2 operands then is not jump instruction and encode operands normally*/
            int_to_bin(get_addr_type(operands[0]), 2, bin[0] + 4);
            int_to_bin(get_addr_type(operands[1]), 2, bin[0] + 2);
            *num_words = 3;
        }

        /*check for the 2 register operands situation and all the other addressing modes combos and encode
         * each word accordingly*/
        if (operands[offset][0] == 'r' && operands[offset + 1][0] == 'r') {/*for example: r2,r3*/
            int_to_bin(bin_operands[offset + 1], 5, bin[word_offset] + 2);
            int_to_bin(bin_operands[offset], 5, bin[word_offset] + 8);
            *num_words -= 1;/*we need one less word then we assumed so reduce word count*/

        } else if (operands[offset][0] == 'r' && operands[offset + 1][0] != 'r') {/*for example: r3,LOOP or r3,#4*/
            int_to_bin(bin_operands[offset], 5, bin[word_offset] + 8);
            int_to_bin(bin_operands[offset + 1], ASM_WORD_SIZE - 2, bin[word_offset + 1] + 2);

        } else if (operands[offset][0] != 'r' && operands[offset + 1][0] == 'r') {/*for example: LOOP,r3 or #4,r3*/
            int_to_bin(bin_operands[offset], ASM_WORD_SIZE - 2, bin[word_offset] + 2);
            int_to_bin(bin_operands[offset + 1], 5, bin[word_offset + 1] + 2);

        } else {/*for example: LOOP,#4 or LOOP,MAIN, or #4,MAIN or #4,#5*/
            int_to_bin(bin_operands[offset], ASM_WORD_SIZE - 2, bin[word_offset] + 2);
            int_to_bin(bin_operands[offset + 1], ASM_WORD_SIZE - 2, bin[word_offset + 1] + 2);
        }

        if (operands[offset][0] == 'l') {/*ERA bits for labels*/
            int_to_bin(2, 2, bin[word_offset]);
        }
        if (operands[offset + 1][0] == 'l') {/*ERA bits for labels*/
            int_to_bin(2, 2, bin[word_offset + 1]);
        }

    } else if (num_operands == 1) {
        *num_words = 2; /*only need two word for opcodes with 1 operand*/
        if (operands[0][0] == 'l') {/*ERA bits for labels*/
            int_to_bin(2, 2, bin[1]);
        }
        int_to_bin(bin_operands[0], ASM_WORD_SIZE - 2, bin[1] + 2);

        if (bin_opcode !=
            12) { /*only prn instruction can accept all addressing modes, otherwise we have addressing mode 2*/
            /*maybe put get_addr_type(operands[0]) instead of 2*/
            int_to_bin(2, 2, bin[0] + 2);
        }
    }

    /*set ERA bits of external and entry words*/
    for (k = 0; k < num_operands; k++) {
        if (operands_type[k] & SYM_EXT) {
            int_to_bin(1, 2, bin[word_offset + k]);
        } else if (operands_type[k] & SYM_ENT) {
            int_to_bin(2, 2, bin[word_offset + k]);
        }
    }
}

void assemble_code(HashTable *labels, int ic, int dc, const char *in_file_path, const char *obj_file_path,
                   const char *ent_file_path, const char *ext_file_path) {
    FILE *in_file, *obj_file, *ent_file, *ext_file; /*file handles*/
    /*iterators and tmp storage*/
    char *operands[3], *opcode, line[LINE_SIZE], tokens[MAX_TOKENS_PER_LINE][MAX_TOKEN_LEN], bin_instructions[4][ASM_WORD_SIZE];
    int num_operands, num_words, num_tokens, i;
    long curr_ic, j;
    byte type;
    SymbolData *label_data;
    List *data;
    Node *curr;
    Entry *ent;
    /*open input file in read mode*/
    in_file = fopen(in_file_path, "r");
    if (in_file == NULL) {
        printf("Error opening in file!\n");
        return;
    }
    /*open .obj file in append mode*/
    obj_file = open_file_append(obj_file_path);
    ent_file = ext_file = NULL;/*assume no entry and extern labels therefore no need to open files*/
    data = new_list(); /*"data image"*/
    curr_ic = BASE_ADDRESS;/*IC for current pass*/

    /*print ic dc at title of obj file*/
    fprintf(obj_file, "%d %d\n", ic, dc);

    while (fgets(line, sizeof(line), in_file)) {/*iterate over all lines in input file*/
        for (i = 0; i < 4; i++) { /*zero tokens tmp storage so as no to get contamination from prev lines*/
            for (j = 0; j < MAX_TOKEN_LEN; j++) {
                tokens[i][j] = 0;
            }
        }

        parse_line(line, &type, tokens, &num_tokens);/*convert line to tokens*/

        if (type & SYM_STR) {/*handle .string data definitions*/
            for (i = 0; i < strlen(tokens[1]); i++) {
                j = tokens[1][i];
                /* push a word to the data image for every character in the token.
                 * we convert integer to (void*) since pointers are by default the size of a word on the machine
                 * and integers are not more than on word on the machine therefore any integer should fit in the
                 * space needed for an address, therefore in this case the list node data pointer is not uses as
                 * a pointer at all but rather as a word sized integer*/
                l_push(data, (void *) j);
            }
            l_push(data, 0);

        } else if (type & SYM_DAT) {/*handle .data data definitions*/
            for (i = 1; i < num_tokens; i++) {
                j = str_to_int(tokens[i]);
                /* push a word to the data image for every number in the data array
                * same as we did in .string*/
                l_push(data, (void *) j);
            }

        } else if (type & SYM_COD) { /*handle instruction lines*/
            for (i = 0; i < 4; i++) { /*zero bin matrix tmp storage so as no to get contamination from prev lines*/
                for (j = 0; j < ASM_WORD_SIZE; j++) {
                    bin_instructions[i][j] = 0;
                }
            }

            if (type & SYM_DEF) {/*set opcode and operands to skip 1 if is label definition since those are handled in addressing step*/
                opcode = tokens[1];
                operands[0] = tokens[2];
                operands[1] = tokens[3];
                operands[2] = tokens[4];
                num_operands = num_tokens - 2;
            } else { /*otherwise set them in order we expect*/
                opcode = tokens[0];
                operands[0] = tokens[1];
                operands[1] = tokens[2];
                operands[2] = tokens[3];
                num_operands = num_tokens - 1;
            }
            /*convert opcode and operands we found to binary into the bin matrix*/
            instruction_to_bin(curr_ic, labels, opcode, operands, num_operands, bin_instructions, &num_words);

            for (i = 0; i < num_words; i++) {/*append print the bin matrix we just got to the obj file*/
                /*print addresss of word*/
                /*fprintf(obj_file, "%04d ",curr_ic++);*/
                print_int_as_word(curr_ic++, obj_file);/*instruction address column*/
                fputc(' ', obj_file);/*separate columns with space*/
                /*print content of word*/
                print_word(bin_instructions[i], obj_file);/*instruction encoding column*/
                fputc('\n', obj_file);
            }
        }
    }
    /*close input file after we finished reading lines*/
    fclose(in_file);

    /*after we finished writing all the instructions to the object file
     * it's time to write all the data to the object file*/

    curr = data->tail;
    while (curr != NULL) {
        j = (long) curr->data; /*get data number from list using the addresss-integer trick we did previously*/
        /*fprintf(obj_file, "%04d ",curr_ic++);*/
        print_int_as_word(curr_ic++, obj_file); /*print address column*/
        fputc(' ', obj_file);
        print_int_as_word(j, obj_file); /*print data encoding column*/
        fputc('\n', obj_file);
        curr = curr->prev;/*keep iterating*/
    }

    free_list(data);/*clean up list, since data pointers are used as integers and dont point anywhere, it's sufficient to just free the list data*/
    fclose(obj_file);/*close .obj file*/

    /*next we simply collect all the labels marked as entry and write them with
     * thier address to the .ent file and the labels marked extern in the .ext file*/

    for (i = 0; i < labels->size; i++) {
        ent = labels->entries[i];
        while (ent != NULL) {
            label_data = (SymbolData *) ent->value;
            if (label_data->type & SYM_ENT) {/*write entries*/
                if (ent_file == NULL) {/*create file only if we have entry labels*/
                    ent_file = open_file_append(ent_file_path);
                }
                fprintf(ent_file, "%s ", ent->key);
                print_int_as_word(label_data->addr, ent_file);
                fputc('\n', ent_file);

            }
            if (label_data->type & SYM_EXT) {
                curr = label_data->other->head;
                while (curr != NULL) {/*write extern uses*/
                    if (ext_file == NULL) { /*create file only if we have extern labels*/
                        ext_file = open_file_append(ext_file_path);
                    }
                    fprintf(ext_file, "%s ", ent->key);
                    print_int_as_word((long) curr->data, ext_file);
                    fputc('\n', ext_file);
                    curr = curr->next;
                }
            }
            ent = ent->next;
        }
    }

    /*close the files if we used them*/
    if (ent_file) {
        fclose(ent_file);
    }
    if (ext_file) {
        fclose(ext_file);
    }
}
