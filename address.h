/*
 * address.h
 *
 *  Created on: Feb 10, 2023
 *      Author: amit
 */

#ifndef ADDRESS_H
#define ADDRESS_H

#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "list.h"
#include "hashtable.h"
#include "parse.h"

/**
 * Symbol Data struct
 *
 * meant to hold data about a label, likes it's ERA type, name, and so on..
 */
typedef struct {
	int addr; /*the address of the label in the binary encoding*/
	byte type;/*the type of the label, can be any of the SYM_ macro values defined in util.h including bitfields*/
	List* other; /*to store various other data that can be associated with the label like amount of references for external labels*/
} SymbolData;

/**
 * Creates new symbol data object
 *
 * @param addr address of binary encoding of the label
 * @param type the SYM_ type value
 * @return pointer to new symbol data object
 */
SymbolData* new_sym_dat(int addr, byte type);

/**
 * Frees symbol data objects
 *
 * @param sd pointer to symbol data object to free
 */
void free_sym_dat(SymbolData* sd);

/**
 * Calculates the addresses for each label in the binary encoding of the provided assembly code file
 * based on it's place in the code and it's type (wheather it's data label or code label). Uses DC
 * IC counters to keep data and code separate
 *
 * @param labels a table in which to fill the labels and their data from the assembly code
 * @param instruction_counter an IC reference to set the instruction count and data count found during the process of label addressing for efficiency in other methods
 * @param data_counter same instruction_counter but for DC
 * @param in_file_path path to assembly code file *!after expanding macros!*
 */
void address_labels(HashTable *labels, unsigned int* instruction_counter, unsigned int* data_counter,const char* in_file_path);

#endif /* ADDRESS_H */
