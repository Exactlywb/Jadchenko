#include "copy.h"

#define BUF_SIZE 1000

static  int     CopyDirRec          (DIR* inputDir, const char* output, const char* firstPath);

static  int     CopyRegularFile     (const char* fileName, const char* outputPath);
static  int     CopyLink            (const char* fileName, const char* outputPath);

/*INOTIFY*/
// static  int     RunInotify          (const char* dirName);
// static  void    AddWatchesForDir    (const int fd, const char* dirName);

/*COMMON FUNCTIONS*/
static  char*   ConcatStrings       (const int num, ...);

int CopyDir (const char* dirName, const char* output) {
    
    DIR* inputDir = opendir (dirName);
    FUNCTION_SECURITY (IS_NULL (inputDir), {syslog (LOG_ERR, "bad opendir (): %s", strerror (errno));}, -1);

    int mkdirCheck = mkdir (output, 0777);
    if (mkdirCheck == -1 && errno != EEXIST) {

        syslog (LOG_ERR, "bad mkdir (): %s", strerror (errno));
        return -1;

    }

    syslog (LOG_INFO, "%s directory was made", output);

    int ret = CopyDirRec (inputDir, output, dirName);

    closedir (inputDir);

    return ret;

}

typedef struct MyInotifyInfo {

    int isWorking;
    int fd;

    HashTable* paths;
    
} MyInotifyInfo;

static void AddWatchesForDir (MyInotifyInfo* myInotify, const char* dirName, const char* output) {

    FUNCTION_SECURITY (IS_NULL (myInotify)  , {}, );
    FUNCTION_SECURITY (IS_NULL (dirName)    , {}, );

    int wd = inotify_add_watch (myInotify->fd, dirName, IN_CREATE | IN_MODIFY);
    FUNCTION_SECURITY (wd == -1, {syslog (LOG_ERR, "bad inotify_add_watch (): %s", strerror (errno));}, );
    syslog (LOG_INFO, "watching for %s\n", dirName);

    HashTableAddElem (myInotify->paths, dirName, output, wd);

    DIR* dir = opendir (dirName);
    struct dirent* curFile = NULL;
    while (curFile = readdir (dir)) {

        const char* fileName = curFile->d_name;
        if (STR_EQ (fileName, ".", 1))
            continue;
        if (STR_EQ (fileName, "..", 2))
            continue;

        unsigned char type = curFile->d_type;
        if (type == DT_DIR) {

            char* fullPath = ConcatStrings (3, dirName, curFile->d_name, "/");
            char* fullOutputPath = ConcatStrings (3, output, curFile->d_name, "/");
            
            AddWatchesForDir (myInotify, fullPath, fullOutputPath);
            
            free (fullPath);
            free (fullOutputPath);

        }

    }

}

static void RunInotify (MyInotifyInfo* myInotify, const char* dirName, const char* output) {

    myInotify->fd = inotify_init1 (0);
    FUNCTION_SECURITY (myInotify->fd < 0, {syslog (LOG_ERR, "Bad inotify_init1 (): %s", strerror (errno));}, );

    myInotify->paths = HashTableConstructor (10000);

    AddWatchesForDir (myInotify, dirName, output);

    syslog (LOG_INFO, "watching for %s", dirName);

    myInotify->isWorking = 1;

}

static void CheckModificationAndCopyThis (MyInotifyInfo* myInotify, const char* dirName, const char* output) {

    FUNCTION_SECURITY (IS_NULL (myInotify)  , {}, );
    FUNCTION_SECURITY (IS_NULL (dirName)    , {}, );
    FUNCTION_SECURITY (IS_NULL (output)     , {}, );

    #define MAX_EVENTS  1024                                //max event num to handle
    #define LEN_NAME    100                                 //max file name length
    #define EVENT_SIZE  (sizeof (struct inotify_event))
    #define BUF_LEN     (MAX_EVENTS * (EVENT_SIZE + LEN_NAME))

    char buffer [BUF_LEN] = {0};

    fcntl (myInotify->fd, F_SETFL, O_NONBLOCK);

    ssize_t length = read (myInotify->fd, buffer, BUF_LEN);
    FUNCTION_SECURITY (length < 0, {}, );

    struct inotify_event* event = NULL;
    for (ssize_t i = 0; i < length; i += EVENT_SIZE + event->len) {

        event = (struct inotify_event*)&buffer [i];

        if (event->len) {
            
            if (event->mask & IN_CREATE) {

                if (event->mask & IN_ISDIR) {

                    syslog (LOG_INFO, "the directory %s was created", event->name);
                    
                    TableElem* parentPath = HashTableGetElemByWD (myInotify->paths, event->wd);
                    
                    char* fullPath = ConcatStrings (3, parentPath->name, event->name, "/");
                    char* fullOutputPath = ConcatStrings (3, parentPath->outputName, event->name, "/");
                    syslog (LOG_INFO, "Full paths: %s and %s", fullPath, fullOutputPath);
                    
                    AddWatchesForDir (myInotify, fullPath, fullOutputPath);
                    CopyDir (fullPath, fullOutputPath);

                    free (fullPath);
                    free (fullOutputPath);

                } else {

                    CopyDir (dirName, output);
                    syslog (LOG_INFO, "the file %s was created", event->name);

                }

            }

            if (event->mask & IN_MODIFY) {

                if (event->mask & IN_ISDIR) {

                    syslog (LOG_INFO, "the directory %s was modified", event->name);
                    
                } else {
                    
                    CopyDir (dirName, output);
                    syslog (LOG_INFO, "the file %s was modified", event->name);

                }

            }

        }

    }

}

