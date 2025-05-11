#define INPUT_LENGTH 2048
#define MAX_ARGS 512

// Struct copied from sample_parser.c givien in assignment 4
struct command_line{
    char *argv[MAX_ARGS + 1];
    int argc;
    char *input_file;
    char *output_file;
    bool is_bg;
};



//built_commands.c
void smallsh_exit();
void smallsh_cd(struct command_line *curr_command);
void smallsh_status(struct command_line *curr_command);
