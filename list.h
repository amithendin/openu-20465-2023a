/*
 * list.h
 *
 *  Created on: Jan 22, 2023
 *      Author: amit
 */

#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Node sturct
 *
 * Used to store node of a linked list with a pointer to
 * data to store at that node
 */
typedef struct Node {
    void* data; /* data stored in the node */
    struct Node* next; /* pointer to the next node in the list */
    struct Node* prev; /* pointer to the previous node in the list */
} Node;

/**
 * List struct
 *
 * Used to store pointer to the elements of the list
 * and the amount of items in the list
 */
typedef struct {
	int length; /*count of items in the list*/
	Node* head; /*the last element to be pushed into the list*/
	Node* tail; /*the first element to be pushed into the list*/
} List;

/**
 * Creates new empty list
 *
 * @return pointer to new empty list in memory
 */
List* new_list();

/**
 * Frees all memory occupied by the list including the nodes but
 * not including the data pointer at each node
 *
 * @param l pointer to the list to free
 */
void free_list(List *l);

/**
 * Frees the memory occupied by the node not including data pointer
 *
 * @param n pointer to the node to free
 */
void free_node(Node* n);

/**
 * Push node to the list like in a stack
 *
 * @param l pointer to list to push into
 * @param data data pointer to store in the list
 */
void l_push(List* l, void* data);

/**
 * Remove node from the list and free it
 *
 * @param n pointer to node to remove
 */
void l_remove(Node* n);

/**
 * Find a node that contains a given data, compares node data by
 * doing byte-wise comparison on the contents of the data pointer
 * and the provided data pointer. preforms linear search on the list
 *
 * @param l list to search
 * @param data to find in list
 * @return the node that contains equal data to the provided data
 */
Node* l_find(List* l, void* data);

#endif
