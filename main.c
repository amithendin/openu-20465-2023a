/*
 ============================================================================
 Name        : assembler.c
 Author      : Amit Hendin
 Version     : 0.0.1
 Copyright   : Copyright 2023 Amit Hendin
 Description : assigment 14 course 20456 Open University of Israel
 ============================================================================
 */

#include <stdio.h>

#include "hashtable.h"
#include "parse.h"
#include "macro.h"
#include "address.h"
#include "assemble.h"
#include "validate.h"

int main(int argc, char *argv[]) {
    HashTable *labels;
    Entry *ent;
    unsigned int ic, dc, i, j;
    char as_file_path[MAX_FILE_PATH],
            am_file_path[MAX_FILE_PATH],
            obj_file_path[MAX_FILE_PATH],
            ent_file_path[MAX_FILE_PATH],
            ext_file_path[MAX_FILE_PATH],
            err[ERR_SIZE];

    for (i = 1; i < argc; i++) {
        labels = new_hashtable(400);
        ic = dc = 0;

        printf("started assembling %s\n", argv[i]);

        sprintf(as_file_path, "%s.as", argv[i]);
        sprintf(am_file_path, "%s.am", argv[i]);
        sprintf(obj_file_path, "%s.ob", argv[i]);
        sprintf(ent_file_path, "%s.ent", argv[i]);
        sprintf(ext_file_path, "%s.ext", argv[i]);

        expand_macros(as_file_path, am_file_path); /*preprocessing step*/
        memset(err, 0, ERR_SIZE);
        validate_code(am_file_path, err);
        if (strlen(err) > 0) {
            printf("got error in file: %s\n%s\n", as_file_path, err);
            return 0;
        }
        address_labels(labels, &ic, &dc, am_file_path); /*first pass*/
        assemble_code(labels, ic, dc, am_file_path, obj_file_path, ent_file_path, ext_file_path);/*second pass*/

        /*free memory for the next file assembly*/
        for (j = 0; j < labels->size; j++) {
            ent = labels->entries[j];
            while (ent != NULL) {
                free_sym_dat((SymbolData *) ent->value);
                ent = ent->next;
            }
        }
        free_hashtable(labels);

        printf("finished assembling %s\n", argv[i]);
    }

    return 0;
}

