#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include<sys/wait.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <string.h>
#include "storage.h"
#define MAX 512
#define TOKEN_SIZE 512
#define STOP_WORD " \t\r\n\a"
#define BIN "/bin/"
#define RDSTDOUT ">"


// global
char* output_file;


/**
 * @brief Get the length of a command
 * 
 * @param cmd 
 * @return the length of the command
 */
int get_length_of_command(char** cmd)
{
    int length = 0;
    while(*(cmd + length) != NULL)
    {
        length++;
    }
    return length;
}

/**
 * @brief Given a single command word and a string of command, remove
 *        the command word from the string if it exist
 * @param target > symbol
 * @param cmd command string
 * @param filename : pointer to the output file
 * @return 0 if misformat. Otherwise 1
 */
int remove_command(char* target, char** cmd)
{
    int length = 0;
    int clean = 0;
    // print_command(cmd);
    while(*(cmd + length) != NULL)
    {
        if(strcmp(*(cmd + length), target) == 0 || clean)
        {
            if(clean == 0)
            {
                // check formatting
                if (*(cmd + length + 2) != NULL || *(cmd + length + 1) == NULL || length == 0)
                {
                    write(2, "Redirection misformatted.\n", strlen("Redirection misformatted.\n"));
                    return 0;
                }
                // Aquire file name
                if (*(cmd + length + 1) != NULL)
                {
                    output_file = *(cmd + length + 1);
                }
            }
            clean = 1;
            *(cmd + length) = NULL;
        }
        length++;
    }
    return 1;
}

/**
 * @brief Tokenize the string using whitespace as delimiters
 * @param line The line return by read_line
 * @return char** 
 */
char **split_line(char *line)
{
    int buf_size = TOKEN_SIZE;
    int pos = 0;
    char **tokens = malloc(buf_size * sizeof(char*));
    char *token;

    // if(!tokens)
    // {
    //     write(1, "allocation error\n", strlen("allocation error\n"));
    //     _exit(1); 
    // }

    token = strtok(line, STOP_WORD);
    while(token != NULL)
    {
        tokens[pos] = token;
        pos++;

        // create larger buffer 
        if (pos >= buf_size)
        {
            buf_size += TOKEN_SIZE;
            tokens = realloc(token,buf_size * sizeof(char*));
            if(!tokens)
            {
                write(1, "allocation error\n", strlen("allocation error\n"));
                _exit(1); 
            }
        }

        token = strtok(NULL, STOP_WORD);
    }
    tokens[pos] = NULL;
    return tokens;
}

/**
 * @brief Reading a line from stdin TODO: unable to read from file
 * Start with a block, if user input exceed it, reallocate with more space
 * @return the line we read
 */
char *read_single_line(FILE* fp)
{
    int buf_size = MAX;
    int pos = 0;
    char *buf = malloc(sizeof(char) * buf_size);
    int c;

    if (!buf)
    {
        write(1, "allocation error\n", strlen("allocation error\n"));
        _exit(1);
    }

    while(1)
    {

        // Read a character
        c = fgetc(fp);
        // If we hit EOF, replace it with a null character and return
        if(c == EOF || c == '\n')
        {
            buf[pos] = '\0';
            return buf;
        }
        else
        {
            buf[pos] = c;
        }
        pos++;

        // if we have exceeded the buffer, reallocate
        if (pos >= buf_size)
        {
            buf_size += MAX;
            buf = realloc(buf,buf_size);
            if(!buf)
            {
                write(1, "allocation error\n", strlen("allocation error\n"));
                _exit(1); 
            }
        }
    }
}

/**
 * @brief Decide which mode the shell is going to run on 
 * 
 * @param argc number of argument
 * @param argv array of argument
 * @return FILE* 
 */
FILE* choose_mode(int argc, char *argv[])
{
    FILE *fp;
    // Check the number of argument
    if (argc == 1)
    {
        // Interactive mode
        fp = stdin;
    }
    else if(argc == 2)
    {
        // Batch file mode
        fp = fopen(argv[1],"r");
        if (fp == NULL)
        {
            // Open file error
            char *prompt = "Error: Cannot open file ";
            write(STDERR_FILENO, prompt, strlen(prompt));
            write(STDERR_FILENO, argv[1], strlen(argv[1]));
            write(STDERR_FILENO, ".\n", strlen(".\n"));
            _exit(1);
        }
    }
    else
    {
        // Something went wrong
        char *prompt = "Usage: mysh [batch-file]\n";
        write(STDERR_FILENO, prompt, strlen(prompt));
        _exit(1);
    }
    
    return fp; // TODO: return a FILE* pointer fp
}


/**
 * @brief mysh built-in function "exit"
 *        Shell will be terminated if "exit" command detected
 * @param cmd 
 */
void check_exit(char** cmd)
{
    if (strcmp(cmd[0],"exit") == 0)
    {
        exit(0);
    }
}

