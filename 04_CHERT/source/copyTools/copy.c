#include "copy.h"

typedef struct MountedDictionary {

    char**  mountedDirs;
    char**  rootDirs;

    int     mountedDirsNum;

} MountedDictionary;

static  inline  char*               BuildRealDirName                (const char* name);

static          int                 CopyDirRec                      (DIR* inputDir, const char* inputPath, const char* output);

static          void                CopyLink                        (const char* input, const char* output);
static          void                CopyRegularFile                 (const char* input, const char* output);

static          MountedDictionary*  FindMountedDirs                 (const char* originalRoot, const char* root);
static          void                ParseMountLineAndCheckOurRoot   (const char* originalRoot, const char* path, const char* line, char** res);

int MountCopy (Watch* watch, const char* dirName, const char* output, const char* shortDirName) {

    MountedDictionary* mountedDirs = FindMountedDirs (watch->root, dirName);

    if (mountedDirs == NULL)
        return RegularCopy (dirName, output);
    else {

        for (int i = 0; i < mountedDirs->mountedDirsNum; ++i) {

            const char* mountedName = mountedDirs->mountedDirs [i];
            syslog (LOG_INFO, "Trying to copy mounted dir %s", mountedName);

            HashTableElem* findMountInTable = GetByName (watch, mountedName);
            if (findMountInTable) {
                
                RegularCopy (mountedDirs->rootDirs [i], findMountInTable->elem->outputPath);
                syslog (LOG_INFO, "%s dir is mounted to %s so it was copied too as %s", mountedName, dirName, findMountInTable->elem->outputPath);

                free (mountedDirs->mountedDirs [i]);
                free (mountedDirs->rootDirs [i]);

            } else
                syslog (LOG_INFO, "%s dir is mounted to %s but not in table", mountedName, dirName);

        }

        free (mountedDirs->mountedDirs);
        free (mountedDirs->rootDirs);
        free (mountedDirs);

        return RegularCopy (dirName, output);

    }

}

int RegularCopy (const char* dirName, const char* output) {

    syslog (LOG_INFO, "Try to copy %s as %s", dirName, output);

    int err = 0;

    char* realDirName = BuildRealDirName (dirName);
    char* realOutName = BuildRealDirName (output);

    DIR* inputDir = opendir (dirName);
    FUNCTION_SECURITY (IS_NULL (inputDir), {syslog (LOG_ERR, "bad opendir (): %s", STR_ERR);}, -1);

    int mkdirCheck = mkdir (output, 0777);
    if (mkdirCheck == -1 && errno != EEXIST) {

        syslog (LOG_ERR, "bad mkdir (): %s", STR_ERR);
        err = -1;
        goto RegularCopyExit;

    } else 
        syslog (LOG_INFO, "%s directory was made", output);

    err = CopyDirRec (inputDir, realDirName, realOutName);

RegularCopyExit:

    closedir (inputDir);

    free (realDirName);
    free (realOutName);

    return err;

}

