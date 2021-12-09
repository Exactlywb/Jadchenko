#include "chert.h"

const char* DaemonName = "Timur";
unsigned int AlarmPer = 10;         //SIGALRM every 10 seconds
const char* NewPathFileName = "/home/exactlywb/Desktop/Jadchenko/04_CHERT/newPath";

#define BUF_SIZE 1000

static      void        ChangePath  (char** path);

int RunTimur () {

    printf ("%s\n", NewPathFileName);
    int pathFile = open (NewPathFileName, O_CREAT, 0666);
    FUNCTION_SECURITY (pathFile < 0, {perror ("Bad open ()");}, -1);
    close (pathFile);
    
    pid_t checkFork = fork ();

    FUNCTION_SECURITY (checkFork < 0, {perror ("bad fork ()");}, -1);               //fork err handling
    FUNCTION_SECURITY (checkFork > 0, {exit (0);}, 0);                              //parent terminate
    FUNCTION_SECURITY (setsid () < 0, {perror ("bad setsid ()"); }, -1);            //child process becomes session leader

    //Fork again.
    checkFork = fork ();
    FUNCTION_SECURITY (checkFork < 0, {perror ("bad fork ()");}, -1);               //fork err handling
    FUNCTION_SECURITY (checkFork > 0, {exit (0);}, 0);                              //parent terminate

    umask (0);                                                                      //new file permissions
    chdir ("/");                                                                    //change the working directory to the root directory

    int fileDNum = sysconf (_SC_OPEN_MAX);
    for (; fileDNum >= 0; fileDNum--)
        close (fileDNum);

    return 0;

}

void RunInterface (char* src, char* dst, const sigset_t signalsSet) { //!TODO update interface.

    FUNCTION_SECURITY (IS_NULL (src), {syslog (LOG_ERR, "NULL const char* src in function %s\n", __func__);}, );
    FUNCTION_SECURITY (IS_NULL (dst), {syslog (LOG_ERR, "NULL const char* dst in function %s\n", __func__);}, );

    syslog (LOG_INFO, "Running Timur's interface...");

    siginfo_t siginfo = {};

    unsigned int secondsToWait = alarm (AlarmPer);

    while (1) {

        int signal = sigwaitinfo (&signalsSet, &siginfo);

        switch (signal) {

            case SIGALRM:
                secondsToWait = alarm (AlarmPer);
                CopyDir (src, dst);
                syslog (LOG_INFO, "Copied %s into %s\n", src, dst);
                break;
            case SIGQUIT:
                syslog (LOG_INFO, "User decided to kill Timur :(");
                return;
            case SIGUSR1:   //change src
                ChangePath (&src);
                syslog (LOG_INFO, "New src %s\n", src);
                break;
            case SIGUSR2:   //change dst
                ChangePath (&dst);
                syslog (LOG_INFO, "New dst %s\n", dst);
                break;
            case SIGINT:
                AlarmPer = siginfo.si_value.sival_int;
                syslog (LOG_INFO, "New copy timout %d seconds\n", AlarmPer);
                break;
            default:
                syslog (LOG_ERR, "Unexpected signal %d\n", signal);

        }

    }

}

int SetSignalsSettings (sigset_t* signalsSet) {

    FUNCTION_SECURITY (IS_NULL (signalsSet), {printf ("NULL sigset_t& signalsSet in function %s\n", __func__);}, -1);
    
    sigemptyset (signalsSet);

    sigaddset (signalsSet, SIGALRM);        //to set copy timeout
    sigaddset (signalsSet, SIGQUIT);        //general exit for user
    sigaddset (signalsSet, SIGINT);         //change copy timeout
    sigaddset (signalsSet, SIGUSR1);        //change src
    sigaddset (signalsSet, SIGUSR2);        //change dst

    FUNCTION_SECURITY (sigprocmask (SIG_BLOCK, signalsSet, NULL), {syslog (LOG_ERR, "bad sigprocmask () in function %s\n", __func__);}, -1);

    return 0;

}

static void ChangePath (char** path) {

    static int checkFree = 0;

    FUNCTION_SECURITY (IS_NULL (path), {syslog (LOG_ERR, "NULL char** path in function %s\n", __func__);}, );

    int pathFile = open (NewPathFileName, O_RDONLY, 0666);
    FUNCTION_SECURITY (pathFile < 0, {syslog (LOG_ERR, "bad open () in function %s\n", __func__);}, );

    char buff [BUF_SIZE] = {0};
    int checkRead = read (pathFile, buff, BUF_SIZE);
    syslog (LOG_INFO, "Readen from file %s\n", buff);

    size_t bufSize = strlen (buff);
    char* newPath = (char*)calloc (bufSize + 1, sizeof (char));
    FUNCTION_SECURITY (IS_NULL (newPath), {syslog (LOG_ERR, "Bad alloc in function %s\n", __func__);}, );

    strncpy (newPath, buff, bufSize);
    
    if (checkFree)
        free (*path);
    else
        checkFree = 1;
    
    *path = newPath;

    close (pathFile);

}
