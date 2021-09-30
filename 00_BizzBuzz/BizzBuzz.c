#include "BizzBuzz.h"

static      int     HandleBizzBuzz      (const int input, const int output);
static      int     CatchBizzBuzzSpace  (const int input, long* offset, char* buffer, int* readenBytes, char* lastWordFlag,
                                        char* mod3, char* mod5, int* bufferInd, const int output);

static      int     PrintWord           (const int input, long* offset, char* buffer, int* readenBytes, int* bufferInd, 
                                        const int output); 

int RunBizzBuzz (const int argc, char** argv) {

    int inDescr = open (argv [1], O_RDONLY);
    FUNCTION_ASSERT (inDescr < 0, {perror ("Can't open input file (1st argument)");}, errno);

    int outDescr = open (argv [2], O_WRONLY | O_CREAT | O_TRUNC, 0666);         //to think about 0666
    FUNCTION_ASSERT (outDescr < 0, {perror ("Can't create and write into output file (2nd argument)");}, errno);

    int errRes = HandleBizzBuzz (inDescr, outDescr);
    
    close (inDescr);
    close (outDescr);

    return errRes;

}

#define BUFFER_SIZE 1000
#define CUR_SYMB buffer [bufferInd]

static int HandleBizzBuzz (const int input, const int output) {

    char buffer [BUFFER_SIZE] = {};
    
    long offset = 0; //in the near future we will do lseek (input, -inputOffset, SEEK_CUR);
    char lastWordFlag = BB_SPACE_;

    char mod3 = 0;
    char mod5 = 0;

    int readenBytes = read (input, buffer, BUFFER_SIZE);
    FUNCTION_ASSERT (readenBytes < 0, {perror ("Bad read from input file");}, errno);
    
    int bufferInd = 0;
    while (readenBytes != 0) {
    
        FUNCTION_ASSERT (readenBytes < 0, {perror ("Bad read from input file");}, errno);

        for (bufferInd = 0; bufferInd < readenBytes; bufferInd++) {
        
            if (isspace (CUR_SYMB))
                CatchBizzBuzzSpace (input, &offset, buffer, &readenBytes, &lastWordFlag, 
                                    &mod3, &mod5, &bufferInd, output);
            else {
            
                if (isdigit (CUR_SYMB) && lastWordFlag != BB_LETTERS_) {
                
                    //here we still have a number (or new number)
                    char curNum = CUR_SYMB - '0';
                    mod5 = curNum % 5;
                    mod3 = (mod3 + curNum) % 3;

                    lastWordFlag = BB_NUMBER_;
                
                } else
                    lastWordFlag = BB_LETTERS_;
            
                offset++;

            }

        }

        readenBytes = read (input, buffer, BUFFER_SIZE);
        FUNCTION_ASSERT (readenBytes < 0, {perror ("Bad read from input file");}, errno);

    }

    if (lastWordFlag != BB_SPACE_) 
        return CatchBizzBuzzSpace   (input, &offset, buffer, &bufferInd, &lastWordFlag, 
                                    &mod3, &mod5, &bufferInd, output);

    return 0;   //no errors

}


static const char Bizz [] = "Bez";
static const char Buzz [] = "Bab";

static int CatchBizzBuzzSpace   (const int input, long* offset, char* buffer, int* readenBytes, char* lastWordFlag,
                                char* mod3, char* mod5, int* bufferInd, const int output) {

    SET_ERRNO;

    if (*lastWordFlag == BB_SPACE_) {
   
        int writeChecker = write (output, buffer + *bufferInd, 1);
        FUNCTION_ASSERT (writeChecker < 0, {perror ("Bad write into output file");}, errno);

    } else if (*lastWordFlag == BB_LETTERS_) {
   
        int printCheck = PrintWord (input, offset, buffer, readenBytes, bufferInd, output);
        FUNCTION_ASSERT (printCheck < 0, {perror ("Bad print word");}, errno);
  
    } else {
         
        if (*mod3 == 0 || *mod5 == 0) {
        
            int writeChecker = 0;
            if (*mod3 == 0) {
            
                writeChecker = write (output, Bizz, sizeof (Bizz) - 1);
                FUNCTION_ASSERT (writeChecker < 0, {perror ("Bad write into output file");}, errno);
            
            }
            if (*mod5 == 0) {
            
                writeChecker = write (output, Buzz, sizeof (Buzz) - 1);
                FUNCTION_ASSERT (writeChecker < 0, {perror ("Bad write into output file");}, errno);
            
            }

            writeChecker = write (output, buffer + *bufferInd, 1); //to write space symbol
            FUNCTION_ASSERT (writeChecker < 0, {perror ("Bad write into output file");}, errno);

        } else {
        
            int printCheck = PrintWord (input, offset, buffer, readenBytes, bufferInd, output);
            FUNCTION_ASSERT (printCheck < 0, {perror ("Bad print");}, errno);

        } 
   
    }

    *mod3 = 0;
    *mod5 = 0;

    *offset = 0;

    *lastWordFlag = BB_SPACE_;

    return 0;

}

static int PrintWord (const int input, long* offset, char* buffer, int* readenBytes, int* bufferInd, const int output) { 

    SET_ERRNO;

    int writeChecker = 0;

    if (*offset <= *bufferInd) {
        
        //word still in buffer
        writeChecker = write (output, buffer + (*bufferInd - *offset), *offset + 1); //+1 for space printing
        FUNCTION_ASSERT (writeChecker < 0, {perror ("Bad write into output file");}, errno);

        *offset = 0;

    } else {
        
        long seekCheck = lseek (input, -*offset - (*readenBytes - *bufferInd), SEEK_CUR);
        FUNCTION_ASSERT (seekCheck < 0, {perror ("lseek error");}, errno);
 
        while (*offset > 0) {
            
            *readenBytes = read (input, buffer, BUFFER_SIZE);
            FUNCTION_ASSERT (*readenBytes < 0, {perror ("Bad read from input file");}, errno);

            *offset -= *readenBytes;
            if (*offset >= 0) {
                    
                writeChecker = write (output, buffer, *readenBytes);
                FUNCTION_ASSERT (writeChecker < 0, {perror ("Bad write into output file");}, errno);

                *bufferInd = *readenBytes;

            } else {
                     
                writeChecker = write (output, buffer, *readenBytes + *offset + 1); //+1 to write space symb
                FUNCTION_ASSERT (writeChecker < 0, {perror ("Bad write into output file");}, errno);

                *bufferInd = *readenBytes + *offset;
                
            }
            
        } 

    }

    return writeChecker;

}

