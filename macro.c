/*
 * macro.c
 *
 *  Created on: Jan 23, 2023
 *      Author: amit
 */
#include "macro.h"

void expand_macros(const char* in_file_path, const char* out_file_path) {
	/*file handles*/
    FILE *in_file;
	FILE *out_file;

	HashTable* macro_table;/*to store defined macro code*/
	Entry* curr;/*iterator in ht*/
    Node *curr_line;/*iterator in list*/
	int in_mcr,i;/*flag and iterator*/

    /*tmp variables for short term line storage*/
	char line[LINE_SIZE], line_cpy[LINE_SIZE], *token, *tmp;
    List *macro_code, *macro;
	int token_len;

    /*open input file in read mode*/
	in_file = fopen(in_file_path, "r");
	if (in_file == NULL) {
		printf("Error opening in file %s\n",in_file_path);
		return;
	}
    /*open output file in write mode inorder to create/ovewrite it*/
	out_file = fopen(out_file_path, "w");
	if (out_file == NULL) {
		printf("Error opening out file!\n");
		return;
	}
    /*close and reopen output file in append mode since we need append mode but
     * we first open in write mode in order to clear the file contents before appending the new lines*/
	fclose(out_file);
	out_file = fopen(out_file_path, "a");

	macro_table = new_hashtable(100);/*init hashtable*/
	in_mcr = 0;/*in macro flag, to indicate if currently loaded line is part of macro code or regular code*/

	while (fgets(line, sizeof(line), in_file)) { /*iterate lines of input file*/
		strcpy(line_cpy,line);/*copy line*/
		token = strtok(line, " ");/*split line by space*/
		token_len = strlen(token);
        /*if we got a label definition, continue to nex part of line*/
		if (token[token_len-1] == ':') {
			token = strtok(NULL, " ");
		}
        /*remove whitespace*/
		token = trim(token);

		if (in_mcr) {/*if we are in macro definition, append lines to macro entry in hashtable*/
			if (strcmp(token, "endmcr") == 0) {/*if line signals end of macro definition, ommit it and turn off flag*/
				in_mcr = 0;

			}else {/*else append to the list of lines of the macro which is pointed to by macro_code pointer*/
                tmp = malloc(LINE_SIZE);
				strcpy(tmp, trim_left(line_cpy));/*copy whitepace-less version of line to a new block of memory*/
                l_push(macro_code, tmp);/*append new block of memory to macro_code list*/
            }

		}else { /*if we aren't in macro definition*/

            /*check if the current line is a call to the macro, if so, insert the macro code
             * lines into the file instead of the macro name*/
			macro = (List*)ht_get(macro_table, token);

			if (macro != NULL) {
				/*fputc('\n', out_file);*/
                curr_line = macro->head;
                while(curr_line!=NULL) {
                    fputs((char *) curr_line->data, out_file);
                    curr_line = curr_line->next;
                }
			} else if (strcmp(token, "mcr") == 0) {
                /*if current line isn't a call to a macro, check if it's a macro definition, if so
                 * then insert a new entry tp the macro ht with macro name as the key and lines list as the value
                 * and turn on the in macro flag in order to append the follwing lines as macro lines*/
				in_mcr = 1;
				macro_code = new_list();/*make new lines list*/
				token = strtok(NULL, " ");/*extract macro name after the macro definition*/
				token = trim_right(token, token_len);
				ht_put(macro_table, token, macro_code);/*insert name and list into ht*/

			} else {/*otherwise we are at a non-macro related line, so just copy it to new file as is*/
				fputs(line_cpy, out_file);
			}
		}
	}
    /*free the macro table along with the lines lists in each entry*/
	for(i=0; i<macro_table->size; i++) {
		curr = macro_table->entries[i];

		while(macro != NULL) {
            macro_code = (List*)curr->value;
            curr_line = macro_code->head;
            while(curr_line != NULL) {
                free(curr_line->data);
                curr_line = curr_line->next;
            }
            free_list(macro_code);
			curr = curr->next;
		}
	}

	free_hashtable(macro_table);
    /*close the files*/
	fclose(in_file);
	fclose(out_file);
}
