#ifndef HASHTABLE_H__
#define HASHTABLE_H__

#include "common.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

typedef struct TableElem {

    unsigned int hash;      //key to get

    char* name;             //file or dir name

    TableElem* next;        //
    TableElem* prev;        //here we use chain method

} TableElem;

typedef struct HashTable {

    int size;
    TableElem** data;

} HashTable;

HashTable*  HashTableConstructor    (const int size);

void        HashTableAddElem        (HashTable* table, const char* name);
TableElem*  HashTableGetElem        (const HashTable* table, const char* hash);

void        HashTableDestructor     (HashTable* table);

#endif
