#include "sys/types.h"
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


int main(){
    printf("hello world");
    return 0;
}