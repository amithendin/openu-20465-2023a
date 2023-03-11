/*
 * hashtable.c
 *
 *  Created on: Jan 22, 2023
 *      Author: amit
 */

#include "hashtable.h"

HashTable *new_hashtable(unsigned int size) {
    HashTable *ht = malloc(sizeof(HashTable));/*allocate memory for hashtable*/
    ht->size = size;/*set size of entries array*/
    ht->entries = calloc(size, sizeof(Entry *));/*allocate memory for entries array*/
    return ht;
}

void free_hashtable(HashTable *ht) {
    /*iterators*/
    int i;
    Entry *curr, *tmp;

    if (ht == NULL)/*make sure we got an initialized hashtable*/
        return;

    /*iterate over all entries*/
    for (i = 0; i < ht->size; i++) {
        curr = ht->entries[i];/*for each index there may be colliding keys which are kept
        * as a linked list, so we iterate over them too*/
        while (curr != NULL) {
            tmp = curr;
            curr = curr->next;
            /*free key and struct since key string is copy of the original kept on the heap*/
            free(tmp->key);
            free(tmp);
        }
    }
    /*free entries array and hashtable memory*/
    free(ht->entries);
    free(ht);
}

unsigned long hash(char *key, unsigned int ht_size) {
    unsigned long c, hash;
    /*calculate hash value*/
    hash = HASH_MAGIC_NUM;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash % ht_size;/*covert hashvalue to index of the entries array*/
}

void *ht_put(HashTable *ht, char *key, void *value) {
    void *tmp;
    Entry *curr, *newEntry;
    unsigned long index, key_len;

    /*calculate hash value for given key*/
    index = hash(key, ht->size);
    key_len = strlen(key);

    /*check if the entries array for that hash value already contains
     * the give key */
    tmp = NULL;
    curr = ht->entries[index];
    while (curr != NULL) {/*iterate until the end of the linked list*/
        if (strcmp(curr->key, key) == 0) {
            /*if key is found, replace it's data pointer to the given pointer and
            * return the old one so it can be freed by the programmer using this*/
            tmp = curr->value;
            curr->value = value;
            return tmp;
        }
        curr = curr->next;/*keep iterating*/
    }
    /*if we haven't returned then the key doesn't exist in the
     * hashtable so we create a new key value pair for it and
     * insert it into the entries array*/
    newEntry = malloc(sizeof(Entry));
    newEntry->key = malloc(key_len);
    memcpy(newEntry->key, key, key_len + 1);
    newEntry->value = value;
    newEntry->next = ht->entries[index];
    ht->entries[index] = newEntry;

    return NULL;
}

void *ht_get(HashTable *ht, char *key) {
    unsigned long index;
    Entry *current;

    /*calculate hash value for given key*/
    index = hash(key, ht->size);
    current = ht->entries[index];

    /*check if the entries array for that hash value contains the give key */
    while (current != NULL) {/*iterate until the end of the linked list*/
        if (strcmp(current->key, key) == 0) {
            /*if key is found, returned the value paired to it*/
            return current->value;
        }
        current = current->next;/*keep iterating*/
    }

    /*if we haven't returned, key doesn't exists in the hashtable so we return null*/
    return NULL;
}

void ht_remove(HashTable *ht, char *key) {
    unsigned long index;
    Entry *curr, *prev;

    /*calculate hash value for given key*/
    index = hash(key, ht->size);
    curr = ht->entries[index];
    prev = NULL;

    while (curr != NULL) {/*iterate until the end of the linked list*/
        if (strcmp(curr->key, key) == 0) {
            /*if key is found, remove it from the linked list*/
            if (prev == NULL) {
                ht->entries[index] = curr->next;
            } else {
                prev->next = curr->next;
            }
            /*and free the memory it occupies*/
            free(curr->key);
            free(curr);
            /*stop iterating*/
            return;
        }
        /*keep track of previous list item so we can
        * remove a list item while keeping the list in tack*/
        prev = curr;
        curr = curr->next;/*keep iterating*/
    }
}
