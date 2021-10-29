#include "Cane.h"

static      void        PrintConveyerErr    (const int err, const int line, const char* function, const char* file);

static      size_t      GetCommandsNum      (char* buffer);
static      int         SeparateCommands    (char* buffer, Command* commandsArr);

static      int         HandleArgc          (char* buffer);
static      char**      HandleArgv          (char* buffer, const int argc);

static      int         ExecCommands        (const Command* commands, size_t commandsNum);

static      void        SkipTrash           (char** buffer);

static      Command*    CommandsConstr      (const size_t commandsNum);
static      void        CommandsDstr        (Command* commandsArr, const size_t commandsNum);

#define PRINT_CONVEYER_ERR(err) PrintConveyerErr (err, __LINE__, __FUNCTION__, __FILE__)

int RunConveyer (char** argv, const int input) {

    SET_ERRNO;

    FUNCTION_PROTECT (input < 0, {PRINT_CONVEYER_ERR (BAD_INPUT_DESC);}, BAD_INPUT_DESC);

    const size_t fileSize = GetFileSize (argv [1]);
    FUNCTION_PROTECT (fileSize == 0, {PRINT_CONVEYER_ERR (EMPTY_FILE);}, EMPTY_FILE);

    char* buffer = (char*)calloc (fileSize + 1, sizeof (*buffer));
    FUNCTION_PROTECT (IS_NULL (buffer), {PRINT_CONVEYER_ERR (BAD_ALLOC);}, BAD_ALLOC);

    int readErr = read (input, buffer, fileSize);
    FUNCTION_PROTECT (readErr < 0, {perror ("bad file read");}, errno);
    buffer [fileSize] = 0;

//    close (input);
    
    size_t commandsNum = GetCommandsNum (buffer);

    Command* commandsArr = CommandsConstr (commandsNum);
    FUNCTION_PROTECT (IS_NULL (commandsArr), {  free (buffer);
                                                PRINT_CONVEYER_ERR (BAD_ALLOC);}, BAD_ALLOC);
    
    int sepErr = SeparateCommands (buffer, commandsArr);
    FUNCTION_PROTECT (sepErr != NO_CONVEYER_ERR, {  CommandsDstr (commandsArr, commandsNum);
                                                    free (buffer);
                                                    PRINT_CONVEYER_ERR (sepErr);}, sepErr);
    
    int execErr = ExecCommands (commandsArr, commandsNum);

    free (buffer);
    
    CommandsDstr (commandsArr, commandsNum);
    
    return NO_CONVEYER_ERR;

}

size_t GetFileSize (const char* file) {

    struct stat fileInfo = {};
    stat (file, &fileInfo);

    return fileInfo.st_size - 1;

}

static const char SeparateSymb = '|';

static size_t GetCommandsNum (char* buffer) {

    size_t n_commands = 1; 
    
    char* nextCommand = NULL;
    while ((nextCommand = strchr (buffer, SeparateSymb)) != NULL) {
    
        n_commands++;
        buffer = nextCommand;
        buffer++;
    
    }

    return n_commands; 

}

static int SeparateCommands (char* buffer, Command* commandsArr) {
    
    size_t  commandsNum = 0; 

    char* curRequire = strtok (buffer, "|");
    while (curRequire) {
        
        SkipTrash (&curRequire);

        int curArgc = HandleArgc (curRequire);
        FUNCTION_PROTECT (curArgc <= 0, {PRINT_CONVEYER_ERR (NO_ANY_COMMAND);}, NO_ANY_COMMAND);
        
        commandsArr [commandsNum].argc = curArgc;
        
        commandsArr [commandsNum].argv = HandleArgv (curRequire, curArgc);
        FUNCTION_PROTECT (IS_NULL(commandsArr [commandsNum].argv), {PRINT_CONVEYER_ERR (BAD_ALLOC);}, BAD_ALLOC);

        curRequire = strtok (NULL, "|");
    
        commandsNum++;

    }

    return NO_CONVEYER_ERR;

}

static int HandleArgc (char* buffer) {

    int argc = 0;
    
    while (*buffer != 0) {
    
        if (isspace (*buffer)) {
        
            argc++;
            SkipTrash (&buffer);
        
        } else 
            buffer++;
    
    }

    if (!isspace(*(buffer - 1)))
        argc++;

    return argc;

}