// extract command for alias
char* extract_command(char**cmd)
{
    char *output;
    int idx = 2; // starting point to read command
    char cc_cmd[MAX];
    strcpy(cc_cmd,cmd[idx]);

    while(cmd[idx+1] != NULL)
    {
        strcat(cc_cmd," ");
        strcat(cc_cmd, cmd[idx+1]);
        idx++;
    }
    output = cc_cmd;
    return output;
}

/**
 * @brief mysh built-in function "alias"
 *        Perform 3 different modes of alias depend on the argument 
 * @param cmd 
 * @return 1 if alias run through, 0 if not
 */
int check_alias(char** cmd)
{
    // Mode 1: print out all of the aliases and their corresponding replacement
    if (strcmp(cmd[0],"alias") == 0 && get_length_of_command(cmd) == 1)
    {
        printList();
        return 1;
    }
    // Mode 2:
    else if (strcmp(cmd[0],"alias") == 0 && get_length_of_command(cmd) > 2)
    {
        // Dangerous alias check
        if(strcmp(cmd[1],"alias") == 0 || strcmp(cmd[1],"unalias") == 0 || strcmp(cmd[1],"exit") == 0)
        {
            printf("alias: Too dangerous to alias that.\n");
            fflush(stdout);
            return 1;
        }
        struct node* cur = find(cmd[1]);
        if(cur != NULL)
        {
            // Need overwrite
            char *replacement_cmd = extract_command(cmd);
            char execv_cmd[MAX];
            strcpy(execv_cmd,replacement_cmd);
            char **tokencmd = split_line(execv_cmd);

            cur->replacement = replacement_cmd;
            cur->tkcmd = tokencmd;
            return 1;
        }
        else if(get_length_of_command(cmd) > 3 && cur == NULL)
        {
            int i;
            char **replacement_cmd = malloc(TOKEN_SIZE * sizeof(char*));
            char *replacement = malloc(sizeof(char*));
            for(i = 2; i < get_length_of_command(cmd); i++)
            {
                replacement_cmd[i-2] = cmd[i];
                strcat(replacement,cmd[i]);
                if (i != get_length_of_command(cmd)-1)
                {
                    strcat(replacement," ");
                }
            } 

            insert(cmd[1], replacement, replacement_cmd);
           
        }
        else if (get_length_of_command(cmd) == 3 && cur == NULL)
        {
           // base case insert 
           char **tokencmd = split_line(cmd[2]);
           insert(cmd[1], cmd[2],tokencmd); 
        }
        return 1;
    }
    // Mode 3:
    else if (strcmp(cmd[0],"alias") == 0 && get_length_of_command(cmd) == 2)
    {
        // TODO: -Print out the alias and the corresponding replacement if alias exist
        //       -Otherwise just continue
        struct node *single = find(cmd[1]);
        if(single == NULL)
        {
            return 1;
        }
        else
        {
            printf("%s %s\n", single->alias,single->replacement);
            fflush(stdout);
            return 1;
        }
    }
    return 0;
}



/**
 * @brief mysh built-in function "unalias"
 *        -Delete the alias from the linked-list if alias exist. Else continue
 *        -If unalias has too many argument, handle it accordingly
 * @param cmd 
 * @return 1 if program run through. Otherwise 0
 */
int check_unalias(char** cmd)
{
    // Correct number of argument
    if (strcmp(cmd[0],"unalias") == 0 && get_length_of_command(cmd) == 2)
    {
        // TODO: delete if exist. Otherwise nothing happen and continue
        discard(cmd[1]);
        return 1;
    }
    else if (strcmp(cmd[0],"unalias") == 0 && get_length_of_command(cmd) != 2)
    {
        // If the user does not specify <alias-name> or there are too many arguments to unalias
        printf("unalias: Incorrect number of arguments.\n");
        fflush(stdout);
        return 1;
    }
    return 0;
}

