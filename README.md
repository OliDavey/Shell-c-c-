# Shell (c/c++)
    Building a shell using c or c++. Based on Xv6 MIT shell software lectures for practice and understanding
    

# Shell program written in C has the following features:
        Options e.g -r -i
        I/O redirection e.g < or > or >> 
        Pipes | (use output of a program as input into another)
        Sequencing ; e.g pgm1 ; pgm2 will run program 2 after pgm1 completes
        Background & e.g pgm1 &, will run pgm1 in the background
        Parentheses for grouping e.g (pgm1 ; pgm2) | pgm3
        Built in cmd: cd pathname

    # System Calls:
        Process control: fork, exec, exit, wait
        I/O: open, close, read, write
        Pipes: pipe, dup
        change Dir: chdir

    Parse cmd line input and build tree structure reperensentation on first pass
    Walk the tree on a second pass

    use parsecmd - recursive descent parser
    runcmd - recursivley walks the cmd tree and executes each node in order (creats child processes as neccasary(fork))

    # Tree:
        Node attributes: type (e.g), left (pointer to val to left of node), right (pointer to val to right of node)
        types: EXEC(1), REDIR (2), PIPE(3), LIST(4), BACK(5). refer to type codes instead of names. Define them with #define TYPE val
        for & nodes: Type, cmd (pointer to whatever needs to be executed (an exec node))

    # Structs:
        pipecmd
        listcmd
        backcmd
        execcmd
        redircmd
        for each struct exists a constructor function with the same name (passed in a few arguments then allocates node, fill in node fields, allocate node pointer)

    # Functions:
        parsecmd (returns pointer to tree it builds)
            parseline
            parsepipe
            parseredirs
            parseblock
            parseexec
        nulterminate (recursivley walks tree and replaces all the spaces, so the all of the strings are null terminated)
        gettoken (read a token from the input buffer (identify where the next token occurs))

        main 
        getcmd (helper func that prints the prompt, read stdin --> buf (reads in from stdin to the buffer))
        runcmd (walks the tree and executes nodes)

        helper funcs: 
            panic (prints error msg and terminates process if error occurs (1 process is created for every cmdline thats entered) the shell itself keeps existing as a parent process)
            fork1 (wrapper func for the fork sys call, checks for any errors when fork sys call is envoked)

    Parsing functions:
        parseblock, reads in left parenthese, recursivley call in func it passes, passess right parenthese, calls function to parse redirections (parseredirs): (Line) REDIR

        parsepipe, calls parseexec, possibly parse a vertical bar |, then call parsepipe recursivly (calls itself): EXEC [| PIPE]

        parseline, call parsepipe, then parse 0 or more & chars, then optionally parse a line if a ';' char exists then call parseline: PIPE {&} [; LINE]

        parseridirs, a redirection is optionally '>', '<' or '>>' followed by a file name to be redirected to, there can be zero or more redirects: {either '>', '<' or '>>' filename}

        parseexec, normally a sequence of tokens, the first being a program name followed by additional tokens/options (you need one or more tokens the first being the filename): REDIR {filename REDIR}+ ( BLOCK, use the peek function to see if left parenthese exists, if so call block

        For parsepipe, we follow the right recursion rule / right associactivity. PIPE -> EXEC, then EXEC | PIPE. We build a right recursive tree. we end up with something like this: 
        aaa | (bbb | (ccc | ddd))




