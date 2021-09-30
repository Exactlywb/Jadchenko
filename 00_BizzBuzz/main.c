#include "BizzBuzz.h"
#include <string.h>

typedef enum INPUT_ERRORS {

    NO_INPUT_ERRORS_,
    NO_SRC_DST_,
    NO_DST_,
    NULL_ARGV_P,
    SAME_FILES,
    UNEXPECTED_ERR_

} INPUT_ERRORS;
int     CheckInput      (const int argc, char** argv);
void    PrintInputErr   (const int err);

int main (int argc, char** argv) {

    int inputErr = CheckInput (argc, argv);
    FUNCTION_ASSERT (inputErr != NO_INPUT_ERRORS_, {PrintInputErr (inputErr);}, inputErr);

    RunBizzBuzz (argc, argv); 

    return 0;

} 

#define STR_EQ(str1, str2) !strcmp (str1, str2)

int CheckInput (const int argc, char** argv) {

    if (argc == 3) {
    
        if (IS_NULL (argv))
            return NULL_ARGV_P;
        else {
        
            if (STR_EQ(argv [1], argv [2])) {
            
                return SAME_FILES;
            
            }

            return NO_INPUT_ERRORS_;
    
        }

    } else {
    
        switch (argc) {
        
            case 1:
                return NO_SRC_DST_;
            case 2:
                return NO_DST_;
            default:
                return UNEXPECTED_ERR_;
        
        }
    
    }

}

void PrintInputErr (const int err) {

    switch (err) {
        
        case NO_INPUT_ERRORS_:
            return;
        case NO_SRC_DST_:
            printf ("No source and destination files!\n");
            return;
        case NO_DST_:
            printf ("No destination file!\n");
            return;
        case NULL_ARGV_P:
            printf ("Null pointer on char** argv");
            return;
        case SAME_FILES:
            printf ("Source and destination files can't be the same!\n");
            return;
        default:
            printf ("Unexpected err!\n");

    }

}

