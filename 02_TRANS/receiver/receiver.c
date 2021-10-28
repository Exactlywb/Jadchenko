#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../frolov_define.h"

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

#include <errno.h>

enum InputErrs {
    
    NO_INPUT_ERR,
    NULL_ARGV_PTR,
    WRONG_INPUT_ARGV_NUM

};

enum TransmitterErrs {

    NO_TRANSMITTER_ERRS,
    BAD_OUTPUT_FILE

};

int     CheckInput      (const int argc, char** argv);
void    PrintInputErrs  (const int err, const int line, const char* funct);

int     RunReceiver     ();

int     OutputFile = 0;

int main (int argc, char** argv) {

    int inputErr = CheckInput (argc, argv);
    FUNCTION_ASSERT (inputErr != NO_INPUT_ERR, {PrintInputErrs (inputErr, __LINE__, __func__);}, inputErr);

    OutputFile = open (argv [1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    FUNCTION_ASSERT (OutputFile < 0, {perror ("Bad output file");}, errno);

    int transmitterErr = RunReceiver ();
    FUNCTION_ASSERT (transmitterErr < 0, {}, transmitterErr);

    int closeErr = close (OutputFile);
    FUNCTION_ASSERT (closeErr < 0, {perror ("Bad close for function");}, errno);

    return 0;

}

#define BUFFER_SIZE 500000
char    OutputBuffer [BUFFER_SIZE]  = "";
int     CurrentOutBuffInd           = 0;

int     ContinueReceive             = 1;

inline int WriteIntoOutputFile (pid_t pid) {

    int writeErr = write (OutputFile, OutputBuffer, CurrentOutBuffInd);
    FUNCTION_ASSERT (writeErr < 0, {perror ("Bad write into output file");
                                    kill (pid, SIGUSR1);}, errno);     //!TODO check si_pid

    kill (pid, SIGUSR1);

    return 0;

}

void CharHandler (int sigN, siginfo_t* sigInfo, void* context) {
    
    if (sigN != SIGUSR1)
        return;

    if (context == NULL)
        return;

    pid_t pidToSend = sigInfo->si_pid;
    int curChar = sigInfo->si_value.sival_int;
    if (curChar == 0) { //end of input
        
        ContinueReceive = 0;
        WriteIntoOutputFile (pidToSend);

        return;

    }

    static const ssize_t del = sizeof (int) / sizeof (char);
    char* fullPres = (char*)(&curChar);
    for (ssize_t i = 0; i < del; i++) {
        
        char nextChar = fullPres [i];
        if (nextChar == 0)
            break;

        OutputBuffer [CurrentOutBuffInd++] = nextChar;
        if (CurrentOutBuffInd == BUFFER_SIZE) {
    
            int writeErr = WriteIntoOutputFile (pidToSend);
            FUNCTION_ASSERT (writeErr != 0, {}, );

            CurrentOutBuffInd = 0;
    
        }

    } 

    kill (sigInfo->si_pid, SIGUSR1);

}

inline int RunReceiver () {

    FUNCTION_ASSERT (OutputFile < 0, {printf ("Bad const int output in function %s on line %d\n", __func__, __LINE__);}, BAD_OUTPUT_FILE);

    struct sigaction usr1_sig = {};
    usr1_sig.sa_sigaction = CharHandler;
    sigemptyset (&usr1_sig.sa_mask);
    usr1_sig.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1, &usr1_sig, NULL);

    while (ContinueReceive) {sleep (1);}

    return 0;

}

void PrintInputErrs (const int err, const int line, const char* funct) {

    switch (err) {
    
        case NULL_ARGV_PTR:
            printf ("Null pointer on char** argv.");
            break;
        case WRONG_INPUT_ARGV_NUM:
            printf ("Wrong arguments number. You have to enter output file name only.");
            break;
        case NO_INPUT_ERR:
            return;
        default:
            printf ("Unexpected error.");

    }

    printf ("Error was catched in function %s on line %d\n", funct, line);

}

int CheckInput (const int argc, char** argv) {

    if (IS_NULL (argv))
        return NULL_ARGV_PTR;
    
    if (argc != 2)
        return WRONG_INPUT_ARGV_NUM;

    return NO_INPUT_ERR;

}

