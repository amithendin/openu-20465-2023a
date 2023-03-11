/*
 * hashtable.h
 *
 *  Created on: Jan 22, 2023
 *      Author: amit
 *
 *  generic hash table structure meant for storing raw bytes of data.
 *  does not handle freeing of values just keys and nodes (only frees internal structure memory)
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#define HASH_MAGIC_NUM 5381 /*for calculating the hash function*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Entry struct
 *
 * Holds a key value pair entry in the hashtable
 * and functions as a node in the list of colliding keys
 */
typedef struct Entry {
    char* key; /*the key paired to the value*/
    void* value; /*the pointer to the memory we want to store*/
    struct Entry* next; /*pointer to the next entry with a different key that generates the same hash value*/
} Entry;

/**
 * Hashtable struct
 *
 * Holds the number of values in the table and the array of entries.
 * each item in the array is an entry struct that may point to other
 * entry structs
 */
typedef struct {
	unsigned int size;/*total amount of entries counted by unique keys*/
    Entry** entries;/*array of entry lists*/
} HashTable;

/**
 * Creates a new hashtable
 * @param size of the entries array
 * @return new hashtable with an entries array of given size
 */
HashTable* new_hashtable(unsigned int size);

/**
 * Frees a given hashtable without freeing the values stored in the table
 *
 * @param ht the hashtable to free
 */
void free_hashtable(HashTable* ht);

/**
 * Calculate an index in the entries array given it's size and the hash value
 * calculated for the given key
 *
 * @param key the key in a form of a string
 * @param size the size of the hashtable entries array
 * @return hash value index
 */
unsigned long hash(char* key, unsigned int size);

/**
 * Inserts a key value pair to the hashtable
 *
 * @param ht the hashtable ot insert into
 * @param key the key
 * @param value the value
 * @return the previous value for the given key in-case the key was already paired with a value, this
 * is so the programmer using this can free the overriden value so as not to cause memory leaks since this
 * hashtable doesn't free values. returns null if this is the first insertion of the key into the table
 */
void* ht_put(HashTable* ht, char* key, void* value);

/**
 * Returns the value paired to the key in the table, returns
 * null if the key isn't in the hashtable
 *
 * @param ht the hashtable
 * @param key the key
 * @return the value of the key in the hashtable, returns null
 * if key isn't in the hashtable
 */
void* ht_get(HashTable* ht, char* key);

/**
 * Removes key value pair from the hashtalbe
 *
 * @param ht the hashtalbe
 * @param key the key to remove, also removes it's paired value
 */
void ht_remove(HashTable* ht, char* key);


#endif /* HASHTABLE_H_ */
