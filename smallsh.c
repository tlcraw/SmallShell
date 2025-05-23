/*
Taylor Crawford
CS 374 Spring 2025
Assignment 4L Small Shell
Created: 05/08/2026
Modified: 05/17/2025

In this assignment you will write your own shell in C called smallsh. smallsh will implement a subset of features of well-known shells, such as bash. Your program will

    Provide a prompt for running commands
    Handle blank lines and comments, which are lines beginning with the # character
    Execute 3 commands exit, cd, and status via code built into the shell
    Execute other commands by creating new processes using a function from the exec() family of functions
    Support input and output redirection
    Support running commands in foreground and background processes
    Implement custom handlers for 2 signals, SIGINT and SIGTSTP

*/

/**
* A sample program for parsing a command line. If you find it useful,
* feel free to adapt this code for Assignment 4.
* Do fix memory leaks and any additional issues you find.


This code has been taken and modified from the provided sample_parser.c file provided in the programming assignment 4: SMALLSH
*/

#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>



#include <stdbool.h>
#define INPUT_LENGTH 2048
#define MAX_ARGS 512

static bool fg_mode = false; // foreground mode, & will be unnecessary and will be ignored. Global variable used so the signal handlers can change them

// Struct copied from sample_parser.c givien in assignment 4
struct command_line{
    char *argv[MAX_ARGS + 1];
    int argc;
    char *input_file;
    char *output_file;
    bool is_bg;
};

// Parsing command line input
// This function will parse the input and return a command_line struct.
struct command_line *parse_input(){
    char input[INPUT_LENGTH];
    struct command_line *curr_command = (struct command_line *) calloc(1,
    sizeof(struct command_line));

    // Get input
    // will need to add signal catchers for SIGINT and SIGTSTP
    printf(": ");
    fflush(stdout);
    fgets(input, INPUT_LENGTH, stdin);

    // Tokenize the input
    char *token = strtok(input, " \n");
    while(token){
        if(!strcmp(token,"<")){
           curr_command->input_file = strdup(strtok(NULL," \n"));
        } else if(!strcmp(token,">")){
            curr_command->output_file = strdup(strtok(NULL," \n"));
        } else if(!strcmp(token,"&")){
            curr_command->is_bg = true;
        } else{
            curr_command->argv[curr_command->argc++] = strdup(token);
        }

        token=strtok(NULL," \n");
    }
    curr_command->argv[curr_command->argc] = NULL; // Null terminate the argv array
    return curr_command;
}

// Free the command line struct
void free_command(struct command_line *cmd) {
    for (int i = 0; i < cmd->argc; i++) {
        free(cmd->argv[i]);
    }

    free(cmd->input_file);
    free(cmd->output_file);
    free(cmd);
}


// Signal handlers----------------------------------------------------------------------------------------------------
//_.~-^*:$ need to implement and insert singal handlers $:*^-~._
//---------------------------------------------------------------------------------------------------------------------


void handle_SIGINT(int signo) {
    // Handle SIGINT signal
    // will kill a child foreground process
    _exit(0);
 
}

void handle_SIGTSTP(int signo) {
    // Handle SIGTSTP signal
    // will toggle the parent precess to and from foreground mode, & will be unnecessary and will be ignored

    if (fg_mode == false){
        // set fg_mode to true
        fg_mode = true;
        fprintf(stdout, "\nEntering foreground-only mode (& is now ignored)\n");
        fflush(stdout);
    } else if (fg_mode == true){
        // set fg_mode to false
        fprintf(stdout, "\nExiting foreground-only mode\n");
        fflush(stdout);
        fg_mode = false;
    }
}
//--------------------------------------------------------------------------------------------------------------------


//Built in commands---------------------------------------------------------------------------------------------------

// exit command
void smallsh_exit(struct command_line *curr_command, int *bg_pids, int *bg_count) {
    /*
    cases to handle
    1. exit command is run
        a. kill all child processes
        b. exit the shell

    */
    //kill all child processes
    //exit the shell
    free_command(curr_command);
    for (int i = 0; i < *bg_count; i++){
        kill(bg_pids[i], SIGTERM); // send SIGTERM to all background processes
    }
    *bg_count = 0; // reset background process count
    free(bg_pids);
    exit(0);
}

