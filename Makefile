.PHONY: assembler
assembler:
	gcc main.c address.c assemble.c hashtable.c list.c parse.c util.c validate.c macro.c -Wall -ansi -pedantic -o assembler