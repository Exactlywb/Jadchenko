#include "daemon.h"

/*FUNCTIONS*/
static          int     CreatePidFile   ();
static  inline  void    HandleSignals   (const int signal, siginfo_t* siginfo);

static  inline  void    ChangePath      (char** src, const int type);

/*CONSTS*/
static  const               char*   PidFilePath         = "/home/exactlywb/Desktop/CHERT_REFACTORED/daemon.pid";
static  const   unsigned    int     DefaultWaitTime     = 5;

static  const               char*   NewDst              = "/home/exactlywb/Desktop/CHERT_REFACTORED/new.dst";
static  const               char*   NewSrc              = "/home/exactlywb/Desktop/CHERT_REFACTORED/new.src";

/*ENUMS*/
enum {

    CHANGE_SRC,
    CHANGE_DST

};

/*GLOBAL VARS*/
static          CopyType            CurCopyMode = CLASSIC_MODE__;

int RunDaemon (const char* daemonName) {

    pid_t checkFork = fork ();

    FUNCTION_SECURITY (checkFork < 0, {perror ("bad fork ()");}, -1);      //fork err handling
    FUNCTION_SECURITY (checkFork > 0, {exit (0);}, 0);                              //parent terminate
    FUNCTION_SECURITY (setsid () < 0, {perror ("bad setsid ()"); }, -1);   //child process becomes session leader

    //Fork again.
    checkFork = fork ();
    FUNCTION_SECURITY (checkFork < 0, {perror ("bad fork ()");}, -1);      //fork err handling
    FUNCTION_SECURITY (checkFork > 0, {exit (0);}, 0);                              //parent terminate

    umask (0);                                                                      //new file permissions
    chdir ("/");                                                                    //change the working directory to the root directory

    int fileDNum = sysconf (_SC_OPEN_MAX);
    for (; fileDNum >= 0; fileDNum--)
        close (fileDNum);

    int checkDaemonExists = CreatePidFile ();
    FUNCTION_SECURITY (checkDaemonExists == -1, {

        printf ("Daemon already exists! %s is dying...\n", daemonName);
        exit (0);

    }, -1);

    openlog (daemonName, LOG_PID, LOG_DAEMON);

    return 0;

}

int SetSignalsSettings (sigset_t* signalsSet) {

    FUNCTION_SECURITY (IS_NULL (signalsSet), {printf ("NULL sigset_t& signalsSet in function %s\n", __func__);}, -1);
    
    sigemptyset (signalsSet);

    sigaddset (signalsSet, SIGALRM);        //to set copy timeout
    sigaddset (signalsSet, SIGQUIT);        //general exit for user
    sigaddset (signalsSet, SIGINT);         //change copy timeout
    sigaddset (signalsSet, SIGUSR1);        //change src
    sigaddset (signalsSet, SIGUSR2);        //change dst
    sigaddset (signalsSet, SIGTRAP);        //change to classic mode
    sigaddset (signalsSet, SIGCHLD);        //change to inotify mode

    FUNCTION_SECURITY (sigprocmask (SIG_BLOCK, signalsSet, NULL), {syslog (LOG_ERR, "bad sigprocmask () in function %s\n", __func__);}, -1);

    return 0;

}

