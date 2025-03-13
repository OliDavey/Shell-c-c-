#include "sys/types.h"
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

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


int main(void){
    static char buf[100]; // num of chars the buffer will allow (how big of a cmd we can pass)
    int fd;

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
            if (chdir(buf+3) < 3){
                fprintf(2, "Cannot cd %s\n", buf+3);
                continue;
            }
        }
        if (fork1() == 0){
            runcmd(parsecmd(buf));
            wait(0);
        }
    }
    exit(0);
}

//helper funcs
void panic(char *s){
    fprintf(2, "%s\n", s); // prints error msg on the std error given that std error will be file descriptor num 2
    exit(1); // system call exit, terminates program (1 is error, 0 is a success)
}

int fork1(void){
    int pid;
    pid = fork();
    if (pid == -1){
        panic("fork"); // terminates process if fork had any error
    }
    return pid;
}