static char** HandleArgv (char* buffer, const int argc) {

    char** argv = (char**)calloc (argc + 1, sizeof (*argv));
    FUNCTION_PROTECT (IS_NULL (argv), {PRINT_CONVEYER_ERR (BAD_ALLOC);}, NULL);

    int curArgvInd = 0;
    argv [curArgvInd++] = buffer;
    
    char* nextWord = NULL;
    while ((nextWord = strchr (buffer, ' ')) && curArgvInd < argc) {
    
        char* toReplace = nextWord;
        SkipTrash (&nextWord);
        *toReplace = 0;

        argv [curArgvInd++] = nextWord;

        buffer = nextWord;
    
    }

    for (int curArg = 0; curArg < argc; curArg++) { //to set '\0' at the end of string
    
        char* curArgStr = argv [curArg];

        size_t curSymb = 0;
        for (; !isspace (curArgStr [curSymb]) && curArgStr [curSymb] != 0; curSymb++) {}
    
        curArgStr [curSymb] = 0;

    }

    argv [argc] = NULL;

    return argv;

}

static int ExecCommands (const Command* commands, size_t commandsNum) {

    int *fd = (int*)calloc (2 * commandsNum - 2, sizeof (int)); //first stdin and last stdout mustn't been allocated
    FUNCTION_PROTECT (IS_NULL (fd), {PRINT_CONVEYER_ERR (BAD_ALLOC);}, BAD_ALLOC);

    SET_ERRNO;
    for (int i = 0; i < commandsNum - 1; i++) {
    
        int checkPipe = pipe (fd + 2 * i);
        FUNCTION_PROTECT (checkPipe < 0, {  free (fd);
                                            PRINT_CONVEYER_ERR (BAD_PIPE);}, BAD_PIPE);
    
    }

    //EXEC PART
    pid_t curPid = 0;
    for (int i = 0; i < commandsNum; i++) {
    
        SET_ERRNO;
        curPid = fork ();
        FUNCTION_PROTECT (curPid < 0, { free (fd);
                                        PRINT_CONVEYER_ERR (BAD_FORK_CALL);}, BAD_FORK_CALL);

        if (curPid == 0) { //child
        
            if (i != 0) {
                FUNCTION_PROTECT (dup2 (fd [2 * i - 2], STDIN_FILENO) < 0, {free (fd);
                                                                            perror ("Bad input dup2");}, errno);
            }
            if (i != commandsNum - 1) {
                FUNCTION_PROTECT (dup2 (fd [2 * i + 1], STDOUT_FILENO) < 0, {   free (fd);
                                                                                perror ("Bad output dup2");}, errno);
            }
            
            for (size_t curCloseInd = 0; curCloseInd < 2 * (commandsNum - 1); curCloseInd++) {
            
                SET_ERRNO;
                int checkClose = close (fd [curCloseInd]);
                FUNCTION_PROTECT (checkClose < 0, { free (fd);
                                                    perror ("Bad pipe's close");}, errno);
            
            }
            
            SET_ERRNO;
            int execErr = execvp (commands [i].argv [0], commands [i].argv);
            FUNCTION_PROTECT (execErr < 0, {free (fd);
                                            printf ("Command %s bad exec :(\n", commands [i].argv [0]);}, errno);           

        }
    
    }

    int w8Status = 0;
    SET_ERRNO;
    pid_t checkW8 = wait (&w8Status);
    FUNCTION_PROTECT (checkW8 < 0, {free (fd);
                                    perror ("bad wait status");}, errno);

    free (fd);
    
    return NO_CONVEYER_ERR;

}

static Command* CommandsConstr (const size_t commandsNum) {

    Command* commandsArr = (Command*)calloc (commandsNum, sizeof (*commandsArr));
    FUNCTION_PROTECT (IS_NULL (commandsArr), {PRINT_CONVEYER_ERR (BAD_ALLOC);}, NULL);

    return commandsArr;

}

static void CommandsDstr (Command* commandsArr, const size_t commandsNum) {

    for (size_t curCommandToFree = 0; curCommandToFree < commandsNum; curCommandToFree++)
        free (commandsArr [curCommandToFree].argv);

    free (commandsArr);

}

static void SkipTrash (char** buffer) {

    while (isspace (**buffer))
        (*buffer)++;

}

static void PrintConveyerErr (const int err, const int line, const char* function, const char* file) {

#define PRINT_ERR(msg)  printf ("Error [%d]: %s. Was catched on line %d in function %s in file %s.\n",\
                                err, msg, line, function, file)

    switch (err) {
    
        case NO_CONVEYER_ERR:
            printf ("No any error!\n");
            return;
        case EMPTY_FILE:
            PRINT_ERR ("empty input file");
            return;
        case BAD_ALLOC:
            PRINT_ERR ("bad memory allocation");
            return;
        case BAD_INPUT_DESC:
            PRINT_ERR ("bad input descriptor");
            return;
        case NO_ANY_COMMAND:
            PRINT_ERR ("no any command");
            return;
        case BAD_EXEC_COMMAND:
            PRINT_ERR ("bad command execution");
            return;
        case BAD_PIPE:
            PRINT_ERR ("bad opening for pipes");
            return;
        case BAD_FORK_CALL:
            PRINT_ERR ("bad call for fork () function");
            return;
        default:
            PRINT_ERR ("unexpected error");

    }

}