//cd command
void smallsh_cd(struct command_line *curr_command){
    /*
    cases to handle

    1. no arguments
        navigate to home directory

    2. one argument
        a. absolute path
        b. relative path

    */

    if (curr_command->argc == 1) {
        // No arguments, go to home directory
        if (chdir(getenv("HOME")) == -1) {
            perror("chdir() failed");
        }
    } else {
        // Attempt to change directory, chdir should handle relative vs absolute paths by itself
        if (chdir(curr_command->argv[1]) == -1) {
            perror("chdir() failed");
        }
    } 
    // Update PWD to reflect the new working directory
    char cwd[INPUT_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        setenv("PWD", cwd, 1);
    } else {
        perror("getcwd() failed");
    }  
}

void smallsh_status(int last_status_or_signal){

    if(last_status_or_signal != 0 && last_status_or_signal != 1){
        printf("terminated by signal %d\n", last_status_or_signal);
        fflush(stdout);
    }else{
        printf("exit value %d\n", last_status_or_signal);
        fflush(stdout);
    }
}
//--------------------------------------------------------------------------------------------------------------------


// Other commands-----------------------------------------------------------------------------------------------------
void smallsh_other_child_process(struct command_line *curr_command){
    // check if input/output redirection is needed
    //input redirection
    sigset_t block_sigstp;
    // block SIGTSTP in the child process
    sigemptyset(&block_sigstp);
    sigaddset(&block_sigstp, SIGTSTP);
    sigprocmask(SIG_BLOCK, &block_sigstp, NULL);

    if (curr_command->is_bg == false || fg_mode == true){
        // if child is in fg, unblock SIGTINT
        sigset_t unblock_sigint;

        // block SIGTSTP in the main process
        sigemptyset(&unblock_sigint);
        sigaddset(&unblock_sigint, SIGINT);
        sigprocmask(SIG_UNBLOCK, &unblock_sigint, NULL); 
        signal(SIGINT, handle_SIGINT); // set SIGINT handler
    }

    if(curr_command->input_file != NULL){
        int input_fd = open(curr_command->input_file, O_RDONLY);
        if(input_fd == -1){
            fprintf(stderr, "cannot open %s for input\n", curr_command->input_file);
            _exit(1);
        }
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }

    //output redirection
    if(curr_command->output_file != NULL){

        int output_fd = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);//open file for writing, create if it doesn't exist, truncate if it does, 
                                                                                            //permissions read, write for owner, read for group and others 
        if(output_fd == -1){                         
            fprintf(stderr, "cannot open %s for output\n", curr_command->output_file);
            exit(1);
        }
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
        //no output redirection
    }
    // use execvp() to execute the command
    execvp(curr_command->argv[0], curr_command->argv);
    // do i need to retore the input/output file descriptors?
    // if execvp() fails, print error message and exit
    fprintf(stderr, "%s: no such file or directory\n", curr_command->argv[0]);
    exit(1);
    
}
void smallsh_other_parent_process_bg(int child_id, int *bg_pids, int *bg_count, int *last_pid){

    // do not wait for child process to finish
    printf("background pid is %d\n", child_id);
    fflush(stdout);
    bg_pids[*bg_count] = child_id;
    (*bg_count)++;   
    *last_pid = child_id;
}

void smallsh_check_bg_procs(int *last_status_or_signal, int *bg_pids, int *bg_count){
    for(int i = 0; i < *bg_count; i++){
        // check if background processes have finished
        int bg_status;
        pid_t bg_pid = waitpid(bg_pids[i], &bg_status, WNOHANG);
        if (bg_pid > 0){
            if (WIFEXITED(bg_status)){
                *last_status_or_signal = WEXITSTATUS(bg_status);
            } else if (WIFSIGNALED(bg_status)){
                *last_status_or_signal = WTERMSIG(bg_status);
            }
            // remove the pid from the array
            for (int j = i; j < *bg_count - 1; j++){
                bg_pids[j] = bg_pids[j + 1];
            }
            (*bg_count)--; // decrement number of bg processes
            i--; // decrement index to account for removed pid
            if(*last_status_or_signal != 0 && *last_status_or_signal != 1){
                printf("background pid %d is done: terminated by signal %d\n", bg_pid, *last_status_or_signal);
                fflush(stdout);
            } else {
                // print exit value
                printf("background pid %d is done: exit value %d\n", bg_pid, *last_status_or_signal);
                fflush(stdout);
            }
        }
    }
}

