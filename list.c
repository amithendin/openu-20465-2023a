/*
 * list.c
 *
 *  Created on: Jan 22, 2023
 *      Author: amit
 */


#include "list.h"

int cmp_data(void *a, void *b) {
    unsigned char *uca = (unsigned char *) a;/*byte pointer to first pointer*/
    unsigned char *ucb = (unsigned char *) b;/*byte pointer to second pointer*/

    /* Compare the contents of the pointers bitwise*/
    return memcmp(uca, ucb, sizeof(void *)) == 0;
}

List *new_list() {
    List *l;
    l = malloc(sizeof(List));/*allocate memory for new list pointers*/
    l->length = 0;/*set count of items to 0*/
    l->head = NULL;/*indicate list is empty*/
    l->tail = NULL;
    return l;
}

void free_node(Node *n) {
    if (n == NULL) /*make sure we got a node*/
        return;

    free(n);/*free memory held by the node*/
}

void free_list(List *l) {
    Node *current;/*iterator*/
    Node *tmp;/*for swapping*/

    if (l == NULL)/*make sure we got a list*/
        return;
    current = l->head;/*start iterating from the last node inserted*/

    while (current != NULL) {/*iterate over all nodes*/
        tmp = current;
        current = current->next;/*move to next node*/
        free_node(tmp);/*free current node*/
    }

    free(l);/*free list memory*/
}

void l_push(List *l, void *data) {
    Node *new_node;
    new_node = (Node *) malloc(sizeof(Node));/*allocate memory for a new node*/
    new_node->data = data;/*set data pointer of new node to given data pointer*/
    new_node->next = l->head;/*set new node as the next node after the current head of the list*/
    new_node->prev = NULL;
    if (l->head != NULL) {/*if isn't first node to be inserted, set prev of the old newest node*/
        l->head->prev = new_node;
    } else {
        l->tail = new_node;/*else set it as the first element to be inserted*/
    }
    l->head = new_node;/*set as the latest node ot be inserted*/
    l->length++;/*increase the count*/
}

void l_remove(Node *n) {
    if (n->prev != NULL) { /*make sure to retain connectivity of pointer chain*/
        n->prev->next = n->next;
    }
    free_node(n);/*free memory occupied by node*/
}

Node *l_find(List *l, void *data) {
    Node *current;

    current = l->head;/*start iterating from the last inserted node*/
    while (current != NULL) {
        if (cmp_data(current->data, data)) {/*if data is equal byte-wise, return node*/
            return current;
        }
        current = current->next;/*keep iterating*/
    }

    return NULL;/*if we haven't returned, data isn't in list*/
}
