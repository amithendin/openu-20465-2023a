/*
 * address.c
 *
 *  Created on: Feb 10, 2023
 *      Author: amit
 */
#include "address.h"

SymbolData *new_sym_dat(int addr, byte type) {
    SymbolData *sd;
    sd = malloc(sizeof(SymbolData)); /*allocate memory for new data symbol*/
    /*set given paramters and initialize list*/
    sd->addr = addr;
    sd->type = type;
    sd->other = new_list();
    return sd;
}

void free_sym_dat(SymbolData *sd) {
    if (sd == NULL)/*make sure we got symbol data*/
        return;
    free_list(sd->other);/*free others list incase it was used*/
    free(sd);/*for symbol data*/
}

void address_labels(HashTable *labels, unsigned int *instruction_counter, unsigned int *data_counter,
                    const char *in_file_path) {
    FILE *in_file;/*input file handle*/
    char line[LINE_SIZE];/*to hold line read from file*/

    byte type;/*to hold type of label*/
    char tokens[MAX_TOKENS_PER_LINE][MAX_TOKEN_LEN];/*to hold tokens the line breaks into*/
    int num_tokens, i, j;/*counter and iterators*/

    char *label_name;/*to hold name of label*/
    unsigned int ic, dc, curr_tok;/*IC, DC, current token we are processing*/

    SymbolData *label_data, *tmp;/*to hold temporary pointers*/
    Entry *ent;/*iterator to got over labels ht*/

    /*open input file in read mode*/
    in_file = fopen(in_file_path, "r");
    if (in_file == NULL) {
        printf("Error opening in file!\n");
        return;
    }

    ic = dc = 0;/*zero counters*/

    while (fgets(line, sizeof(line), in_file)) {/*iterate over all lines in file*/
        for (i = 0; i < MAX_TOKENS_PER_LINE; i++) {/*zero all tokens so as not to get contaminents from previous lines*/
            for (j = 0; j < MAX_TOKEN_LEN; j++) {
                tokens[i][j] = 0;
            }
        }

        /*parse the read line into tokens*/
        parse_line(line, &type, tokens, &num_tokens);

        curr_tok = 0;/*zero current token counter*/
        if (type & SYM_EXT) {
            /*if first token is an extern label, create new symbol data object for it
             * and intsert it into the labels table with it's name as the key*/
            label_data = new_sym_dat(0, type);
            label_name = (char *) malloc(strlen(tokens[curr_tok]));
            strcpy(label_name, tokens[curr_tok]);

            /*free the previous label symbole data incase we ovewritten it*/
            tmp = (SymbolData *) ht_put(labels, label_name, label_data);
            free_sym_dat(tmp);

        } else if ((type & SYM_ENT)) {
            /* if first symbol is an entry label, do the same os before except instead of
             * ovewritting the existing symbol data for this label name, just turn on the
             * SYM_ENT bit in it's type byte to mark it as entry label*/
            label_data = (SymbolData *) ht_get(labels, tokens[curr_tok]);
            if (label_data == NULL) {
                label_data = new_sym_dat(-1, type);
                label_name = (char *) malloc(strlen(tokens[curr_tok]));
                strcpy(label_name, tokens[curr_tok]);

                /*not supposed to return anything but just incase to make double sure we dont leak memory*/
                tmp = ht_put(labels, label_name, label_data);
                free_sym_dat(tmp);
            }
            /*turn on correct bit according to it's type*/
            label_data->type |= type;

        } else if (type & SYM_DEF) {
            /*if is a label definition do the same as entry but keep a pointer to it's symbol data
             * because it's type will be dicovered in the following tokens*/
            label_data = (SymbolData *) ht_get(labels, tokens[curr_tok]);
            if (label_data == NULL) {
                label_data = new_sym_dat(ic, type);
                label_name = (char *) malloc(strlen(tokens[curr_tok]));
                strcpy(label_name, tokens[curr_tok]);

                /*not supposed to return anything but just incase to make double sure we dont leak memory*/
                tmp = ht_put(labels, label_name, label_data);
                free_sym_dat(tmp);
            }
            /*turn on correct bit according to it's type*/
            label_data->type |= type;
            /*increase IC*/
            curr_tok++;
        }

        if (type & SYM_STR) {
            /*if the next token is a string data token, set it's type and address accordingly and
             * increase the DC by the length of the token since in this assembly
             * language we use ASCII and assign each character 1 word(not byte) in memory*/
            label_data->addr = dc;
            dc += strlen(tokens[curr_tok]);/*add 1 for '\0'*/

        } else if (type & SYM_DAT) {
            /*if the next token is data token then do the same as string but count the next tokens instead of
             * the length of the current tokens*/
            label_data->addr = dc;
            for (; curr_tok < num_tokens; curr_tok++) {
                dc++;
            }

        } else if (type & SYM_COD) {
            /*if the next token is an opcode counter the number of words needed to encode
             * it including it's operands and the double register operand sharing a word
             * situation thingy*/
            for (; curr_tok < num_tokens; curr_tok++) {
                if (tokens[curr_tok][0] == 'r') {
                    if (curr_tok + 1 < num_tokens && tokens[curr_tok + 1][0] == 'r') {/*to not count the double reg thing twice*/
                        curr_tok++;
                    }
                }
                ic++;
            }
        }

    }

    /*go over the entire labels table we just created and add IC to the address of each data label
     * in the table in oderder to make sure the get addressed at the end of the code file to separate
     * code and data*/
    for (i = 0; i < labels->size; i++) {
        ent = labels->entries[i];
        while (ent != NULL) {
            label_data = (SymbolData *) ent->value;
            if (label_data->type & (SYM_DAT | SYM_STR)) {/*add ic to dc to separate data and instructions*/
                label_data->addr += ic;
            }
            ent = ent->next;
        }
    }

    /*set IC and DC we found so they dont need to be recalculated in following passes*/
    *instruction_counter = ic;
    *data_counter = dc;
}
