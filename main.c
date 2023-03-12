/*
 ============================================================================
 Name        : main.c
 Author      : Amit Hendin
 Version     : 0.0.1
 Copyright   : Copyright 2023 Amit Hendin
 Description : assigment 14 course 20456 semeter 2023a Open University of Israel
 ============================================================================
 */

#include <stdio.h>

#include "hashtable.h"
#include "parse.h"
#include "macro.h"
#include "address.h"
#include "assemble.h"
#include "validate.h"

int main(int argc, char *argv[]) {/*main function*/
    HashTable *labels;/*to store labels for each file*/
    Entry *ent;/*iterator*/
    unsigned int ic, dc, i, j;/*counters*/
    byte is_valid;/*check if line is valid*/
    char as_file_path[MAX_FILE_PATH],/*buffers for file paths*/
            am_file_path[MAX_FILE_PATH],
            obj_file_path[MAX_FILE_PATH],
            ent_file_path[MAX_FILE_PATH],
            ext_file_path[MAX_FILE_PATH];

    for (i = 1; i < argc; i++) {/*iterate over all file names in the command arguments*/
        labels = new_hashtable(400);/*init new ht for each file*/
        ic = dc = 0;/*reset counters*/
        is_valid = 1;/*assume file is valid*/

        printf("assembling %s\n", argv[i]);/*notify user we started assembling the file*/
        /*generate file paths*/
        sprintf(as_file_path, "%s.as", argv[i]);
        sprintf(am_file_path, "%s.am", argv[i]);
        sprintf(obj_file_path, "%s.ob", argv[i]);
        sprintf(ent_file_path, "%s.ent", argv[i]);
        sprintf(ext_file_path, "%s.ext", argv[i]);
        /*preprocessing step, expand macros into new file*/
        expand_macros(as_file_path, am_file_path);
        /*send expanded macro source file to the validation function*/
        validate_code(am_file_path, &is_valid);
        if (!is_valid) {/*if file has errors, contiune to next file*/
            printf("got error(s) in file %s. files not created\n", as_file_path);

        }else {
            /*first pass, generate labels ht and IC and DC from the macro expanded source file*/
            address_labels(labels, &ic, &dc, am_file_path);
            /*second pass, generate binary files from the labels ht and the macro expanded source file*/
            assemble_code(labels, ic, dc, am_file_path, obj_file_path, ent_file_path, ext_file_path);

            /*free labels hashtable memory for the next file assembly*/
            for (j = 0; j < labels->size; j++) {
                ent = labels->entries[j];
                while (ent != NULL) {
                    free_sym_dat((SymbolData *) ent->value);
                    ent = ent->next;
                }
            }
        }
        /*free labels hashtable*/
        free_hashtable(labels);
    }

    return 0;
}