int CopyUsingInotify (const char* dirName, const char* output) {

    static MyInotifyInfo myInotify = {-1, -1, NULL};
    if (myInotify.isWorking == -1)
        RunInotify (&myInotify, dirName, output);
        
    FUNCTION_SECURITY (myInotify.isWorking == -1, {syslog (LOG_ERR, "can't run inotify :(");}, -1);

    CheckModificationAndCopyThis (&myInotify, dirName, output);

}

static int CopyDirRec (DIR* inputDir, const char* output, const char* firstPath) {

    struct dirent* curFile = NULL;
    while (curFile = readdir (inputDir)) {
        const char* fileName = curFile->d_name;
        if (STR_EQ (fileName, "." , 1))
            continue;
        if (STR_EQ (fileName, "..", 2))
            continue;

        unsigned char type = curFile->d_type;
        if (type == DT_DIR) {

            char* newInputPath      = ConcatStrings (3, firstPath, fileName, "/");
            char* newOutputPath     = ConcatStrings (3, output, fileName, "/");

            syslog (LOG_INFO, "Here we copy dir %s into %s", newInputPath, newOutputPath);

            CopyDir (newInputPath, newOutputPath);

            free (newInputPath);
            free (newOutputPath);

        } else {

            char* newInputPath  = ConcatStrings (2, firstPath, fileName);
            char* newOutputPath = ConcatStrings (2, output, fileName);

            syslog (LOG_INFO, "Here we copy file %s into %s", newInputPath, newOutputPath);
            if (type == DT_LNK)
                CopyLink (newInputPath, newOutputPath);
            else
                CopyRegularFile (newInputPath, newOutputPath);
    
            free (newInputPath);
            free (newOutputPath);

        }

    }

    return 0;

}

static char* ConcatStrings (const int num, ...) {

    va_list strs;
    va_start (strs, num);
    const char* curStr = NULL;
    size_t fullLength = 0;
    for (int i = 0; i < num; ++i) {

        curStr = va_arg (strs, char*);
        fullLength += strlen (curStr);

    }

    va_end (strs);
    
    va_list newStrs;
    char* res = (char*)calloc (fullLength + 1, sizeof (*res));
    va_start (newStrs, num);

    for (int i = 0; i < num; ++i) {

        curStr = va_arg (newStrs, char*);

        size_t curLength = strlen (curStr);
        strncpy (res, curStr, curLength);

        res += curLength;

    }
    *res = 0;

    va_end (newStrs);
    
    res -= fullLength;

    return res;

}

static int CopyRegularFile (const char* fileName, const char* outputPath) {

    int input = open (fileName, O_RDONLY, 0666);
    FUNCTION_SECURITY (input == -1, {syslog (LOG_ERR, "bad open ()");}, -1);

    int output = open (outputPath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    FUNCTION_SECURITY (output == -1, {syslog (LOG_ERR, "bad open ()");}, -1);

    char buffer [BUF_SIZE] = {0};
    ssize_t readenBytes = read (input, buffer, BUF_SIZE);
    while (readenBytes > 0) {

        FUNCTION_SECURITY (write (output, buffer, readenBytes) == -1, {perror ("bad write ()");}, -1);
        readenBytes = read (input, buffer, BUF_SIZE);

    }

    syslog (LOG_INFO, "file %s was copied as %s", fileName, outputPath);

    close (output);
    close (input);

    return 0;

}

static int CopyLink (const char* fileName, const char* outputPath) {

    char buffer [BUF_SIZE] = {0};
    ssize_t sourceSize = readlink (fileName, buffer, BUF_SIZE);
    FUNCTION_SECURITY (symlink (buffer, outputPath) == -1, {syslog (LOG_ERR, "bad symlink (): %s", strerror (errno));}, -1);
    syslog (LOG_INFO, "link was copied as %s with target %s", outputPath, buffer);

    return 0;

}
