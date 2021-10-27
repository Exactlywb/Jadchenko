#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <errno.h>

#include "../frolov_define.h"

enum ProducerErrs {

    NO_ERR,
    BAD_PID_PARSE

};

int         CheckInput              (const int argc, char** argv);

int         RunProducer             (const int input, const pid_t consumer);
int         RunByteByByteOutput     (const pid_t consumer, const char* buffer, const ssize_t readenBytes);

int main (int argc, char** argv) {

    int errInput = CheckInput (argc, argv);
    FUNCTION_ASSERT (errInput != 0, {}, errInput);

    int inputFile = open (argv [1], O_RDONLY);
    FUNCTION_ASSERT (inputFile < 0, {perror ("Bad input file");}, inputFile); 

    pid_t receiver = (pid_t)strtoll (argv [2], NULL, 0);
    FUNCTION_ASSERT (receiver == (pid_t)LONG_MAX || 
                     receiver == (pid_t)LONG_MIN || 
                     receiver == 0, {printf ("Bad parse for pid\n");}, BAD_PID_PARSE);

    int producerErr = RunProducer (inputFile, receiver);

    close (inputFile);

    return producerErr;

}

#define BUFFER_SIZE 100000
int RunProducer (const int input, const pid_t receiver) {
    
    char buffer [BUFFER_SIZE] = ""; 
    
    ssize_t readenBytes     = read (input, buffer, BUFFER_SIZE);
    while (readenBytes != 0) {
    
        FUNCTION_ASSERT (readenBytes < 0, {perror ("Bad read from input file");}, errno);
            
        int outErr = RunByteByByteOutput (receiver, buffer, readenBytes);
        FUNCTION_ASSERT (outErr != NO_ERR, {}, outErr);

        readenBytes = read (input, buffer, BUFFER_SIZE);

    }
    printf ("Sended 0\n");
    buffer [0] = 0;
    readenBytes = 1;
    int outErr = RunByteByByteOutput (receiver, buffer, readenBytes);
    FUNCTION_ASSERT (outErr != NO_ERR, {}, outErr);
    printf ("Now I have to end it\n");
    return NO_ERR;

}

int stoppedSending = 1;

void ControlHandler (int sigN) {
    
    if (sigN == SIGUSR1)
        stoppedSending = 0;

}

int RunByteByByteOutput (const pid_t receiver, const char* buffer, const ssize_t readenBytes) {
    
    struct sigaction usr1_sig = {};
    usr1_sig.sa_handler = ControlHandler;

    sigemptyset (&usr1_sig.sa_mask);

    sigaction(SIGUSR1, &usr1_sig, NULL);
    
    for (ssize_t i = 0; i < readenBytes; i++) {

        stoppedSending = 1;
        
        union sigval sendedChar;
        sendedChar.sival_int = buffer [i];

        int sendSigErr = sigqueue (receiver, SIGUSR1, sendedChar);
        FUNCTION_ASSERT (sendSigErr < 0, {perror ("Bad send for a signal");}, errno);

        while (stoppedSending) { sleep (1); }

    }

    return NO_ERR;

}

int CheckInput (const int argc, char** argv) {

    if (argc != 3) {
        
        printf ("U have to enter: input file and consumer pid\n");
        return -1;
    
    }

    if (IS_NULL (argv)) {
    
        printf ("Nullptr on char** argv");
        return -1;

    }

    return 0;

}
