#include "inotifyR.h"

//==========================================================
//======================= INOTIFY ==========================
//==========================================================
void RunInotifyViewer (Watch* watch, const char* root, const char* outputPath) {

    watch->fd = inotify_init1 (0);
    FUNCTION_SECURITY (watch->fd < 0, {syslog (LOG_ERR, "Bad inotify_init1 ()");}, );

    watch->root = root;

    AddWatchesForDir (watch, root, outputPath);

}

void AddWatchesForDir (Watch* watch, const char* root, const char* outputPath) {

    assert (watch);
    assert (root);
    assert (outputPath);

    int wd = inotify_add_watch (watch->fd, root, IN_CREATE | IN_MODIFY);
    FUNCTION_SECURITY (wd == -1, {syslog (LOG_ERR, "bad inotify_add_watch (): %s", strerror (errno));}, );
    syslog (LOG_INFO, "watching for %s\n", root);

    WatchInsert (watch, wd, root, outputPath);

    DIR* dir = opendir (root);
    struct dirent* curFile = NULL;
    while (curFile = readdir (dir)) {

        const char* fileName = curFile->d_name;
        if (STR_EQ (fileName, ".", 1))
            continue;
        if (STR_EQ (fileName, "..", 2))
            continue;

        unsigned char type = curFile->d_type;
        if (type == DT_DIR) {

            char* fullPath = ConcatStrings (3, root, curFile->d_name, "/");
            char* fullOutputPath = ConcatStrings (3, outputPath, curFile->d_name, "/");
            
            AddWatchesForDir (watch, fullPath, fullOutputPath);
            
            free (fullPath);
            free (fullOutputPath);

        }

    }

}

//==========================================================
//====================== HASH TABLE ========================
//==========================================================
HashTable* HashTableConstructor (const int tableSize) {
    
    HashTable* table = (HashTable*)calloc (1, sizeof (*table));
    assert (table);

    table->data = (HashTableElem**)calloc (tableSize, sizeof (*table->data));
    assert (table->data);

    table->curSize = 0;
    table->maxSize = tableSize;

    return table;

}

void HashTableDestructor (HashTable* table) {

    assert (table);

    for (int i = 0; i < table->maxSize; ++i)
        HashTableListDestruct (table->data [i]);

    free (table->data);
    free (table);

}

void HashTableListDestruct (HashTableElem* elem) {

    if (!elem)
        return;
    
    HashTableElem* curElem = elem;
    while (curElem->next != NULL)
        curElem = curElem->next;
    
    while (curElem->prev) {

        WatchElemDestructor (curElem->elem);
        HashTableElem* toFree = curElem;
        curElem = curElem->prev;

        free (toFree);

    }

    WatchElemDestructor (elem->elem);
    free (elem);

}

HashTableElem* HashTableElemInit (WatchElem* elem, HashTableElem* next, HashTableElem* prev) {

    assert (elem);

    HashTableElem* res = (HashTableElem*)calloc (1, sizeof (*res));
    assert (res);

    res->elem = elem;
    res->next = next;
    res->prev = prev;

    return res;
    
}

HashTableElem*  HashTableInsert         (HashTable* table, const int pd, 
                                        const char* inputPath, const char* outputPath) {

    assert (table);

    WatchElem* toInsert = WatchElemInit (pd, inputPath, outputPath);
    assert (toInsert);
    
    int hash = Hash (table, toInsert);
    HashTableElem* curElem = table->data [hash];

    while (curElem && curElem->next != NULL)
        curElem = curElem->next;
    
    HashTableElem* newElem = HashTableElemInit (toInsert, NULL, curElem);
    if (!curElem)  //first elem in list
        table->data [hash] = newElem;
    else
        curElem->next = newElem;

    table->curSize++;

    return newElem;

}

void HashTableErase (HashTable* table, HashTableElem* elem) {

    if (elem) {

        HashTableElem* nextElem = elem->next;
        HashTableElem* prevElem = elem->prev;

        if (nextElem)
            nextElem->prev = prevElem;
        if (prevElem)
            prevElem->next = nextElem;
        else {
            
            int index = Hash (table, elem->elem);
            table->data [index] = nextElem;

        }

        WatchElemDestructor (elem->elem);
        free (elem);

        table->curSize--;

    }

}

