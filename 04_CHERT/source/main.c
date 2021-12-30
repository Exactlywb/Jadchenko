#include "daemon.h"
#include "copy.h"

typedef enum InputErrs {

    INPUT_OK,
    NO_INPUT_PATH,
    NO_OUTPUT_PATH,
    TOO_MUCH_ARGS,
    NULL_ARGV,          //How can it be? It doesn't matter
    EQ_DST_SRC

} InputErrs;

int     CheckInput      (const int argc, char** argv);
void    PrintInputErr   (const int err);

int main (int argc, char** argv) {

    int checkInput = CheckInput (argc, argv);
    FUNCTION_SECURITY (checkInput != INPUT_OK, {PrintInputErr (checkInput);}, -1);

    int checkDaemon = RunDaemon ("Timur");
    FUNCTION_SECURITY (checkDaemon == -1, {}, -1);

    sigset_t setSignals = {};
    int checkSignalsSetting = SetSignalsSettings (&setSignals);
    FUNCTION_SECURITY (checkSignalsSetting == -1, {syslog (LOG_ERR, "bad signals settings: %s", STR_ERR);}, -1);

    int srcLength = strlen (argv [1]);
    char* src = (char*)calloc (srcLength + 1, sizeof (char));
    FUNCTION_SECURITY (IS_NULL (src), {}, -1);
    strncpy (src, argv [1], srcLength);

    int dstLength = strlen (argv [2]);
    char* dst = (char*)calloc (dstLength + 1, sizeof (char));
    FUNCTION_SECURITY (IS_NULL (dst), {}, -1);
    strncpy (dst, argv [2], dstLength);

    RunInterface (src, dst, setSignals);

    return 0;

}

int CheckInput (const int argc, char** argv) {

    FUNCTION_SECURITY (IS_NULL (argv)                                   , {}, NULL_ARGV);
    FUNCTION_SECURITY (argc == 1                                        , {}, NO_INPUT_PATH);
    FUNCTION_SECURITY (argc == 2                                        , {}, NO_OUTPUT_PATH);
    FUNCTION_SECURITY (argc >  3                                        , {}, TOO_MUCH_ARGS);   //!TODO check real path 
    FUNCTION_SECURITY (STR_EQ (argv [1], argv [2], strlen (argv [1]))   , {}, EQ_DST_SRC);      //!TODO upgrade

    return INPUT_OK;

}

void PrintInputErr (const int err) {

    #define PRINT_ERR(msg)  printf ("Error [%d]: %s\n", err, msg)

    switch (err) {

        case NULL_ARGV:
            PRINT_ERR ("null pointer on char** argv");
            return;
        case NO_INPUT_PATH:
        case NO_OUTPUT_PATH:
            PRINT_ERR ("you have to enter input dir and output path");
            return;
        case TOO_MUCH_ARGS:
            PRINT_ERR ("too much input arguments");
            return;
        case EQ_DST_SRC:
            PRINT_ERR ("source and destination folders can't be equal");
            return;
        default:
            PRINT_ERR ("unexpected error");

    }

    #undef PRINT_ERR

}
