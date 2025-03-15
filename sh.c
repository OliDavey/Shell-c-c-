#include "sys/types.h"
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

/*
shell program using c
*/

#define EXEC 1
#define REDIR 2
#define PIPE 3
#define LIST 4
#define BACK 5

#define MAXARGS 10 

struct cmd
{
    /* data */
    int type;
};

struct execcmd
{
    int type;
    char *argv[MAXARGS]; // fix sized arry (size 10)
    char *eargv[MAXARGS];// end of arg
};

struct redircmd
{
    int type;
    struct cmd *cmd;
    char *file;
    char *efile; // of file
    int mode;
    int fd;
};

struct pipecmd
{
    int type;
    struct cmd *left;
    struct cmd *right;   
};

struct listcmd
{
    int type;
    struct cmd *left;
    struct cmd *right;   
};

struct backcmd
{
    int type;
    struct cmd *cmd;
};

// constructor functions 

struct cmd* execcmd(void)
{
    struct execcmd *cmd;
    cmd = malloc(sizeof(*cmd)); 
    memset(cmd, 0, sizeof(*cmd));
    cmd-> type = EXEC;
    return (struct cmd*) cmd;
};

struct cmd* redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
{
    struct redircmd *cmd;
    cmd = malloc(sizeof(*cmd)); 
    memset(cmd, 0, sizeof(*cmd));
    cmd-> type = REDIR;
    cmd-> cmd = subcmd;
    cmd-> file = file;
    cmd-> efile = efile;
    cmd-> mode = mode;
    cmd-> fd = fd;
    return (struct cmd*) cmd;
};

struct cmd* pipecmd(struct cmd *left, struct cmd *right)
{
    struct pipecmd *cmd; // using local var cmd, which is a pointer to a pipe node

    cmd = malloc(sizeof(*cmd)); // allocating node
    memset(cmd, 0, sizeof(*cmd)); // sets it all to 0
    cmd -> type = PIPE;
    cmd -> left = left;
    cmd -> right = right;
    return (struct cmd*) cmd; // return pointer to the node (typecasted)
};

struct cmd* listcmd(struct cmd *left, struct cmd *right)
{
    struct listcmd *cmd; // using local var cmd, which is a pointer to a pipe node

    cmd = malloc(sizeof(*cmd)); // allocating node
    memset(cmd, 0, sizeof(*cmd)); // sets it all to 0
    cmd -> type = LIST;
    cmd -> left = left;
    cmd -> right = right;
    return (struct cmd*) cmd; // return pointer to the node (typecasted)
};

struct cmd* backcmd(struct cmd *subcmd)
{
    struct backcmd *cmd; // using local var cmd, which is a pointer to a pipe node

    cmd = malloc(sizeof(*cmd)); // allocating node
    memset(cmd, 0, sizeof(*cmd)); // sets it all to 0
    cmd -> type = LIST;
    cmd -> cmd = subcmd;
    return (struct cmd*) cmd; // return pointer to the node (typecasted)
};


//helper funcs
void panic(char *s){
    fprintf(2, "%s\n", s); // prints error msg on the std error given that std error will be file descriptor num 2
    exit(1); // system call exit, terminates program (1 is error, 0 is a success)
}

int fork1(void){
    int pid; // process ID
    pid = fork();
    if (pid == -1){
        panic("fork"); // terminates process if fork had any error
    }
    return pid;
}

// functions that do things and run cmds
int fork1(void);
void panic(char*);
struct cmd *parsecmd(char*);

void runcmd(struct cmd *cmd){ // dosnt return jsut finishes by terminating a process
    int p[2];
    struct backcmd *bcmd;
    struct execcmd *ecmd;
    struct listcmd *lcmd;
    struct pipecmd *pcmd;
    struct redircmd *rcmd;

    if (cmd == 0){
        exit(1);
    }

    switch (cmd -> type)
    {
    default:
        panic("runcmd");

    case EXEC:
        ecmd = (struct execcmd*)cmd;
        // checking to see there is at least one argument
        if (ecmd->argv[0] == 0){ // argv[0] points to null terminated string of file name of program to execute
            exit(1);
        }
        exec(ecmd->argv[0], ecmd->argv); // sys call exec(filename of program to be run, pointer to array)
        fprintf(2, "exec %s failed\n", ecmd->argv[0]);
        break;

    case REDIR:
        rcmd = (struct redircmd*)cmd;
        close(rcmd->fd);
        if (open(rcmd->file, rcmd->mode) < 0){
            fprintf(2, "open %s failed\n", rcmd->file);
            exit(1);
        }
        runcmd(rcmd->cmd);
        break;

    case PIPE:
        pcmd = (struct pipecmd*)cmd;
        if (pipe(p) < 0){ // system call pipe
            panic("pipe");
        }
        // redirect pgm1 output to pgm1 input
        if (fork1() == 0){ // creates first child process
            close(1); //close std out
            dup(p[1]); // creates copy of fd
            close(p[0]); // close fd #
            close(p[1]); // close next fd
            runcmd(pcmd->left);
        }
        if (fork1() == 0){ // creates second child process
            close(0); // close std in
            dup(p[0]); // creats copy of file descriptor fd
            close(p[0]);
            close(p[1]);
            runcmd(pcmd->right);
        }
        close(p[0]); // close fd 1
        close(p[1]); // close fd 2
        wait(0); // wait for first child to terminate
        wait(0); // waits for other child to terminate
        break;

    case LIST:
        lcmd = (struct listcmd*)cmd;
        if (fork1() == 0){
            runcmd(lcmd -> left);
        }
        wait(0);
        runcmd(lcmd->right);
        break;

    case BACK:
        bcmd = (struct backcmd*)cmd;
        if (fork1() == 0){
            runcmd(bcmd->cmd);
        }
        break;
    }
    exit(0);
}


int main(void){
    static char buf[100]; // num of chars the buffer will allow (how big of a cmd we can pass)
    int fd; // file descriptor 

    while ((fd = open("console", O_RDWR)) >= 0) // open for reading and writing
    {
        if (fd >= 3){
            close(fd);
            break;
        }
    }

    // read then run cmd
    while (getcmd(buf, sizeof(buf)) >= 0)
    {
        if (buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ') // checking for cd dir
        {
            buf[strlen(buf)-1] = 0; // buf[length of string -1] set to 0
            if (chdir(buf+3) < 3){ // inokes chdir sys call and points to file name
                fprintf(2, "Cannot cd %s\n", buf+3);
                continue; // repeat while loop so it makes a permant change to the process
            }
        }
        // if not cd cmd 
        if (fork1() == 0){ // fork returns 0 for child process and PID for parent process
            runcmd(parsecmd(buf)); // parent forks child and runs runcmd. runcmd terminates itself and dosnt return any value
        }
        wait(0); // parent process immediatly begins waiting for child process to finish
    }
    exit(0);
}

