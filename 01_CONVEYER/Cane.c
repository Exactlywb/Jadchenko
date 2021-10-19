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
    
    size_t commandsNum = GetCommandsNum (buffer);

    Command* commandsArr = CommandsConstr (commandsNum);
    FUNCTION_PROTECT (IS_NULL (commandsArr), {  free (buffer);
                                                PRINT_CONVEYER_ERR (BAD_ALLOC);}, BAD_ALLOC);
    
    int sepErr = SeparateCommands (buffer, commandsArr);
    FUNCTION_PROTECT (sepErr != NO_CONVEYER_ERR, {  CommandsDstr (commandsArr, commandsNum);
                                                    free (buffer);
                                                    PRINT_CONVEYER_ERR (sepErr);}, sepErr);
    
    int execErr = ExecCommands (commandsArr, commandsNum);
    FUNCTION_PROTECT (execErr != NO_CONVEYER_ERR, { CommandsDstr (commandsArr, commandsNum);
                                                    free (buffer);
                                                    PRINT_CONVEYER_ERR (execErr);}, execErr);
    
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

    size_t curArgvInd = 0;
    argv [curArgvInd++] = buffer;
    
    char* nextWord = NULL;
    while ((nextWord = strchr (buffer, ' ')) && curArgvInd < argc) {
    
        char* toReplace = nextWord;
        SkipTrash (&nextWord);
        *toReplace = 0;

        argv [curArgvInd++] = nextWord;

        buffer = nextWord;
    
    }

    for (size_t curArg = 0; curArg < argc; curArg++) { //to set '\0' at the end of string
    
        char* curArgStr = argv [curArg];

        size_t curSymb = 0;
        for (; !isspace (curArgStr [curSymb]) && curArgStr [curSymb] != 0; curSymb++) {}
    
        curArgStr [curSymb] = 0;

    }

    argv [argc] = NULL;

    return argv;

}

static int ExecCommands (const Command* commands, size_t commandsNum) {

    int oldOut = STDIN_FILENO;
    size_t curCommandInd = 0;
    while (commandsNum != 0) {
        
        int ioFiles [2] = {STDIN_FILENO, STDOUT_FILENO};
       
        pipe (ioFiles);

        pid_t curPid = fork ();
        FUNCTION_PROTECT (curPid == -1, {perror ("bad fork");}, BAD_FORK_CALL); 

        if (curPid == 0) {        
            //in child

            if (curCommandInd != 0)
                dup2 (oldOut, STDIN_FILENO);

            if ((commandsNum - 1) != 0)
                dup2 (ioFiles [1], STDOUT_FILENO);
             
            int execErr = execvp (commands [curCommandInd].argv [0], commands [curCommandInd].argv);
            FUNCTION_PROTECT (execErr == -1, {perror ("bad exec");}, BAD_EXEC_COMMAND);

        } else {
            //in parent

            int waitStatus = 0;
            int waitErr = wait (&waitStatus);

            close (ioFiles [1]);
            close (oldOut);
            oldOut = ioFiles [0];

            commandsNum--;
            curCommandInd++;
        
        }

    }

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
        case BAD_FORK_CALL:
            PRINT_ERR ("bad call for fork () function");
            return;
        default:
            PRINT_ERR ("unexpected error");

    }

}

