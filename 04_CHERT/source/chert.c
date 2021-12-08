#include "chert.h"

const char* DaemonName = "Timur";
unsigned int AlarmPer = 10;         //SIGALRM every 10 seconds

int RunTimur () {

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

void RunInterface (const char* src, const char* dst, const sigset_t signalsSet) { //!TODO update interface.

    FUNCTION_SECURITY (IS_NULL (src), {printf ("NULL const char* src in function %s\n", __func__);}, );
    FUNCTION_SECURITY (IS_NULL (dst), {printf ("NULL const char* dst in function %s\n", __func__);}, );

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
            default:
                syslog (LOG_ERR, "Unexpected signal %d\n", signal);

        }

    }

}

int SetSignalsSettings (sigset_t* signalsSet) {

    //!TODO

    return 0;

}
