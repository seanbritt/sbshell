/*
 * sean britt, march 2022
 *
 * lite edition of a shell program to run in a unix terminal
 */

#include "shellheader.h"

int  myShell(char** builtins, char** redir, char** environ, char* shellargv);
struct tokptr* addtok(struct tokptr* current);
void printcoms(struct tokptr* current);
int runrecursive(struct tokptr *current, char** builtins, int builtcheck, char** commandline, char** environ);
int builtinchecker(struct tokptr *current, char** builtins);
int runbuiltin(struct tokptr *current, char** commandline, char** envp);


//this acts as a baby linked list node to keep track of the commands and their modes

struct tokptr{
    char *args[256];    //the first args is the command
    char* mode;
    int len;            //from args[1] to the last arg, args[0] is the command
    pid_t pid;
    struct tokptr* next;
};


int main(int argc, char **argv, char ** envp){


    char currentdirectory[PATH_MAX];
    getcwd(currentdirectory, sizeof(currentdirectory));
    strcat(currentdirectory, "/myshell");
    setenv("SHELL", currentdirectory, 1);
    //printf("SHELL: %s\n", getenv("SHELL"));

    char *builtins[] = {"cd", "dir", "environ", "echo", "help", "pause", "quit", NULL};
    char *redir[] = {"<", ">", ">>", "&", "|", NULL};

    //if there is 1 argument, it is just the myShell program
    if(argc == 1){
        //continuously run myShell() until the user calls exit
        fflush(stdout);
        printf("\n");
        while(myShell(builtins, redir, envp, NULL)){

        }
        printf("exiting\n");
        exit(0);
    }

    //if there is more than 1 argument, run the script
    else if (argc < 3){

        FILE * shellfile;
        char * shellpath;
        shellpath = strdup("./");
        strcat(shellpath, argv[1]);
        ssize_t linesread;
        size_t linelen;
        char* line = NULL;

        if ((shellfile = fopen(shellpath, "r")) != NULL){

            while ((linesread = getline(&line, &linelen, shellfile)) != -1){
                *(line + linesread - 1) = '\0';
                //printf("%s", line);
                myShell(builtins, redir, envp, line);
            }
        }else{
            printf("could not open file: %s", argv[1]);
        }


    }else{
        printf("invalid initiation of myshell: can only accept one argument in the form of a shell file\n");
    }
    exit(0);
}