void smallsh_other_parent_process_fg(int *last_status_or_signal, int child_id, int *last_pid){
    // wait for child process to finish
    int child_status;
    waitpid(child_id, &child_status, 0);
    if (WIFEXITED(child_status)){
        *last_status_or_signal = WEXITSTATUS(child_status);
    } else if (WIFSIGNALED(child_status)){
        *last_status_or_signal = WTERMSIG(child_status);
    }

    if(*last_status_or_signal != 0 && *last_status_or_signal != 1){
        printf("terminated by signal %d\n", *last_status_or_signal);
        fflush(stdout);
    }
    *last_pid = child_id;
}

void smallsh_other_commands(struct command_line *curr_command, int *bg_pids, int *bg_count, int *last_status_or_signal, int *last_pid){
    /*
    other commands will be handled by execlp()/execcvp(), fork() and waitpid()
    1. fork() creates a child process
        a. parent process waits for the child process to finish using waitpid()

    2. child process uses execlp()/execcvp() to execute the command

    3. shell uses PATH variable to look for non-built in commands and it must allow shell scripts to be executed

    4. If a command fails because the shell could not find the command to run,  
    then the shell will print an error message and set the exit status to 1

    5. child process must terminate afte running a command ( whether the command is successful or fails) 

    */   

    // fork a child process
    pid_t child_id = fork();
    
    switch (child_id){
        case -1:
            perror("fork() failed");
            exit(1);
            break;
        case 0: //child process
            
            smallsh_other_child_process(curr_command);
            break;
        default: // parent process


            // fg processes
            if(curr_command->is_bg == false || fg_mode == true){
                smallsh_other_parent_process_fg(last_status_or_signal, child_id, last_pid);
            // bg processes
            } else if (curr_command->is_bg == true && fg_mode == false){
                smallsh_other_parent_process_bg(child_id, bg_pids, bg_count, last_pid);
            }

    }

        
}
//--------------------------------------------------------------------------------------------------------------------


// Main function----------------------------------------------------------------------------------------------------
int main(){
    struct command_line *curr_command = NULL;
    int last_pid = 0;
    int last_status_or_signal = 0;
    int *bg_pids = malloc(sizeof(int) * 100); // array to store background pids
    int bg_count = 0; // number of background pids
    sigset_t block_set;

    // block SIGINT in the main process
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);
    sigprocmask(SIG_BLOCK, &block_set, NULL); 

    //handle SIGSTP (paraent process)
    signal(SIGTSTP, handle_SIGTSTP); // set SIGTSTP handler
     



    while(true){

        if (curr_command != NULL) {
            free_command(curr_command); // Free the previously allocated memory
        }

        smallsh_check_bg_procs(&last_status_or_signal, bg_pids, &bg_count); // check for background processes
        curr_command = parse_input();

        if (curr_command->argc ==0){ // empty commands
            continue; // Ignore empty commands
        }
        if (curr_command->argv[0][0] == '#'){ // comment
            continue; // Ignore comments
        }
        // check for built in commands
        if(strcmp(curr_command->argv[0], "exit") == 0 && curr_command->argc == 1){ // exit
             smallsh_exit(curr_command, bg_pids, &bg_count);
        }
        else if(strcmp(curr_command->argv[0], "cd") == 0){ // change dir
            smallsh_cd(curr_command);
        } else if(strcmp(curr_command->argv[0], "status") == 0){ // show status of last process, not including built in commands
            smallsh_status(last_status_or_signal);
        }
        else if (curr_command->argc > 0){ // all other commands
            smallsh_other_commands(curr_command, bg_pids, &bg_count, &last_status_or_signal, &last_pid);
        }

    }
    return EXIT_SUCCESS;
}
//--------------------------------------------------------------------------------------------------------------------