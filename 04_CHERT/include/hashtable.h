#ifndef HASHTABLE_H__
#define HASHTABLE_H__

#include "common.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

typedef struct TableElem TableElem;

struct TableElem {

    unsigned int hash;      //key to get

    char*   name;           //file or dir name
    char*   outputName;     //output path
    int     wd;             //for inotify watching

    TableElem* next;        //
    TableElem* prev;        //here we use chain method

};

typedef struct HashTable {

    int size;
    TableElem** data;

} HashTable;

HashTable*  HashTableConstructor    (const int size);

void        HashTableAddElem        (HashTable* table, const char* name, const char* outputPath, const int wd);
TableElem*  HashTableGetElemByWD    (const HashTable* table, const int wd);

void        HashTableDestructor     (HashTable* table);

#endif
