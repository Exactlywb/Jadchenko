#include "common.h"

int StoppedSending;

inline  int     CheckInput          (const int argc, char** argv);

        int     RunSender           (const int input, const pid_t receiver, const off_t size);
inline  int     RunSenderConveyer   (const pid_t receiver, const unsigned char* buffer, const ssize_t readenBytes);
inline  int     SendFileSize        (const pid_t receiver, const off_t size);

        void    ControlHandler      (int sigN);

        off_t   GetFileSize         (const int file);

int main (int argc, char** argv) {

    FUNCTION_SECURITY (CheckInput (argc, argv), {}, 0);

    clock_t begin = clock ();

    int input = open (argv [1], O_RDONLY);
    FUNCTION_SECURITY (input < 0, {perror ("Bad input file");}, errno);
    
    off_t size = GetFileSize (input);
    // printf ("size of file = %ld\n", size);

    pid_t receiver = (pid_t)strtoll (argv [2], NULL, 0);
    FUNCTION_SECURITY (receiver == (pid_t)LONG_MAX || 
                       receiver == (pid_t)LONG_MIN || 
                       receiver == 0, {printf ("Bad parse for pid\n");}, 0);

    int senderErr = RunSender (input, receiver, size);

    int closeErr = close (input);
    FUNCTION_SECURITY (closeErr, {perror ("Bad close for input file");}, errno);

    clock_t end = clock ();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf ("Speed: %f MB / s\n", (size / time_spent) / 1e6);

    return senderErr;

}

int RunSender (const int input, const pid_t receiver, const off_t size) {

    StoppedSending = 1;

    struct sigaction usr1_sig = {0};
    usr1_sig.sa_handler = ControlHandler;

    sigemptyset (&usr1_sig.sa_mask);

    sigaction (SIGUSR1, &usr1_sig, NULL);

    int sendSizeErr = SendFileSize (receiver, size);
    FUNCTION_SECURITY (sendSizeErr, {}, sendSizeErr);

    while (StoppedSending) {sleep (1);}

    unsigned char buffer [BUFFER_SIZE] = "";
    
    ssize_t readenBytes = read (input, buffer, BUFFER_SIZE);
    while (readenBytes != 0) {

        int outErr = RunSenderConveyer (receiver, buffer, readenBytes);
        FUNCTION_SECURITY (outErr, {}, outErr);

        readenBytes = read (input, buffer, BUFFER_SIZE);
        
    }

    return 0;

}

void ControlHandler (int sigN) {
    
    if (sigN == SIGUSR1)
        StoppedSending = 0;

}

inline void BuildSend (union sigval* sendedChar, const unsigned char* buffer, const ssize_t readenBytes, ssize_t* i) {

    static const ssize_t del = sizeof (void*) / sizeof (char);
    // printf ("del = %zu\n", del);
    static const ssize_t charSize = sizeof (char);

    size_t buildedPtr = 0;

    size_t highBorder = (readenBytes - *i) < del ? (readenBytes - *i) : del;
    // printf ("highBorder = %zu\n", highBorder);
    for (size_t curByte = 0; curByte < highBorder; curByte++) {

        // printf ("curByte [%zu] = %c\n", curByte, (buffer [(*i)]));
        buildedPtr |= (((size_t)buffer [(*i)++]) << curByte * charSize * 8);
    
    }

    sendedChar->sival_ptr = (void*)buildedPtr;

}

inline int RunSenderConveyer (const pid_t receiver, const unsigned char* buffer, const ssize_t readenBytes) {
    
    for (ssize_t i = 0; i < readenBytes;) {

        StoppedSending = 1;
        
        union sigval sendedChar;
        BuildSend (&sendedChar, buffer, readenBytes, &i);
        // printf ("sended void: %p\n", sendedChar.sival_ptr);

        int sendSigErr = sigqueue (receiver, SIGUSR1, sendedChar);
        FUNCTION_SECURITY (sendSigErr < 0, {perror ("Bad send for a signal");}, errno);

        while (StoppedSending) { 
        
            int receiverStillAlive = kill (receiver, 0);
            FUNCTION_SECURITY (receiverStillAlive < 0, {perror ("Receiver is dead. Me too..."); exit (errno);}, errno);
            sleep (1); 
       
        }

    }

    return 0;

}

inline int SendFileSize (const pid_t receiver, const off_t size) {

    union sigval toSend;
    toSend.sival_int = (int)size;

    int sendSigErr = sigqueue (receiver, SIGUSR2, toSend);
    FUNCTION_SECURITY (sendSigErr < 0, {perror ("Bad send for a signal!\n");}, errno);

    return 0;

}

//=========================
//==========COMMON=========
//=========================
off_t GetFileSize (const int file) {

    off_t curPos = lseek (file, 0, SEEK_CUR);
    off_t resSize = lseek (file, 0, SEEK_END);
    lseek (file, curPos, SEEK_SET);

    return resSize - curPos;

}

int CheckInput (const int argc, char** argv) {

    if (IS_NULL (argv)) {

        printf ("Null pointer on argv!\n");
        return -1;

    }

    if (argc < 3) {

        printf ("You have to enter input file and sender pid!\n");
        return -2;

    }

    return 0;

}
