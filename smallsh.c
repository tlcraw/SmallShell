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
        //add conditions to handle blank lines and comments
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

    while(true){
        curr_command = parse_input();
        
        if(cmpstr(curr_command->argv, "exit") == 0 && curr_command->argc == 1){ // exit
            smallsh_exit();
        
        }else if(cmpstr(curr_command->argv[0], "cd") == 0){ // change dir
            smallsh_cd(&curr_command);
        }else if(cmpstr(curr_command->argv[0], "status") == 0){ // show status of last fg(?) process
            smallsh_status(&curr_command);
        }else if (curr_command->argc > 0){

        }

    }
    return EXIT_SUCCESS;
}
