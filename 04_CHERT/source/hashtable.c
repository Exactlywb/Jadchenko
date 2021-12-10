#include "hashtable.h"

static  unsigned int    Hash                (const HashTable* table, const int wd);
static  TableElem*      InitTableElem       (HashTable* table, const char* name, const char* outputPath, const int wd);
static  void            TableElemDestruct   (TableElem* elem);

HashTable* HashTableConstructor (const int size) {

    FUNCTION_SECURITY (size <= 0, {}, NULL);

    HashTable* res = (HashTable*)calloc (1, sizeof (*res));
    FUNCTION_SECURITY (IS_NULL (res), {}, NULL);

    res->size = size;

    res->data = (TableElem**)calloc (size, sizeof (TableElem*));
    FUNCTION_SECURITY (IS_NULL (res->data), {free (res);}, NULL);

    return res;

}

void HashTableAddElem (HashTable* table, const char* name, const char* outputPath, const int wd) {

    FUNCTION_SECURITY (IS_NULL (table), {}, );

    TableElem* elem = InitTableElem (table, name, outputPath, wd);
    syslog (LOG_INFO, "elem path: %s\n", elem->name);
    FUNCTION_SECURITY (IS_NULL (elem), {}, );

    TableElem* curOldElem = table->data [elem->hash];

    if (curOldElem == NULL) {

        table->data [elem->hash] = elem;
        return;

    }

    while (curOldElem->next != NULL)
        curOldElem = curOldElem->next;

    curOldElem->next = elem;
    elem->prev = curOldElem; 

}

TableElem* HashTableGetElemByWD (const HashTable* table, const int wd) {

    FUNCTION_SECURITY (IS_NULL (table), {}, NULL);
    
    unsigned int key = Hash (table, wd);
    return table->data [key]; 

}

void HashTableDestructor (HashTable* table) {

    for (int i = 0; i < table->size; ++i) {

        if (table->data [i] != NULL) {

            TableElem* curElem = table->data [i];
            while (curElem) {

                TableElem* toDelete = curElem;
                curElem = curElem->next;
                
                TableElemDestruct (toDelete);

            }

        }

    }

    free (table->data);
    free (table);

}

static void TableElemDestruct (TableElem* elem) {

    FUNCTION_SECURITY (IS_NULL (elem), {}, );

    free (elem->name);
    free (elem);

}

static TableElem* InitTableElem (HashTable* table, const char* name, const char* outputPath, const int wd) {

    TableElem* elem = (TableElem*)calloc (1, sizeof (*elem));
    FUNCTION_SECURITY (IS_NULL (elem), {}, NULL);

    elem->hash = Hash (table, wd);
    
    size_t strLength = strlen (name);
    elem->name = (char*)calloc (strLength + 1, sizeof (char));
    FUNCTION_SECURITY (IS_NULL (elem->name), {free (elem);}, NULL);
    strncpy (elem->name, name, strLength);

    size_t outputLength = strlen (outputPath);
    elem->outputName = (char*)calloc (outputLength + 1, sizeof (char));
    FUNCTION_SECURITY (IS_NULL (elem->outputName), {free (elem->outputName); free (elem);}, NULL);
    strncpy (elem->outputName, outputPath, outputLength);

    elem->next = NULL;
    elem->prev = NULL;

    elem->wd = wd;

    return elem;

}

static unsigned int Hash (const HashTable* table, const int wd) {

    unsigned int resHash = 0;

    for (int i = 0; i < 32; ++i) {

        resHash ^= wd;
        resHash *= 1099511628211;

    }

    return resHash % (table->size);

}