HashTableElem* FindInTable (const HashTable* table, const WatchElem* elem) {

    int hash = Hash (table, elem);
    HashTableElem* curElem = table->data [hash];
    if (curElem) {

        while (curElem) {

            WatchElem* watchElem = curElem->elem;

            char* input     = watchElem->inputPath;
            char* output    = watchElem->outputPath;
            if (strcmp (input, elem->inputPath)     == 0 &&
                strcmp (output, elem->outputPath)   == 0 &&
                watchElem->pd                       == elem->pd)
                return curElem;

            curElem = curElem->next;

        }

    }

    return curElem;
    
}

HashTableElem* FindInTableByName (const HashTable* table, const char* name) {
    //slow function. 
    for (int i = 0; i < table->maxSize; ++i) {

        HashTableElem* curElem = table->data [i];
        if (curElem) {

            while (curElem) {

                if (STR_EQ (curElem->elem->inputPath, name, strlen (name) - 1)) //without slash
                    return curElem;

                curElem = curElem->next;

            }

        }

    }

    return NULL;

}

HashTableElem* FindInTableByWd (const HashTable* table, const int wd) {

    assert (table);

    WatchElem elem = {wd, "", ""};
    int index = Hash (table, &elem);

    HashTableElem* res = table->data [index];
    while (res && res->elem->pd != wd)
        res = res->next;

    return res;

}

int Hash (const HashTable* table, const WatchElem* elem) {
    //magic const hash implementation
    assert (elem);

    int hash = 0;

    for (int i = 0; i < 32; ++i) {

        hash ^= elem->pd;
        hash *= 1099511628211;

    }

    return (hash % table->maxSize) > 0 ? (hash % table->maxSize) : -(hash % table->maxSize);

}

void HashTableDump (const HashTable* table) {

    assert (table);

    for (int i = 0; i < table->maxSize; ++i) {

        HashTableElem* curElem = table->data [i];
        if (!curElem)
            continue;
        
        while (curElem->next) {

            printf ("%d, %s, %s -> ", curElem->elem->pd, 
                                      curElem->elem->inputPath, 
                                      curElem->elem->outputPath);
            curElem = curElem->next;

        } 
        
        printf ("%d, %s, %s\n", curElem->elem->pd, 
                                curElem->elem->inputPath, 
                                curElem->elem->outputPath);

    }

}

//==========================================================
//===================== WATCH STRUCT =======================
//==========================================================
WatchElem* WatchElemInit (const int pd, const char* inputPath, const char* outputPath) {

    WatchElem* elem = (WatchElem*)calloc (1, sizeof (*elem));
    assert (elem);

    elem->pd = pd;

    size_t  inputLength     = strlen (inputPath);
    char*   copyInput       = (char*)calloc (inputLength + 1, sizeof (char));
    assert (copyInput);
    strncpy (copyInput, inputPath, inputLength);
    elem->inputPath         = copyInput;

    size_t  outputLength    = strlen (outputPath);
    char*   copyOutput      = (char*)calloc (outputLength + 1, sizeof (char));
    assert (copyOutput);
    strncpy (copyOutput, outputPath, outputLength);
    elem->outputPath     = copyOutput;

    return elem;

}

void WatchElemDestructor (WatchElem* elem) {

    assert (elem);

    free (elem->inputPath);
    free (elem->outputPath);

    free (elem);

}

Watch* WatchConstructor (const int tableSize) {

    assert (tableSize > 0);

    Watch* res = (Watch*)calloc (1, sizeof (*res));
    assert (res);

    res->watch = HashTableConstructor (tableSize);
    assert (res->watch);

    return res;

}

void WatchDestructor (Watch* watch) {

    assert (watch);

    HashTableDestructor (watch->watch);
    close (watch->fd);
    free (watch);

}

HashTableElem* WatchInsert (Watch* watch, const int pd, const char* inputPath, const char* outputPath) {

    assert (inputPath);
    assert (outputPath);

    return HashTableInsert (watch->watch, pd, inputPath, outputPath);

}
void WatchErase (Watch* watch, WatchElem* elem) {

    assert (watch);
    assert (elem);

    HashTableErase (watch->watch, FindInTable (watch->watch, elem));

}

void WatchDump (Watch* watch) {

    assert (watch);

    printf ("Watch ptr: %p\n", (void*)watch);
    printf ("Watch hashtable ptr: %p\n", (void*)watch->watch);

    HashTableDump (watch->watch);

}

HashTableElem* GetByWd (const Watch* watch, const int wd) {

    assert (watch);

    return FindInTableByWd (watch->watch, wd);

}

HashTableElem* GetByName (const Watch* watch, const char* name) {

    assert (watch);
    assert (name);

    return FindInTableByName (watch->watch, name);

}
