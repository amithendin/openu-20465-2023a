cmake_minimum_required(VERSION 3.24)
project(assembler C)

set(CMAKE_C_STANDARD 90)

add_executable(assembler main.c address.c assemble.c hashtable.c list.c parse.c util.c validate.c macro.c)