void RunInterface (char* src, char* dst, const sigset_t signalsSet) {

    FUNCTION_SECURITY (IS_NULL (src), {syslog (LOG_ERR, "NULL const char* src in function %s\n", __func__);}, );
    FUNCTION_SECURITY (IS_NULL (dst), {syslog (LOG_ERR, "NULL const char* dst in function %s\n", __func__);}, );

    syslog (LOG_INFO, "Running Timur's interface...");

    siginfo_t siginfo = {};
    
    int waitTime = DefaultWaitTime;
    alarm (waitTime);
    
    Watch* inotifyWatches = NULL;
    while (1) {

        int signal = sigwaitinfo (&signalsSet, &siginfo);

        switch (signal) {

            case SIGALRM:
                alarm (waitTime);
                
                if (CurCopyMode == CLASSIC_MODE__) {

                    syslog (LOG_INFO, "Regular coercion copy...");
                    RegularCopy (src, dst);

                } else if (CurCopyMode == INOTIFY_MODE__) {

                    syslog (LOG_INFO, "Inotify copy...");
                    InotifyCopy (inotifyWatches, src, dst);

                } else
                    syslog (LOG_ERR, "Unexpected copy mode");

                break;
            case SIGQUIT:
                syslog (LOG_INFO, "User decided to kill Timur :(");
                free (src);
                free (dst);
                return;
            case SIGUSR1:   //change src
                ChangePath (&src, CHANGE_SRC);
                break;
            case SIGUSR2:   //change dst
                ChangePath (&dst, CHANGE_DST);
                break;
            case SIGTRAP:   //change to classic mode
                if (inotifyWatches) {
                    
                    WatchDestructor (inotifyWatches);
                    inotifyWatches = NULL;

                }
                
                CurCopyMode = CLASSIC_MODE__;
                syslog (LOG_INFO, "Copy mode set as classic");

                break;
            case SIGCHLD:   //change to inotify mode

                if (inotifyWatches) {

                    syslog (LOG_INFO, "Inotify mode is already set");
                    break;

                }

                inotifyWatches = WatchConstructor (10000);      //!TODO realloc
                RunInotifyViewer (inotifyWatches, src, dst);    //!TODO check / on the end
                syslog (LOG_INFO, "Copy mode set as inotify");

                CurCopyMode = INOTIFY_MODE__;

                break;
            case SIGINT:
                waitTime = siginfo.si_value.sival_int;
                alarm (waitTime);
                syslog (LOG_INFO, "New copy timeout is %d seconds\n", waitTime);
                break;
            default:
                syslog (LOG_ERR, "Unexpected signal %d\n", signal);

        }

    }

    syslog (LOG_INFO, "is dying...");

    closelog ();

}

static inline void ChangePath (char** src, const int type) {

    syslog (LOG_INFO, "trying change path...");

    int fd = 0;
    if (type == CHANGE_DST) 
        fd = open (NewDst, O_RDONLY, 0666);
    else if (type == CHANGE_SRC)
        fd = open (NewSrc, O_RDONLY, 0666);
    else {

        syslog (LOG_ERR, "Unexpected type of changing in function %s\n", __func__);
        return;

    }

    FUNCTION_SECURITY (fd == -1, {syslog (LOG_ERR, "bad open (): %s", STR_ERR);}, );
    
    char buffer [BUF_SIZE] = {0};
    ssize_t readenBytes = read (fd, buffer, BUF_SIZE);

    free (*src);

    *src = (char*)calloc ((size_t)readenBytes + 1, sizeof (char));
    FUNCTION_SECURITY (IS_NULL (*src), {}, );

    strncpy (*src, buffer, readenBytes);

    close (fd);

    if (type == CHANGE_SRC)
        syslog (LOG_INFO, "source path was changed to %s", *src);
    else
        syslog (LOG_INFO, "destination path was changed to %s", *src);

}

static int CreatePidFile () {
    
    int fd = open (PidFilePath, O_RDONLY, 0666);
    if (fd > 0) {   //file already exists

        char buffer [BUF_SIZE] = {0};
        read (fd, buffer, BUF_SIZE);
        pid_t checkPid = atoi (buffer);

        printf ("Check %d for existing...", checkPid);
        int checkExists = kill (checkPid, 0);

        FUNCTION_SECURITY (checkExists == 0, {}, -1);

    }

    fd = open (PidFilePath, O_RDWR | O_CREAT | O_TRUNC, 0666);
    FUNCTION_SECURITY (fd == -1, {syslog (LOG_ERR, "Bad open () for pid file: %s\n", STR_ERR);}, -1);

    char buffer [BUF_SIZE] = {0};
    snprintf (buffer, BUF_SIZE, "%ld", (long)getpid ());

    FUNCTION_SECURITY   (write (fd, buffer, strlen (buffer)) != strlen (buffer),
                        {syslog (LOG_ERR, "bad write into pid file: %s", STR_ERR);}, -1);

    close (fd);

    return 0;

}
