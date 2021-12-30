#include "../include/common.h"

#include <readline/readline.h>
#include <readline/history.h>

const   char*   NewSrc      = "/home/exactlywb/Desktop/CHERT_REFACTORED/new.src";
const   char*   NewDst      = "/home/exactlywb/Desktop/CHERT_REFACTORED/new.dst";

const   char*   PidPath     = "/home/exactlywb/Desktop/CHERT_REFACTORED/daemon.pid";
        pid_t   DaemonPid   = -1;

int     HandleLine  (const char* line);

int main (int argc, char** argv) {

    if (argc == 2) {

        if (STR_EQ (argv [1], "-help", 5)) {

            printf ("* log - opens new terminal and show logs in lifetime\n");
            printf ("* murder - kills daemon.\n");
            printf ("* timeout <number> - changes daemon's timeout\n");
            printf ("* src <path> - change source\n");
            printf ("* dst <path> - new destination path\n");
            printf ("* inotify - switch on inotify mode");
            printf ("* classic - switch on classic mode");
            printf ("* quit\n");

        }

    }


    //READ PID FILE
    int pidFile = open (PidPath, O_RDONLY, 0666);
    FUNCTION_SECURITY (pidFile == -1, {perror ("bad open ()");}, -1);

    char buffer [BUF_SIZE] = {0};
    int checkRead = read (pidFile, buffer, BUF_SIZE);
    FUNCTION_SECURITY (checkRead == -1, {perror ("bad read ()");}, -1);

    DaemonPid = atoi (buffer);

    close (pidFile);
    //

    while (1) {

        char* line = readline ("$ ");

        if (IS_NULL (line))
            break;
        if (*line)
            add_history (line);

        int checkExit = HandleLine (line);
        FUNCTION_SECURITY (checkExit == -1, {}, 0);

        free (line);

    }

    return 0;
    
}

int HandleLine (const char* line) {

    if (STR_EQ (line, "quit", 4))
        return -1;
    else if (STR_EQ (line, "murder", 6)) {

        int checkKill = kill (DaemonPid, SIGQUIT);
        FUNCTION_SECURITY (checkKill == -1, {perror ("can't kill daemon");}, 0);

    } else if (STR_EQ (line, "timeout", strlen ("timeout"))) {

        line += 7;
        
        while (isspace (*line))
            line++;

        int timeToSet = atoi (line);
        if (timeToSet == 0) {

            printf ("Timeout can't be 0!\n");
            return 0;
            
        }
        
        union sigval val;
        val.sival_int = timeToSet;
        int testKill = sigqueue (DaemonPid, SIGINT, val);
        FUNCTION_SECURITY (testKill == -1, {perror ("bad sigqueu ()");}, 0);

    } else if (STR_EQ (line, "inotify", strlen ("inotify"))) {

        int checkKill = kill (DaemonPid, SIGCHLD);
        FUNCTION_SECURITY (checkKill == -1, {perror ("can't send signal to daemon");}, 0);

    } else if (STR_EQ (line, "classic", strlen ("classic"))) {

        int checkKill = kill (DaemonPid, SIGTRAP);
        FUNCTION_SECURITY (checkKill == -1, {perror ("can't send signal to daemon");}, 0);

    } else if (STR_EQ (line, "src", strlen ("src"))) {
        
        line += 3;
        while (isspace (*line))
            line++;
        
        int fd = open (NewSrc, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        FUNCTION_SECURITY (fd == -1, {perror ("bad src open (): ");}, 0);
        write (fd, line, strlen (line));
        close (fd);

        int checkKill = kill (DaemonPid, SIGUSR1);
        FUNCTION_SECURITY (checkKill == -1, {perror ("bad kill ()");}, 0);      //!I know it's terrible, but I don't wanna 
                                                                                //waste time on codestyle in interface. So sorry.
    } 
    else if (STR_EQ (line, "dst", 3)) {

        line += 3;
        while (isspace (*line))
            line++;
        
        int fd = open (NewDst, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        FUNCTION_SECURITY (fd == -1, {perror ("bad dst open ()");}, 0);
        write (fd, line, strlen (line));
        close (fd);

        int checkKill = kill (DaemonPid, SIGUSR2);
        FUNCTION_SECURITY (checkKill == -1, {perror ("bad kill ()");}, 0);

    }
    else if (STR_EQ (line, "log", 3))
        system ("../scripts/log.sh");
    else
        printf ("Unexpected command!\n");
    
    return 0;

}
