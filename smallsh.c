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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "smallsh.h"

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

int main(){
    struct command_line *curr_command;
    int last_pid = 0;
    int last_status_or_signal = 0;
    int *bg_pids = malloc(sizeof(int) * 100); // array to store background pids
    int bg_count = 0; // number of background pids



    while(true){
        
        if (curr_command != NULL) {
            free_command(curr_command); // Free the previously allocated memory
        }
        curr_command = parse_input();

        if (curr_command->argc ==0){ // empty commands
            continue; // Ignore empty commands
        }
        if (curr_command->argv[0][0] == '#'){ // comment
            continue; // Ignore comments
        }
        if(strcmp(curr_command->argv[0], "exit") == 0 && curr_command->argc == 1){ // exit
            /*
            cases to handle
            1. exit command is run
                a. kill all child processes
                b. exit the shell

            */
            //kill all child processes
            //exit the shell
            free_command(curr_command);
            free(bg_pids);
            exit(0); //do i need more than this?
        }
        else if(strcmp(curr_command->argv[0], "cd") == 0){ // change dir
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
                
            
        }else if(strcmp(curr_command->argv[0], "status") == 0){ // show status of last fg(?) process
            if(last_pid == 0){

                printf("exit value %d\n", last_status_or_signal);
                fflush(stdout);
            }else{
                return last_status_or_signal;
            }
            
        }else if (curr_command->argc > 0){ // all other commands
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
                    // check if input/output redirection is needed
                    //input redirection
                    if(curr_command->input_file != NULL){
                        int input_fd = open(curr_command->input_file, O_RDONLY);
                        if(input_fd == -1){
                            perror("open() failed");
                            exit(1);
                        }
                        dup2(input_fd, STDIN_FILENO);
                        close(input_fd);
                        //no input redirection
                    } else if(curr_command->input_file == NULL){
                        // if no input file, set input to /dev/null
                        int input_fd = open("/dev/null", O_RDONLY);
                        if(input_fd == -1){
                            perror("open() failed");
                            exit(1);
                        }
                        dup2(input_fd, STDIN_FILENO);
                        close(input_fd);
                    }
                    //output redirection
                    if(curr_command->output_file != NULL){
                        int output_fd = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);//open file for writing, create if it doesn't exist, truncate if it does, 
                                                                                                            //permissions read, write for owner, read for group and others 
                        if(output_fd == -1){
                            perror("open() failed");
                            exit(1);
                        }
                        dup2(output_fd, STDOUT_FILENO);
                        close(output_fd);
                        //no output redirection
                    } else if(curr_command->output_file == NULL){
                        // if no output file, set output to /dev/null
                        int output_fd = open("/dev/null", O_WRONLY);
                        if(output_fd == -1){
                            perror("open() failed");
                            exit(1);
                        }
                        dup2(output_fd, STDOUT_FILENO);
                        close(output_fd);
                    }
                    // execute process
                    // there's more to this....
                    printf("executed/n");
                    fflush(stdout);
                    execlp(curr_command->argv[0], curr_command->argv[0], NULL);
                    // do i need to retore the input/output file descriptors?
                    // if execlp() fails, print error message and exit
                    perror("execlp() failed");
                    exit(1);
                    break;
                default: // parent process
                    int child_status;
                    if(curr_command->is_bg == false){
                        // wait for child process to finish
                        // i feel decent about this
                        waitpid(child_id, &child_status, 0);
                        if (WIFEXITED(child_status)){
                            last_status_or_signal = WEXITSTATUS(child_status);
                        } else if (WIFSIGNALED(child_status)){
                            last_status_or_signal = WTERMSIG(child_status);
                        }
                        last_pid = child_id;
                    } else if (curr_command->is_bg == true){
                        // do not wait for child process to finish
                        printf("background pid is %d\n", child_id);
                        fflush(stdout);
                        bg_pids[bg_count] = child_id;
                        bg_count++;
                        for(int i = 0; i < bg_count; i++){
                            // check if background processes have finished
                            int bg_status;
                            pid_t bg_pid = waitpid(bg_pids[i], &bg_status, WNOHANG);
                            if (bg_pid > 0){
                                if (WIFEXITED(bg_status)){
                                    last_status_or_signal = WEXITSTATUS(bg_status);
                                } else if (WIFSIGNALED(bg_status)){
                                    last_status_or_signal = WTERMSIG(bg_status);
                                }
                                // remove the pid from the array
                                for (int j = i; j < bg_count - 1; j++){
                                    bg_pids[j] = bg_pids[j + 1];
                                }
                                bg_count--; // decrement number of bg processes
                                i--; // decrement index to account for removed pid
                                printf("background pid %d is done: exit value %d\n", bg_pid, last_status_or_signal);
                                fflush(stdout);
                            }
                        }
                        

                        last_pid = child_id;
                    }

            }

        }

    }
    return EXIT_SUCCESS;
}