int myShell(char** builtins, char** redir, char** environ, char* shellargv){

    //fflush(stdin);
    //fflush(stdout);
    //printf("current pid: %d\n", getpid());

    //initiate the baby linkedlist of tokens
    struct tokptr *first, *current;
    first = (struct tokptr*)malloc(sizeof(struct tokptr));

    //printcoms(first);

    //initialize the thing
    memset(first->args, 0, sizeof(first->args));
    first->args[0]=NULL;
    first->mode = NULL;
    first->len = 0;
    first->pid = 0;
    first->next = NULL;
    current = first;        //current will keep track of our place in the list, first will be the head

    //printcoms(first);

    int mylen = 0;      //the amount of commands
    int len = 0;        //temporary length of a particular command, its args, and the mode
    //char *commandline;  //represents the commandline to parse, and is passed to echo
    char *commandline = NULL;
    char* commandline_topass[256];  //this will be passed to echo, holding all the tokens as an **argv
    int tokens_len = 0;
    int in= 0;          //says we are in a command, 0 when we find a mode operator
    

    //allocate space for the commandline string

    //**************************************************************** loop to get commandline
    //loop to get the input from the keyboard
    if(shellargv == NULL){

        size_t len = 0;
        ssize_t linelen = 0;
        fflush(stdout);
        printf("myShell> ");
        while((linelen = getline(&commandline, &len, stdin)) != EOF){
            if (strstr(commandline, "\n")){
                *(commandline + linelen - 1) = '\0';
                break;
            }

            free(commandline);
        }

    }else{
        commandline = strdup(shellargv);
    }





    //return to keyboard if the first command is an enter or NULL
//    if(strlen(commandline) == 0 || strcmp(commandline, "\n") == 0){
    if(strlen(commandline) == 0){
        return 1;
    }



    //********************************************************** build the linked list of tokens



    //add all commandline items to the list of tokens
    char* temptoken;
    while( (temptoken = strsep(&commandline, " ")) != NULL){

        tokens_len++;
        //in begins at 0, and we only enter this while loop if the temptoken is not null, so we know we have a token
        if(in==0){
            current->mode = "";
            mylen++;        //the length of the token list
            len = 0;
        }

        in = 1;             //assume we are in a command
        len++;              //len and current->len are the amount of tokens in a command (command+args)
        current->len = len;

        commandline_topass[tokens_len-1] = temptoken;  //to be passed as a **argv to echo

        //check to make sure the token is not a redirection char*
        int j;
        for(j = 0; j<RED_LEN; j++){

            //if we find a mode operator or the end of the commands
            if(strcmp(redir[j], temptoken) == 0){
                //if we found a mode operator but len is 0, it's the first command and that's no good
                if(len == 1){
                    printf("`%s` is not a valid command/", temptoken);
                    return 0;
                }

                //we have found a mode operator
                else{
                    in = 0;       //we are done with this current command, moving to the next
                    break;
                }
            }
        }

        //if the token is not a redirection, add it to current->args[len-1]
        if(in){
            //add the token to where it needs to go
            current->args[len-1]=strdup(temptoken);
        }
        else{

            //we are out of the command, bookended with the mode char*
            current->mode = strdup(temptoken);

            //set up a new tokptr
            current->args[current->len] = NULL;
            //printf("final arg: %s\n", current->args[current->len]);
            struct tokptr *temp= (struct tokptr*)malloc(sizeof(struct tokptr));
            //memset(first->args, NULL, sizeof(first->args));
            temp->args[0] = NULL;
            temp->args[1] = NULL;
            temp->next = NULL;
            current->next = temp;
            current = current->next;

        }
        
    }

    current->args[current->len] = NULL;
    commandline_topass[tokens_len] = NULL;


    //**************************************************************** start running the commands
    //reset the current pointer to the first int the list
    current = first;


    //run the commands
    free(commandline);
    int returnsig;
    returnsig = runrecursive(current, builtins, 1, commandline_topass, environ);
    free(first);
//    current = first;
//    struct tokptr * temp = first;
//    while (current != NULL){
//        fflush(stdin);
//        //printf("clearing current: <%s>\n", current->args[0]);
//        current = current->next;
//        free(temp);
//        temp = current;
//    }

    printf("\n");
    return returnsig;

}











