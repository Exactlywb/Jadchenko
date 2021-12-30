#ifndef INOTIFY_R_H__
#define INOTIFY_R_H__

#include "common.h"

#define EVENT_SIZE          (sizeof (struct inotify_event))
#define EVENT_BUF_LENGTH    (1024 * (EVENT_SIZE + NAME_MAX + 1))
#define WATCH_FLAGS         (IN_CREATE | IN_MODIFY)

static int InotifyRun = 1;

typedef struct WatchElem {

    int     pd;

    char*   inputPath;
    char*   outputPath;

} WatchElem;

typedef struct HashTableElem HashTableElem;

struct HashTableElem {

    WatchElem*      elem;

    HashTableElem*  next;
    HashTableElem*  prev;

};

typedef struct HashTable {

    HashTableElem** data;    
    
    int             maxSize;
    int             curSize;

} HashTable;

typedef struct Watch {

    int         fd;

    HashTable*  watch;

    const char*       root;

} Watch;

//HASH TABLE INTERFACE
HashTable*      HashTableConstructor    (const int tableSize);
void            HashTableDestructor     (HashTable* table);

HashTableElem*  HashTableElemInit       (WatchElem* elem, HashTableElem* next, HashTableElem* prev);
void            HashTableListDestruct (HashTableElem* elem);
HashTableElem*  HashTableInsert         (HashTable* table, const int pd, 
                                        const char* inputPath, const char* outputPath);

void            HashTableErase          (HashTable* table, HashTableElem* elem);

HashTableElem*  FindInTable             (const HashTable* table, const WatchElem* elem);
HashTableElem*  FindInTableByWd         (const HashTable* table, const int wd);
HashTableElem*  FindInTableByName       (const HashTable* table, const char* name);

int             Hash                    (const HashTable* table, const WatchElem* elem);

void            HashTableDump           (const HashTable* table);

//WATCH INTERFACE
WatchElem*      WatchElemInit           (const int pd, const char* inputPath, const char* outputPath);
void            WatchElemDestructor     (WatchElem* elem);

Watch*          WatchConstructor        (const int tableSize);
void            WatchDestructor         (Watch* watch);

HashTableElem*  WatchInsert             (Watch* watch, const int pd, const char* inputPath, const char* outputPath);
void            WatchErase              (Watch* watch, WatchElem* elem);

void            WatchDump               (Watch* watch);

HashTableElem*  GetByWd                 (const Watch* watch, const int wd);
HashTableElem*  GetByName               (const Watch* watch, const char* name);

//INOTIFY
void            RunInotifyViewer        (Watch* watch, const char* root, const char* outputPath);
void            AddWatchesForDir        (Watch* watch, const char* root, const char* outputPath);

#endif
