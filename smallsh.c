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
            int pid_t = getpid();
            int fork_val = fork();
            //if parent proc
            if (fork_val != 0){
                //if not a background process wait for the child to finish
                if (curr_command->is_bg == false){
                    waitpid()


                }
            }

        }

    }
    return EXIT_SUCCESS;
}