// helper method for redirect
int reoncstruction(char** cmd)
{
        int len = get_length_of_command(cmd);
        int i;
        int idx_reconstruction;
        char ** temp_storage = malloc(TOKEN_SIZE * sizeof(char*));
        int idx = 0;
        int position; // 2: left, 3: right
        for (i = 0; i < len; i++)
        {
            char* locate = strchr(cmd[i], '>');
            if(locate != NULL)
            {
                // check for special case: > is the token
                if (strcmp(cmd[i], ">") == 0) return 0;
                if(strcmp(locate,">") == 0)
                {
                    // > is on the right side of the word
                    position = 3;
                }
                else
                {
                    // > is on the left side of the word
                    position = 2;
                }
                idx_reconstruction = i;
                char* temp = cmd[i];
                // get the first token
                char *token;
                token = strtok(temp, ">");
                while(token != NULL)
                {
                    temp_storage[idx] = token;
                    idx++;
                    token = strtok(NULL, ">");
                }
            }
        }
        // Reconstrcution
        if (get_length_of_command(temp_storage) == 2)
        {
            // shift 2 to the right
            for (int i = get_length_of_command(cmd) - 1; i >= idx_reconstruction; i--)
            {
                cmd[i+2] = cmd[i];
            }
            cmd[idx_reconstruction] = temp_storage[0];
            cmd[idx_reconstruction+1] = ">";
            cmd[idx_reconstruction+2] = temp_storage[1];
        }
        else if (get_length_of_command(temp_storage) == 1)
        {
            // > is found in this command token
            // > is on the right side of the word
            // shift 1 to the right
            if (position == 3 )
            {
                for (int i = get_length_of_command(cmd) - 1; i > idx_reconstruction; i--)
                {
                    cmd[i+1] = cmd[i];
                }
                cmd[idx_reconstruction+1] = ">";
            }
            if (position == 2)
            {
                for (int i = get_length_of_command(cmd) - 1; i > idx_reconstruction; i--)
                {
                    cmd[i+1] = cmd[i];
                }
                cmd[idx_reconstruction] = ">";
                cmd[idx_reconstruction + 1] = temp_storage[0];
            }
        }
        return 0;
}

/**
 * @brief Redirect stdout to a file provided by the user
 * 
 * @param cmd 
 * @return 1 if > is detected (execv)
 *         0 if no > is detected (execv)
 *         2 if Redirection misformatted (error continue)
 */
int check_rdstdout(char** cmd)
{
    int length = get_length_of_command(cmd);
    int i,j;
    int count = 0;
    // check the number of > inside of the command
    for (i = 0; i < length; i++)
    {
        for(j = 0; j < strlen(cmd[i]); j++)
        {
            if(cmd[i][j] == '>')
            {
                // > is part of the token
                count++;
            }
        }
    }
    // Check formatting
    if(count > 1)
    {
        write(2, "Redirection misformatted.\n", strlen("Redirection misformatted.\n"));
        return 2;
    }

    // Reconstruct the command
    if(count == 1) 
    {
        reoncstruction(cmd);
        return 1;
    }
    return 0;
}

/**
 * @brief execute the command 
 * 
 * @param command 
 * @return int 
 */
int execute(char **command)
{
    // if alias exist, excute the corresponding replacement
    struct node *easy = find(command[0]);
    if(easy == NULL)
    {
        // Alias not exist
    }
    else
    {
        // Alias exist
        command = easy->tkcmd;
    }
    if (execv(command[0], command) == -1)
    {
        write(STDERR_FILENO, command[0],strlen(command[0]));
        write(STDERR_FILENO, ": Command not found.\n", 21);
        free(command);
        _exit(1);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    FILE* fp = choose_mode(argc, argv);
    int rd = 0; // 0: No redirection. 1: Redirection
    while(!feof(fp))
    {
        char** cmd; // command that will get carried out by the child process
        if (fp == stdin)
        {
            // Activated interactive mode
            printf("mysh> "); 
            fflush(stdout);
        }
        
        char *line = read_single_line(fp);
        if ((*line) == '\0') {
                printf("%s\n", line);
                continue;
        }

        // Echo the command to stdout under batch mode
        if (fp != stdin) 
        {
            printf("%s\n",line);
            fflush(stdout);
        }
        cmd = split_line(line);

        // Deal with empty line of command
        if (cmd[0] == NULL) continue;

        // Check built-in command exit
        check_exit(cmd);

        // TODO: check built-in command alias
        if (check_alias(cmd)) continue;
        // TODO: check built-in command unalias
        if (check_unalias(cmd)) continue;
        // TODO: check redirection output
        if (check_rdstdout(cmd) == 2) continue;
        // printf("%d\n",check_rdstdout(cmd));
        if (check_rdstdout(cmd) == 1)
        {
            rd = 1;

            if (remove_command(RDSTDOUT,cmd) == 0) 
            {
                continue;
            }
        }
        else
        {
            rd = 0;
        }

        int pid = fork();
        if (pid == 0)
        {
            // Child process is running
            // Handling redirection
            if(rd == 1)
            {

                int fd = open(output_file, O_WRONLY | O_CREAT, 0777);

                // Overwrite files that already exist
                FILE* fp2 = fopen(output_file, "w");
                if (fp2 == NULL)
                {
                    printf("Cannot write to file %s.\n", output_file);
                    fflush(stdout);
                    exit(0);
                }
                fseek(fp2, 0 , SEEK_SET);

                // replace the stdout file with the file user provided
                dup2(fd,1);
                close(fd);

            }
            execute(cmd);
            _exit(0);
        }
        else if (pid < 0)
        {
            printf("forking error");
            fflush(stdout);
        }
        else
        {
            // Parent process is running
            wait(&pid);
        }
    }
    fflush(fp);
    _exit(0);
    return 0;
}
