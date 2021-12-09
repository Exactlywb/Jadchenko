#include "copy.h"

#define BUF_SIZE 1000

static  int     CopyDirRec          (DIR* inputDir, const char* output, const char* firstPath);

static  int     CopyRegularFile     (const char* fileName, const char* outputPath);
static  int     CopyLink            (const char* fileName, const char* outputPath);

/*COMMON FUNCTIONS*/
static  char*   ConcatTwoStrings    (const char* first, const char* second);

int CopyDir (const char* dirName, const char* output) {

    DIR* inputDir = opendir (dirName);
    FUNCTION_SECURITY (IS_NULL (inputDir), {perror ("bad opendir");}, -1);

    FUNCTION_SECURITY (mkdir (output, 0777) == -1, {syslog (LOG_ERR, "bad mkdir ()");}, -1);
    syslog (LOG_INFO, "%s directory was made", output);

    int ret = CopyDirRec (inputDir, output, dirName);

    closedir (inputDir);

    return ret;

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

            char* fileNameAddSlash      = ConcatTwoStrings (fileName, "/");                 //!TODO va_args?
            char* newInputPath          = ConcatTwoStrings (firstPath, fileNameAddSlash);   //

            char* newOutputPath         = ConcatTwoStrings (output, fileName);
            char* newOutputPathSlash    = ConcatTwoStrings (newOutputPath, "/");

            CopyDir (newInputPath, newOutputPathSlash);

            free (fileNameAddSlash);
            free (newInputPath);

            free (newOutputPath);
            free (newOutputPathSlash);

        } else {
            
            char* newInputPath  = ConcatTwoStrings (firstPath, fileName);
            char* newOutputPath = ConcatTwoStrings (output, fileName);
    
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

static char* ConcatTwoStrings (const char* first, const char* second) {

    size_t firstLength  = strlen (first);
    size_t secondLength = strlen (second);
    size_t fullLength = firstLength + secondLength;

    char* newString = (char*)calloc (fullLength + 1, sizeof (char));
    FUNCTION_SECURITY (IS_NULL (newString), {}, NULL);

    strncpy (newString, first, firstLength);
    strncpy (newString + firstLength, second, secondLength);

    return newString;

}

static int CopyRegularFile (const char* fileName, const char* outputPath) {

    int input = open (fileName, O_RDONLY, 0666);
    FUNCTION_SECURITY (input == -1, {perror ("bad open ()");}, -1);

    int output = open (outputPath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    FUNCTION_SECURITY (output == -1, {perror ("bad open ()");}, -1);

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
    FUNCTION_SECURITY (symlink (buffer, outputPath) == -1, {syslog (LOG_ERR, "bad symlink ()");}, -1);
    syslog (LOG_INFO, "link was copied as %s with target %s", outputPath, buffer);

    return 0;

}