//recursively run each command in the
int runrecursive(struct tokptr *current, char** builtins, int builtcheck, char** commandline, char** environ) {


    //flag for how to change the timing/piping/redirection of the command
    char *com_mode = current->mode;

    if (strcmp(com_mode, "&") == 0) {          //parent does not wait

        if (builtinchecker(current, builtins) == 0) {         //it's an exec or nothing
            pid_t pid;
            if ((pid = fork()) == -1) {
                printf("error forking with &\n");
                exit(0);
            } else if (pid == 0) { //child
                execvp(current->args[0], current->args);
                printf("WHOOPS, not a valid command: <%s>\n", current->args[0]);
                _exit(1);

            } else {   //parent
                //run recursively if there is more to run, do not wait for the child to exit
                if (current->next != NULL) {
                    runrecursive(current->next, builtins, 0, NULL, NULL);
                }
            }
        } else {                                                      //it's a builtin

            if (builtcheck == POSSIBLE_BUILTIN) {
                if (current->next != NULL) {
                    runrecursive(current->next, builtins, 0, NULL, NULL);
                }
                runbuiltin(current, commandline, environ);
                //run recursively if there is more to run
            } else {
                printf("1 cannot use builtins after the first command <%s>\n", current->args[0]);
                //exit(0);
            }
        }


    } else if (strcmp(com_mode, "|") == 0) {   //set up pipe


        //printf("pipe\n");
        int fd[2], status;
        pid_t pid = -1, pid2 = -1;


        if (pipe(fd) == -1) {
            printf("error piping\n");
            exit(1);
        }

        if ((pid = fork()) == -1) {
            printf("error forking\n");
            exit(1);
        } else if (pid == 0) {                            //write

            //check for builtin
            if (builtinchecker(current, builtins)) {
                if (builtcheck == POSSIBLE_BUILTIN) {
                    runbuiltin(current, commandline, environ);
                } else {
                    printf("1 cannot use builtins after the first command <%s>\n", current->args[0]);
                    return 1;
                }
            }

            printf("write child: <%s>", current->args[0]);
            close(fd[0]);
            dup2(fd[1], 1);
            close(fd[1]);

            if (execvp(current->args[0], current->args) < 0) {

                printf("woops 287");
                _exit(1);
            }

        } else {

            if ((pid2 = fork()) == -1) {
                printf("error forking\n");
                exit(1);
            }

            if (pid2 == 0) {                        //read


                printf("read child: <%s>\n", current->next->args[0]);
                close(fd[1]);
                dup2(fd[0], 0);
                close(fd[0]);
                if (execvp(current->next->args[0], current->next->args)) {
                    printf("woops 310");
                    _exit(1);
                }
            }
            current->pid = pid;
            current->next->pid = pid2;
//            printf("pid: %d\n", pid);
//            printf("pid2: %d\n", pid2);
//            printf("current pid: %d\n", current->pid);
//            printf("next pid: %d\n", current->next->pid);

        }
        waitpid(pid, &status, 0);

        close(fd[0]);
        close(fd[1]);


    } else if (strcmp(com_mode, ">") == 0) {  //current outputs to a file, write

        //        int fd[2];
//        if(pipe(fd) == -1){
//            printf("failed to pipe\n");
//            _exit(1);
//        }
        pid_t pid;

        if ((pid = fork()) == -1) {
            printf("error forking with &\n");
            exit(0);
        } else if (pid == 0) { //child1, the write end

            char* outpath;
            outpath = strdup("./");
            strcat(outpath, current->next->args[0]);

            int out = open(outpath, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU | S_IRWXG| S_IRWXO);

            dup2(out, 1);
            close(out);

            if (builtinchecker(current, builtins) == 1) {          //run the builtin
                if (builtcheck == POSSIBLE_BUILTIN) {
                    runbuiltin(current, commandline, environ);
                    _exit(1);
                } else {
                    printf("1 cannot use builtins after the first command <%s>\n", current->args[0]);
                    _exit(1);
                }
            } else {

                execvp(current->args[0], current->args);
                printf("WHOOPS");
                _exit(1);

            }


        } else {

            waitpid(pid, NULL, 0);

        }



    }else if (strcmp(com_mode, ">>") == 0) { //current outputs to a file, append

        pid_t pid;

        if ((pid = fork()) == -1) {
            printf("error forking with &\n");
            exit(0);
        } else if (pid == 0) { //child1, the write end

            char* outpath;
            outpath = strdup("./");
            strcat(outpath, current->next->args[0]);

            int out = open(outpath, O_WRONLY|O_APPEND, S_IRWXU | S_IRWXG| S_IRWXO);

            dup2(out, 1);
            close(out);

            if (builtinchecker(current, builtins) == 1) {          //run the builtin
                if (builtcheck == POSSIBLE_BUILTIN) {
                    runbuiltin(current, commandline, environ);
                    _exit(1);
                } else {
                    printf("1 cannot use builtins after the first command <%s>\n", current->args[0]);
                    _exit(1);
                }
            } else {
                execvp(current->args[0], current->args);
                printf("WHOOPS");
                _exit(1);
            }


        } else {

            waitpid(pid, NULL, 0);

        }
    } else if (strcmp(com_mode, "<") == 0) {  //current is a filename to read
        pid_t pid;

        if ((pid = fork()) == -1) {
            printf("error forking with &\n");
            exit(0);
        } else if (pid == 0) { //child1, the write end

            char* inpath;
            inpath = strdup("./");
            strcat(inpath, current->args[0]);

            int in = open(inpath, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU, S_IRWXU | S_IRWXG| S_IRWXO);

            dup2(in, 1);
            close(in);

            if (*(current->next->args[0]) != '\0'){
                if (builtinchecker(current->next, builtins) == 1) {          //run the builtin

                    printf("1 cannot use builtins after the first command <%s>\n", current->args[0]);
                    _exit(1);

                } else {
                    execvp(current->next->args[0], current->next->args);
                    printf("WHOOPS");
                    _exit(1);
                }
            }else{
                printf("redirection with \"<\" must be followed by a command\n");
                _exit(1);
            }

            _exit(1);


        } else {

            waitpid(pid, NULL, 0);

        }
    } else if (com_mode == NULL) {
        printf("fwoops\n");
    } else {                                  //the mode operator is null so this should be the end of the commands

        if (builtinchecker(current, builtins) == 1) {         //if it is a builtin and can be run
            if (builtcheck == POSSIBLE_BUILTIN) {
                (runbuiltin(current, commandline, environ));

            } else {
                printf("cannot use builtins after the first command <%s>\n", current->args[0]);
                //exit(1);
            }

        } else if (strlen(com_mode) == 0 || *com_mode == '\0') {  //it's an exec without a redirection mode

            pid_t pid;
            int status = 0;
            if ((pid = fork()) == -1) {
                printf("error with &\n");
                exit(0);
            } else if (pid == 0) {
                //printf("current pid: [0]: %d\n", getpid());
                //printf("tries to exec : %s\n", current->args[0]);
                //printf("\t%s\n\t%d\n", current->args[1], current->len);

                execvp(current->args[0], current->args);

                printf("<%s> not a valid command\n", current->args[0]);
                fprintf(stdout, "error: %d", errno);
                _exit(0);


            } else {

                waitpid(0, &status, 0);

            }

        }
    }
    return 1;
}



