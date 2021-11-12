#include "common.h"

int Output = 0;
int ContinueReceive = 1;
int SizeOfFile = 0;

char OutputBuffer [BUFFER_SIZE] = "";
int CurSymbInd = 0;

pid_t FirstSender = 0;

int CheckInput  (const int argc, char** argv);
int RunReceiver ();

int main (int argc, char** argv) {

    FUNCTION_SECURITY (CheckInput (argc, argv), {}, 0);

    Output = open (argv [1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    FUNCTION_SECURITY (Output < 0, {perror ("Bad output file!");}, errno);

    printf ("Receiver's pid: %u\n", getpid ());

    int receiveErr = RunReceiver ();

    int closeErr = close (Output);
    FUNCTION_SECURITY (closeErr, {perror ("Bad close for output file");}, errno);

    return receiveErr;

}

void SizeHandler (int sigN, siginfo_t* sigInfo, void* context) {
    
    if (IS_NULL (context))
        return;

    if (sigN == SIGUSR2) {

        if (FirstSender == 0)
            FirstSender = sigInfo->si_pid;
        else {

            printf ("Second sender. Try to kill...\n");
            int tryMurder = kill (sigInfo->si_pid, SIGKILL);
            FUNCTION_SECURITY (tryMurder < 0, {perror ("Can't kill second sender");}, );
            FUNCTION_SECURITY (tryMurder >= 0, {printf ("Second sender is killed succesfully!\n");}, );
            
        }

        SizeOfFile = sigInfo->si_value.sival_int;

    }

    int sendSigTry = kill (sigInfo->si_pid, SIGUSR1);
    FUNCTION_SECURITY (sendSigTry < 0, {perror ("Bad signal send to sender");}, );

}

void CharHandler (int sigN, siginfo_t* sigInfo, void* context) {

    if (sigN != SIGUSR1)
        return;
    
    if (IS_NULL (context))
        return;

    static const ssize_t del = sizeof (void*) / sizeof (char);

    pid_t pidToSend = sigInfo->si_pid;

    size_t receiverVoid = (size_t)sigInfo->si_value.sival_ptr;
    // printf ("receiverVoid = %p\n", sigInfo->si_value.sival_ptr);
    int highestBorder = SizeOfFile < del ? SizeOfFile : del;

    for (int i = 0; i < highestBorder; i++) {

        OutputBuffer [CurSymbInd++] = (char)((receiverVoid >> i * 8) & 0xFF);
        SizeOfFile--;

        if (CurSymbInd == BUFFER_SIZE) {

            int writeErr = write (Output, OutputBuffer, BUFFER_SIZE);
            FUNCTION_SECURITY (writeErr < 0, {perror ("Bad write into output file");}, );

            CurSymbInd = 0;

            break;

        }

    }

    if (SizeOfFile == 0)
        ContinueReceive = 0;

    kill (pidToSend, SIGUSR1);

}

int RunReceiver () {

    //*SIZE RECEIVE
    struct sigaction usr2Sig = {0};
    usr2Sig.sa_sigaction = SizeHandler;
    sigemptyset (&usr2Sig.sa_mask);
    usr2Sig.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR2, &usr2Sig, NULL);

    while (SizeOfFile == 0) {sleep (1);}

    // printf ("Size of file = %d\n", SizeOfFile);

    //*BUFFER'S CHAR RECEIVE
    struct sigaction usr1Sig = {0};
    usr1Sig.sa_sigaction = CharHandler;
    sigemptyset (&usr1Sig.sa_mask);
    usr1Sig.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1, &usr1Sig, NULL);

    while (ContinueReceive) {sleep (1);}

    int writeErr = write (Output, OutputBuffer, CurSymbInd);
    FUNCTION_SECURITY (writeErr < 0, {perror ("Bad write into output file");}, errno);

    return 0;
    
}

int CheckInput (const int argc, char** argv) {

    if (IS_NULL (argv)) {

        printf ("Null pointer on argv!\n");
        return -1;

    }

    if (argc < 2) {

        printf ("You have to enter output file!\n");
        return -2;

    }

    return 0;

}
