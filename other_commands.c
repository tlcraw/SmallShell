


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