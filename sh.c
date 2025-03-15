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
        printf(2, "exec %s failed\n", ecmd->argv[0]);
        break;

    case REDIR:
        rcmd = (struct redircmd*)cmd;
        close(rcmd->fd);
        if (open(rcmd->file, rcmd->mode) < 0){
            printf(2, "open %s failed\n", rcmd->file);
            exit(1);
        }
        runcmd(rcmd->cmd);
        break;

    case PIPE:
        pcmd = (struct pipecmd*)cmd;
        if (pipe(p) < 0){ // system call pipe
            panic("pipe");
        }
        // redirect pgm1 output to pgm2 input (pgm1 | pgm2)
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


int getcmd(char *buf, int nbuf){
    printf(2, "$ ");
    memset(buf, 0, nbuf);
    gets(buf);
    gets(nbuf);
    if (buf[0] == 0){
        return -1;
    }
    return 0;
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
                printf(2, "Cannot cd %s\n", buf+3);
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

//helper funcs
void panic(char *s){
    printf(2, "%s\n", s); // prints error msg on the std error given that std error will be file descriptor num 2
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

// parsing stuff

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()"; // tokens

// scans the buffer chars and gets the next token
/*
arguments: 
    s points to (" ") the spot to begin the buffer scan, ps points to s
    es points to ("\o") where the scan should end
    q and eq are pointers to where the token is found and the byte after the token
    q and eq can be null values
returns int value describing token type:
    |, ;, &, ), (, <, >, >> (+), other (a), (end) (0 / null byte)

*/ 
int gettoken(char **ps, char *es, char **q, char **eq){
    char *s;
    int ret; // return value

    s = *ps;
    while (s < es && strchr(whitespace, *s)) //advance s to first not whitespace
    { // make sure where not at the end yet
        s++;
    }
    if (q){
        *q = s;
    }
    ret = *s;
    switch (*s)
    {
    case 0: // null byte / end of scan 
        break;
    case '|': // just a single character token, point to after token
    case '(':
    case ')':
    case ';':
    case '&':
    case '<':
        s++;
        break;
    case '>':
        s++; // if 1 > advanced to next symbol
        if (*s == '>'){ // if the next symbol is > then return +
            ret = '+';
            s++; // advance pointer again
        }
        break;
    default:
        ret = 'a';
        while (s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
        {
            s++;
        }
    
        break;
    }
    if (eq){
        *eq = s;
    }
    // advances to null pointer at string end, or something thats not whitespace
    while (s < es && strchr(whitespace, *s))
    {
        s++;
    }
    *ps = s;
    return ret;
}

// look past any whitespace char to find the next non white space character
int peek(char **ps, char *es, char*toks){
    char *s;

    s = *ps;
    while(s < es && strchr(whitespace, *s)){
        s++;
    }
    *ps = s;
    return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);
struct cmd *nulterminate(struct cmd*);


// returns pointer to tree that is built
struct cmd* parsecmd(char *s){ // pass pointer to input buffer
    char *es;
    struct cmd *cmd;

    es = s  + strlen(s); // figures out where null char is, es points to it
    cmd = parseline(&s, es);// does work, 
    peek(&s, es, "");// advance past any white space and rest at null pointer
    // make sure nothing is left
    if (s != es){ // stopped on something besides the null point (end of string)
        printf(2, "leftovers: %s\n", s);
        panic("syntax"); // terminates process, something is wrong
    }
    nulterminate(cmd);
    return cmd;
}
// example inputs:
// pgm opt1 opt2 <file1 >file2
// you can put the redirections anywhere such as:
// <file1 pgm opt1 >file opt2, this creates the same tree
struct cmd* parseexec(char **ps, char *es){
    char *q, *eq;
    int tok, argc;
    struct execcmd *cmd;
    struct cmd *ret;

    if(peek(ps, es, "(")){
        return parseblock(ps, es);
    }
    ret = execcmd(); // allocates node without filling it in
    cmd = (struct execcmd*)ret; // pointer to exec node

    argc = 0;
    ret = parseredirs(ret, ps, es); // pass it the tree built so far (empty exec node)
    // check to see if something thats not a punctuation char after the redir
    while(!peek(ps, es, "|)&;")){ // can pass 0 or more filenames, if 0 then exec starts with null pointer
        if ((tok=gettoken(ps, es, &q, &eq)) == 0){ // check to see if at the end of the string
            break;
        }
        if (tok != 'a'){ // check we got a filename or option or not 0
            panic("syntax");
        }
        cmd->argv[argc] = q; // fill in argv pointers, q points to pgm 
        cmd->eargv[argc] = eq; // fill in eargv pointers eq points to end of pgm
        argc++;
        if (argc >= MAXARGS){ // check for buffer oveflow, no room for null at end of string
            panic("too many args");
        }
        // passed pointer to some tree, we add a root then return pointer to the root
        // we do the last redir first, then the next until we get to exec
        ret = parseredirs(ret, ps, es); // loop till we see one of the while loop symbols
    }
    cmd->argv[argc] = 0;
    cmd->eargv[argc] = 0;
    return ret;
}

// gonna add a root and replace it
struct cmd* parseredirs(struct cmd *cmd, char **ps, char *es){ // pointer to sub tree were going to use
    int tok;
    char *q, *eq;

    while(peek(ps, es, "<>")){
        tok = gettoken(ps, es, 0, 0); // saves the characters, either <> or + (>>)
        // if there isnt an 'a' then there isnt a file to redirect to
        // passing addresses of q and eq. pointers to first byte of file name and end of file name + 1
        if (gettoken(ps, es, &q, &eq) != 'a'){
            panic("missing file for redirection");
        }
        switch (tok)
        {
        case '<':
            cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
            break;
        case '>':
            cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREAT, 1);
            break;
        case '+':
            cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREAT, 1);
            break;
        }
    }
    return cmd;
}

struct cmd* parseblock(char **ps, char *es){
    struct cmd *cmd;
    // if no block to parse in the buffer, panic cause were trying to anyway
    if (!peek(ps, es, "(")){ 
        panic("parseblock");
    }
    gettoken(ps, es, 0, 0);
    cmd = parseline(ps, es); // return pointer to tree constructed placed in cmd
    if (!peek(ps, es, ")")){ // check for missing close to block
        panic("syntax - missing )");
    }
    gettoken(ps, es, 0, 0); // scan the parenthese if it exists
    // pass pointer to tree thats been built so far, along with ps and es
    // possibly adding a new tree route
    cmd = parseredirs(cmd, ps, es);
    return cmd; // return pointer to resulting tree
}

struct cmd* parseline(char **ps, char *es){
    struct cmd *cmd;

    cmd = parsepipe(ps, es); // returns a tree saved in cmd
    while (peek(ps, es, "&")) // check if next thing is &
    {
        gettoken(ps, es, 0, 0); // pick it up
        //add new root node / background node to tree using the prevoius tree as the subtree
        cmd = backcmd(cmd);
    }

    if(peek(ps, es, ";")){ // peek at next token see if its ;
        gettoken(ps, es, 0, 0);// scan it
        // parseline returns subtree, we make a list node with pgm1 or whatever on left and subtree on right
        cmd = listcmd(cmd, parseline(ps, es)); 
    }
    return cmd; //return the node
}

// *es points to end of string / null byte
// **ps, tells where in the buffer to start the scan 
struct cmd* parsepipe(char **ps, char *es){ //pointers tell us where we should look at the input buffer
    struct cmd *cmd;

    cmd = parseexec(ps, es); // builds a tree, we save a pointer to it in cmd
    if (peek(ps, es, "|")){ // if thing is bar scan it
        gettoken(ps, es, 0, 0); 
        // whatever exec returns is left subtree, whatever the recursive call returns is used in right subtree 
        cmd = pipecmd(cmd, parsepipe(ps, es)); // new pipe node made
    }
    return cmd; // return the pipe node
}

struct cmd* nulterminate(struct cmd *cmd) // recursively walks the tree
{
    int i;
    struct backcmd *bcmd;
    struct execcmd *ecmd;
    struct listcmd *lcmd;
    struct pipecmd *pcmd;
    struct redircmd *rcmd;

    if (cmd == 0){
        return 0;
    }

    switch (cmd->type)
    {
    case EXEC:
        ecmd = (struct execcmd*)cmd;
        // go through argv array in order  and follow the pointer
        for (i =0; ecmd->argv[i]; i++){
            // write 0 at eargv
            *ecmd->eargv[i] =0;
        }
        break;
    case REDIR:
        rcmd = (struct redircmd*)cmd;
        nulterminate(rcmd->cmd);
        *rcmd->efile = 0;
        break;
    case PIPE:
        pcmd = (struct pipecmd*)cmd;
        nulterminate(pcmd->left);
        nulterminate(pcmd->right);
        break;
    case LIST:
        lcmd = (struct listcmd*)cmd;
        nulterminate(lcmd->left);
        nulterminate(lcmd->right);
        break;
    case BACK:
        bcmd = (struct backcmd*)cmd;
        nulterminate(bcmd->cmd);
        break;
    }
    return cmd;
}