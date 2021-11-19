#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>


#define FUNCTION_SECURITY(condition, toDo, err) do {                        \
                                                                            \
                                                    if (condition) {        \
                                                                            \
                                                        toDo                \
                                                        return err;         \
                                                                            \
                                                    }                       \
                                                                            \
                                                } while (0)


size_t  GetFileSize     (int fd);
int     RunCopy         (char* input, char* output);

//========================================================

int main(int argc, char* argv[])
{
    
    FUNCTION_SECURITY (argc != 3, {printf ("U have to enter input and output filenames\n");}, -1);

    char* input     = argv[1];
    char* output    = argv[2];

    return RunCopy(input, output);

}

int RunCopy(char* input, char* output) {

    assert(input);
    assert(output);

    int inputDesc = open(input, O_RDONLY);
    FUNCTION_SECURITY (inputDesc == -1, {perror ("Bad open () for input");}, errno);

    size_t  size    = GetFileSize(inputDesc);
    char*   srcBuff = mmap(NULL, size, PROT_READ, MAP_SHARED, inputDesc, 0);
    FUNCTION_SECURITY (srcBuff == MAP_FAILED, {perror ("Bad mmap ()");close (inputDesc);}, errno);

    int dstDesc = open(output, O_RDWR | O_CREAT, 0666);
    FUNCTION_SECURITY (dstDesc == -1, {perror ("Bad open () for output");}, errno);
    close(dstDesc);

    FUNCTION_SECURITY (truncate (output, size) == -1, { perror ("Bad truncate ()");
                                                        close (inputDesc);
                                                        close (dstDesc);
                                                        munmap (srcBuff, size);}, errno);

    dstDesc = open(output, O_RDWR | O_CREAT, 0666);
    FUNCTION_SECURITY (dstDesc == -1, {perror ("Bad open () for dstDesc");}, errno);

    char* dstBuf = mmap(NULL, size, PROT_WRITE, MAP_SHARED, dstDesc, 0);
    FUNCTION_SECURITY (dstBuf == MAP_FAILED, {  perror ("Bad mmap ()");
                                                close (inputDesc);
                                                close (dstDesc);
                                                munmap (srcBuff, size);}, errno);

    memcpy(dstBuf, srcBuff, size);

    FUNCTION_SECURITY (munmap (srcBuff, size) == -1, {  perror ("Bad munmap ()");
                                                        close (inputDesc);
                                                        close (dstDesc);}, errno);
    FUNCTION_SECURITY (munmap (dstBuf, size) == -1, {   perror ("Bad munmap ()"); 
                                                        close (inputDesc); 
                                                        close (dstDesc);}, errno);

    return 0;

}

size_t GetFileSize(int fd)
{

    struct stat info = {0};
    if (fstat(fd, &info) == -1) return 0;

    return (size_t)info.st_size;

}

