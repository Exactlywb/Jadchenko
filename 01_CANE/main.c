#include "Cane.h"

typedef enum InputErrors {

    NO_INPUT_ERR,
    NO_INPUT_FILE,
    TOO_MUCH_ARGS,
    NULL_ARGV_PTR

} InputErrors;

#define PRINT_INPUT_ERR(err) PrintInputErrs (err, __func__, __LINE__, __FILE__)

int     CheckInput      (const int argc, char** argv);
void    PrintInputErrs  (const int err, const char* func, const int line, const char* file);

int main (int argc, char** argv) {

    SET_ERRNO;

    int inputErrs = CheckInput (argc, argv);
    FUNCTION_PROTECT (inputErrs != NO_INPUT_ERR, {PRINT_INPUT_ERR (inputErrs);}, inputErrs);

    int input = open (argv [1], O_RDONLY | O_CLOEXEC);
    FUNCTION_PROTECT (input < 0, {  close (input); 
                                    perror ("can't open input file");}, errno);

    return RunConveyer (argv, input);

}

int CheckInput (const int argc, char** argv) {

    if (argc != 2) {
    
        if (argc > 2)
            return TOO_MUCH_ARGS;
        else
            return NO_INPUT_FILE;

    }
        
    if (IS_NULL (argv))
        return NULL_ARGV_PTR;

    return NO_INPUT_ERR;

}

void PrintInputErrs (const int err, const char* func, const int line, const char* file) {

    #define PRINT_ERR(msg) printf ("Error [%d]: %s. Error was catched on line %d in function %s in file %s.\n", \
                                    err, msg, line, func, file)

    switch (err) {
    
        case NO_INPUT_ERR:
            printf ("Success!\n");
            return;            
        case NO_INPUT_FILE:
            PRINT_ERR ("no input file found");
            return;
        case TOO_MUCH_ARGS:
            PRINT_ERR ("you have to enter only 1 arg (input file name)");
            return;
        case NULL_ARGV_PTR:
            PRINT_ERR ("null pointer on argv ptr");
            return;
        default:
            PRINT_ERR ("unexpected error");

    }

}

