/*
Taylor Crawford
CS 374 Spring 2025
Assignment 4L Small Shell
Created: 05/08/2026
Modified: 05/10/2025

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
        //conditions to handle blank lines and comments
        } else if(token[0] == '#'){
            break; // Ignore comments
        } else if(token[0] == '\0'){
            break; // Ignore blank lines
        } else{
            curr_command->argv[curr_command->argc++] = strdup(token);
        }

        token=strtok(NULL," \n");
    }
    return curr_command;
}


int main(){
    struct command_line *curr_command;
    int last_pid = 0;
    int last_status_or_signal = 0;
    int *bg_pids = malloc(sizeof(int) * 100); // array to store background pids
    int bg_count = 0; // number of background pids



    while(true){
        curr_command = parse_input();
        
        if(cmpstr(curr_command->argv, "exit") == 0 && curr_command->argc == 1){ // exit
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
        else if(cmpstr(curr_command->argv[0], "cd") == 0){ // change dir
            /*
            cases to handle

            1. no arguments
                navigate to home directory

            2. one argument
                a. absolute path
                b. relative path

            */

            if(curr_command->argc == 1){
                // if no args, set PWD to HOME
                setenv("PWD", getenv("HOME"), 1);
            } 
            else{
                // if one arg, check if it is absolute or relative path
                if (curr_command->argv[1][0] == '/'){
                    // if absolute path, set PWD to the absolute path
                    setenv("PWD", curr_command->argv[1], 1);
                } 
                else {
                    // if relative path, set PWD to the relative path
                    // this is a simplification, in reality we would need to resolve the relative path
                    char *current_dir = getenv("PWD");
                    char *new_dir = malloc(strlen(current_dir) + strlen(curr_command->argv[1]) + 2);

                    strcpy(new_dir, current_dir);
                    strcat(new_dir, "/");
                    strcat(new_dir, curr_command->argv[1]);
                    setenv("PWD", new_dir, 1);
                    free(new_dir);
                }
                
            }
        }else if(cmpstr(curr_command->argv[0], "status") == 0){ // show status of last fg(?) process
            if(last_pid == 0){
                exit(0);
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
            pid_t spawnpid = getpid();
            pid_t child_id = fork();
            
            switch (spawnpid){
                case -1:
                    perror("fork() failed");
                    exit(1);
                    break;
                case 0: //child process
                    // check if input/output redirection is needed
                    if(curr_command->input_file != NULL){
                        int input_fd = open(curr_command->input_file, O_RDONLY);
                        if(input_fd == -1){
                            perror("open() failed");
                            exit(1);
                        }
                        dup2(input_fd, STDIN_FILENO);
                        close(input_fd);
                    }
                    if(curr_command->output_file != NULL){
                        int output_fd = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);//open file for writing, create if it doesn't exist, truncate if it does, 
                                                                                                            //permissions read, write for owner, read for group and others 
                        if(output_fd == -1){
                            perror("open() failed");
                            exit(1);
                        }
                        dup2(output_fd, STDOUT_FILENO);
                        close(output_fd);
                    }
                    // execute process
                    // there's more to this....
                    execlp(curr_command->argv[0], curr_command->argv[0], NULL);

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
    return EXIT_SUCCESS;
}