int InotifyCopy (Watch* watch, const char* dirName, const char* output) {

    FUNCTION_SECURITY (IS_NULL (watch)  , {syslog (LOG_ERR, "NULL ptr on Watch* watch in function %s", __func__);}, -1);
    FUNCTION_SECURITY (IS_NULL (dirName), {}, -1);
    FUNCTION_SECURITY (IS_NULL (output) , {}, -1);

    char buffer [EVENT_BUF_LENGTH] = {0};

    fcntl (watch->fd, F_SETFL, O_NONBLOCK);

    ssize_t length = read (watch->fd, buffer, EVENT_BUF_LENGTH);
    FUNCTION_SECURITY (length < 0, {}, 0);

    struct inotify_event* event = NULL;
    for (ssize_t i = 0; i < length; i += EVENT_SIZE + event->len) {

        event = (struct inotify_event*)&buffer [i];

        if (event->len) {
            
            if (event->mask & IN_CREATE) {

                if (event->mask & IN_ISDIR) {

                    syslog (LOG_INFO, "the directory %s was created", event->name);
                    
                    HashTableElem* parentPath = GetByWd (watch, event->wd);
                    
                    char* fullPath = ConcatStrings (3, parentPath->elem->inputPath, event->name, "/");
                    char* fullOutputPath = ConcatStrings (3, parentPath->elem->outputPath, event->name, "/");

                    AddWatchesForDir (watch, fullPath, fullOutputPath);
                    MountCopy (watch, fullPath, fullOutputPath, event->name);

                    free (fullPath);
                    free (fullOutputPath);

                } else {
                    
                    HashTableElem* parentPath = GetByWd (watch, event->wd);
                    MountCopy (watch, parentPath->elem->inputPath, parentPath->elem->outputPath, event->name);
                    syslog (LOG_INFO, "the file %s was created", event->name);

                }

            }

            if (event->mask & IN_MODIFY) {

                if (event->mask & IN_ISDIR) {

                    syslog (LOG_INFO, "the directory %s was modified", event->name);
                    
                } else {
                    
                    HashTableElem* parentPath = GetByWd (watch, event->wd);
                    MountCopy (watch, parentPath->elem->inputPath, parentPath->elem->outputPath, event->name);
                    syslog (LOG_INFO, "the file %s was modified", event->name);

                }

            }

        }

    }

}

static int CopyDirRec (DIR* inputDir, const char* inputPath, const char* output) {

    struct dirent* curFile = NULL;
    while (curFile = readdir (inputDir)) {
        
        const char* unitName = curFile->d_name;
        if (STR_EQ (unitName, ".", 1))
            continue;
        if (STR_EQ (unitName, "..", 2))
            continue;

        unsigned char type = curFile->d_type;
        switch (type) {

            case DT_DIR: {
                
                char* newInPath     = ConcatStrings (3, inputPath, unitName, "/");
                char* newOutPath    = ConcatStrings (3, output, unitName, "/");

                RegularCopy (newInPath, newOutPath);
                
                free (newInPath);
                free (newOutPath);

                break;

            }
            case DT_LNK:
            default: {
                
                char* newInPath     = ConcatStrings (2, inputPath, unitName);
                char* newOutPath    = ConcatStrings (2, output, unitName);

                if (type == DT_LNK)
                    CopyLink (newInPath, newOutPath);
                else
                    CopyRegularFile (newInPath, newOutPath);

                free (newInPath);
                free (newOutPath);

            }

        }

    }

}

static void CopyLink (const char* input, const char* output) {

    syslog (LOG_INFO, "Try copy %s link as %s", input, output);

    char buffer [BUF_SIZE] = {0};
    ssize_t sourceSize = readlink (input, buffer, BUF_SIZE);
    FUNCTION_SECURITY (symlink (buffer, output) == -1, {syslog (LOG_ERR, "bad symlink (): %s", STR_ERR);}, );
    
    syslog (LOG_INFO, "link was copied as %s with target %s", output, buffer);

}

