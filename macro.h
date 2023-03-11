/*
 * macro.h
 *
 *  Created on: Jan 23, 2023
 *      Author: amit
 */

#ifndef MACRO_H
#define MACRO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "list.h"
#include "hashtable.h"

/**
 * Expands macros of an assembly file into a new assembly file
 *
 * @param in_file_path path to assembly file to expand
 * @param out_file_path path to create expanded assembly file in. overrites existing file at path
 */
void expand_macros(const char* in_file_path, const char* out_file_path);

#endif /* MACRO_H_ */
