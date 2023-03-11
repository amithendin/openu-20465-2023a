/*
 * assemble.h
 *
 *  Created on: Mar 2, 2023
 *      Author: amit
 */

#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "hashtable.h"
#include "list.h"
#include "parse.h"
#include "address.h"

/**
 * Converts a long integer to bit characters and puts n of those bits in the
 * first n indexes of the provided character array. does a 2's complement
 * binary conversion on the bits if the number is negative *!based on n!*
 * meaning it will overwrite n bits of the character array.
 * 0 and 1 bit characters in the bits string are represented by the characters
 * set in macros ZERO_CHAR and ONE_CHAR respectively
 *
 * @param num the number to convert to binary
 * @param bits the n bits to write
 * @param bin the array of characters to write the bits into
 */
void int_to_bin(long num, int bits, char* bin);

/**
 * Get the addressing mode of an operand in it's string representation (e.g from the source code)
 *
 * @param operand the string representation of the operand
 * @return 0,1,2 based on the addressing mode of the operand, -1 if no addressing mode defined for such operand
 */
int get_addr_type(char* operand);

/**
 * Converts an instruction from it's string representation into it's final binary encoding
 *
 * @param ic the IC counter from the current pass
 * @param labels the labels hashtable generated in addressing step
 * @param opcode the string representation of the opcode
 * @param operands the string representation of the operands
 * @param num_operands the number of operands in the operands parameter
 * @param bin a clear (set to 0) matrix of 4xASM_WORD_SIZE characters where the final representation of the instruction will be put
 * @param num_words the number of words in the bin matrix the instruction uses in it's final binary encoding
 */
void instruction_to_bin(long ic, HashTable *labels, char *opcode, char *operands[3], int num_operands, char bin[4][ASM_WORD_SIZE], int *num_words);

/**
 * Generates the and .obj, .ent, and .ext files based on a provided *!macro expanded!* .am source code file
 * as specified in the assignment description. files will be written the their respective file paths
 * and will overwrite the file at that path currently.
 *
 * @param labels the labels hashtable generated in addressing step
 * @param ic the IC counter from the addressing step
 * @param dc the DC counter from the addressing step
 * @param in_file_path the path to the source code .am file
 * @param obj_file_path the path to write the .obj file to
 * @param ent_file_path the path to write the .ent file to
 * @param ext_file_path the path to write the .ext file to
 */
void assemble_code(HashTable *labels, int ic, int dc, const char* in_file_path, const char* obj_file_path, const char* ent_file_path, const char* ext_file_path);

#endif /* ASSEMBLE_H_ */