static void CopyRegularFile (const char* input, const char* output) {
    
    syslog (LOG_INFO, "Try copy %s file as %s...", input, output);
    
    int inputDesc = open (input, O_RDONLY, 0666);
    FUNCTION_SECURITY (inputDesc == -1, {syslog (LOG_ERR, "bad open ()");}, );
    
    int outputDesc = open (output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    FUNCTION_SECURITY (outputDesc == -1, {perror ("fuck all");syslog (LOG_ERR, "bad open ()");}, );

    char buffer [BUF_SIZE] = {0};
    ssize_t readenBytes = read (inputDesc, buffer, BUF_SIZE);
    
    while (readenBytes > 0) {
        
        FUNCTION_SECURITY (write (outputDesc, buffer, readenBytes) == -1, {perror ("bad write ()");}, );
        readenBytes = read (inputDesc, buffer, BUF_SIZE);

    }

    syslog (LOG_INFO, "file %s was copied as %s", input, output);

    close (outputDesc);
    close (inputDesc);

}

static inline char* BuildRealDirName (const char* name) {

    size_t nameLength = strlen (name);
    int curInd = (int)nameLength - 1;
    while (isspace (name [curInd]))
        curInd--;

    if (name [curInd] != '/') {

        char* res = ConcatStrings (2, name, "/");
        return res;

    }

    char* res = ConcatStrings (1, name);
    return res;

}

static MountedDictionary* FindMountedDirs (const char* originalRoot, const char* root) {

    FUNCTION_SECURITY (IS_NULL (root), {}, NULL);

    static const char* mountInfoPath = "/proc/self/mountinfo";

    char buffer [BUF_SIZE] = {0};

    MountedDictionary* res = NULL;

    int mountInfoFD = open (mountInfoPath, O_RDONLY, 0666);
    ssize_t readenBytes = read (mountInfoFD, buffer, BUF_SIZE);
    while (readenBytes > 0) {
        
        //Parse
        char* line = strtok (buffer, "\n");

        while (line) {

            char* mountedName [2] = {NULL};
            ParseMountLineAndCheckOurRoot (originalRoot, root, line, mountedName);
            if (mountedName [0]) {

                if (IS_NULL (res)) {
                
                    res = (MountedDictionary*)calloc (1, sizeof (*res));
                    FUNCTION_SECURITY (IS_NULL (res), {syslog (LOG_ERR, "1) Bad alloc in function %s", __func__);}, NULL);

                    res->mountedDirs = (char**)calloc (1000, sizeof (char*));   //!TODO realloc
                    FUNCTION_SECURITY (IS_NULL (res->mountedDirs), {syslog (LOG_ERR, "2) Bad alloc in function %s", __func__); 
                                                                    free (res);}, NULL);

                    res->rootDirs = (char**)calloc (1000, sizeof (char*));   //!TODO reallloc
                    FUNCTION_SECURITY (IS_NULL (res->rootDirs), {syslog (LOG_ERR, "3) Bad alloc in function %s", __func__); 
                                                                 free (res);}, NULL);

                }

                syslog (LOG_INFO, "rootDirs %s and mountedDirs %s", mountedName [0], mountedName [1]);
                res->mountedDirs    [res->mountedDirsNum]   = mountedName [0];
                res->rootDirs       [res->mountedDirsNum++] = mountedName [1];

            }

            line = strtok (NULL, "\n");

        }

        //Read again
        readenBytes = read (mountInfoFD, buffer, BUF_SIZE);

    }

    close (mountInfoFD);
    return res;

}

static void ParseMountLineAndCheckOurRoot (const char* originalRoot, const char* path, const char* line, char** res) {

    FUNCTION_SECURITY (IS_NULL (originalRoot), {}, );
    FUNCTION_SECURITY (IS_NULL (path), {}, );
    FUNCTION_SECURITY (IS_NULL (line), {}, );

    char rootNameToCmp  [BUF_SIZE] = {0};
    char mountedNameTmp [BUF_SIZE] = {0};
    sscanf (line, "%*u %*u %*u:%*u %s %s", rootNameToCmp, mountedNameTmp);

    if (strstr (rootNameToCmp, originalRoot)) {  //without last slash
        
        size_t mountedNameLength = strlen (mountedNameTmp);
        char* mountedName = (char*)calloc (mountedNameLength + 1, sizeof (*mountedName));
        FUNCTION_SECURITY (IS_NULL (mountedName), {syslog (LOG_ERR, "Bad alloc in function %s", __func__);}, );
        strncpy (mountedName, mountedNameTmp, mountedNameLength);
        res [0] = mountedName;

        size_t rootNameLength = strlen (rootNameToCmp);
        char* rootName = (char*)calloc (rootNameLength + 1, sizeof (*rootName));
        FUNCTION_SECURITY (IS_NULL (rootName), {syslog (LOG_ERR, "Bad alloc in function %s", __func__);}, );
        strncpy (rootName, rootNameToCmp, rootNameLength);
        res [1] = rootName;

        syslog (LOG_INFO, "HERE root %s and mounted %s", rootName, mountedName);

    }

}