//should only be called for the first command
int runbuiltin(struct tokptr *current, char** commandline, char** envp){

    char* com = current->args[0];
    if(strcmp(com, "cd") == 0){
        cd(current->len, current->args);
    }else if(strcmp(com, "clr")==0){
        clr();
    }else if(strcmp(com, "dir") == 0){
        dir(current->len, current->args);
    }else if(strcmp(com, "environ") == 0){
        environ(envp);
    }else if(strcmp(com, "echo") == 0){
        echo(commandline);
    }else if(strcmp(com, "help") == 0){
        help();
    }else if(strcmp(com, "pause") == 0){
        _pause();
    }else if(strcmp(com, "quit") == 0){
        quit();
    }else{
        return 0;
    }
    return 1;
}


int builtinchecker(struct tokptr *current, char**builtins){
    //printf("checker start: %s\n", current->args[0]);
    char** temp = builtins;
    char* tocheck = current->args[0];
    //printf("%s, %s\n", tocheck, *temp);
    while(*temp != NULL) {
        //printf("\t%s\n", *temp);
        if (strcmp(tocheck, *temp) == 0){  //return 1 if this is a builtin
            //printf("\t%s\n", *temp);
        return 1;
        }
        temp++;
    }
    //printf("checker end: %s\n", current->args[0]);
    return 0;
}




void printcoms(struct tokptr *current){
    int com = 1;
    while(current != NULL){
        int i;
        printf("command %d: %s\n", com, current->args[0]);
        printf("\targs: %s", current->args[1]);
        for(i =2; i < current->len; i++ ){
            printf(" ,%s", current->args[i]);
        }
        printf("\n\tmode: %s", current->mode);
        printf("\n");
        current = current->next;
        com++;
    }
}

