#include "smallsh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Built in commands
/*
Your shell will support three built-in commands: exit, cd, and status.
 These three built-in commands are the only ones that your shell will handle itself 
 - all others are simply passed on to a member of the exec() family of functions.

You do not have to support input/output redirection for these built in commands
These commands do not have to set any exit status.
If the user tries to run one of these built-in commands in the background with the & option,
 ignore that option and run the command in the foreground anyway 
 (i.e. don't display an error, just run the command in the foreground).

exit
The exit command exits your shell. It takes no arguments.
When this command is run, your shell must kill any other processes or jobs that your shell has started before it terminates itself.

cd
The cd command changes the working directory of smallsh.

By itself - with no arguments - it changes to the directory specified in the HOME environment variable
This is typically not the location where smallsh was executed from, unless your shell executable is located in the HOME directory, in which case these are the same.
This command can also take one argument: the path of a directory to change to. Your cd command must support both absolute and relative paths.
status
The status command prints out either the exit status or the terminating signal of the last foreground process ran by your shell.

If this command is run before any foreground command is run, then it must simply return the exit status 0.
The three built-in shell commands do not count as foreground processes for the purposes of this built-in command - i.e., status must ignore built-in commands.





*/

void smallsh_exit(){
    /*
    cases to handle
    1. exit command is run
        a. kill all child processes
        b. exit the shell

    */

    //kill all child processes
    //exit the shell
    exit(0); //do i need more than this?
}


//changes the current working directory
void smallsh_cd(struct command_line *curr_command){
    /*
    cases to handle

    1. no arguments
        navigate to home directory

    2. one argument
        a. absolute path
        b. relative path

    */

    if(curr_command->argc == 1){
         
    }

}
//prints the status of the last foreground process
void smallsh_status(struct command_line *curr_command){
    /*
    cases to handle
    1. before any foreground process are run
        returns exit statue 0
        exit, cd and status dont count as foreground processes
    2. after a foreground process is run
        returns the exit status or terminating signal of the last foreground process

    */

    

}