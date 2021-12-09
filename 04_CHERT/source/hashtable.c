#include "hashtable.h"

static  unsigned int    Hash                (HashTable* table, const char* name);
static  TableElem*      InitTableElem       (HashTable* table, const char* name);
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

void HashTableAddElem (HashTable* table, const char* name) {

    FUNCTION_SECURITY (IS_NULL (table), {}, );


    TableElem* elem = InitTableElem (table, name);
    FUNCTION_SECURITY (IS_NULL (elem), {}, );

    TableElem* curOldElem = table->data [elem->hash];
    while (curOldElem->next != NULL)
        curOldElem = curOldElem->next;

    curOldElem->next = elem;
    elem->prev = curOldElem; 

}

TableElem* HashTableGetElem (const HashTable* table, const char* name) {

    FUNCTION_SECURITY (IS_NULL (table), {}, NULL);
    return table->data [Hash (table, name)];

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

static TableElem* InitTableElem (HashTable* table, const char* name) {

    TableElem* elem = (TableElem*)calloc (1, sizeof (*elem));
    FUNCTION_SECURITY (IS_NULL (elem), {}, NULL);

    elem->hash = Hash (table, name);
    
    size_t strLength = strlen (name);
    elem->name = (char*)calloc (strLength + 1, sizeof (char));
    FUNCTION_SECURITY (IS_NULL (elem->name), {free (elem);}, NULL);
    strncpy (elem->name, name, strLength);

    elem->next = NULL;
    elem->prev = NULL;

    return elem;

}

static unsigned int Hash (HashTable* table, const char* name) {

    unsigned int resHash = 7;
    for (int i = 0; i < strlen (name); ++i)
        resHash = resHash * 31 + name [i];
    
    return resHash % (table->size);

}
