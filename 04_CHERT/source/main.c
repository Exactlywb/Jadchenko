#include "chert.h"

typedef enum InputErrs {

    INPUT_OK,
    NO_INPUT_PATH,
    NO_OUTPUT_PATH,
    TOO_MUCH_ARGS,
    NULL_ARGV,          //How can it be? It doesn't matter
    EQ_DST_SRC

} InputErrs;

extern  char* DaemonName;
extern  char* NewPathFileName;

int     CheckInput      (const int argc, char** argv);
void    PrintInputErr   (const int err);

//!TODO add checking for existing daemon

int main (int argc, char** argv) {
    
    int checkInp = CheckInput (argc, argv);
    FUNCTION_SECURITY (checkInp != INPUT_OK, {PrintInputErr (checkInp);}, -1);    

    //This is a Timur. Post-modernistic chert (daemon).
    int checkDaemon = RunTimur ();
    FUNCTION_SECURITY (checkDaemon == -1, {}, -1);

    openlog (DaemonName, LOG_PID, LOG_DAEMON);

    //Running interface
    sigset_t setSignals = {};
    int checkSignalsSetting = SetSignalsSettings (&setSignals);
    FUNCTION_SECURITY (checkSignalsSetting == -1, {}, -1);

    RunInterface (argv [1], argv [2], setSignals);

    syslog (LOG_NOTICE, "Timur is dying...");
    closelog ();

    return 0;

}

int CheckInput (const int argc, char** argv) {

    FUNCTION_SECURITY (IS_NULL (argv)                                   , {}, NULL_ARGV);
    FUNCTION_SECURITY (argc == 1                                        , {}, NO_INPUT_PATH);
    FUNCTION_SECURITY (argc == 2                                        , {}, NO_OUTPUT_PATH);
    FUNCTION_SECURITY (argc >  3                                        , {}, TOO_MUCH_ARGS);
    FUNCTION_SECURITY (STR_EQ (argv [0], argv [1], strlen (argv [0]))   , {}, EQ_DST_SRC);

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
