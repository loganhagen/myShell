#include "myShell.h"
#include "LinkedListAPI.h"

// This holds all of the relevant information about the user line.
Input* new_input() { 
    Input* input = malloc(sizeof(Input));

    // Malloc space for strings.
    input->line = malloc(sizeof(char) * INPUT_SIZE);
    input->outfile = malloc(sizeof(char) * FILENAME_MAX);
    input->infile = malloc(sizeof(char) * FILENAME_MAX);
    input->tokens = malloc(sizeof(char*) * NUM_TOKENS);
    for (int i = 0; i < NUM_TOKENS; i++) {
        input->tokens[i] = malloc(sizeof(char) * TOKEN_SIZE);
    }
    input->cmd = malloc(sizeof(char*) * NUM_TOKENS);
    for (int i = 0; i < NUM_TOKENS; i++) {
        input->cmd[i] = malloc(sizeof(char) * TOKEN_SIZE);
    }
    input->right_cmd = malloc(sizeof(char*) * NUM_TOKENS);
    for (int i = 0; i < NUM_TOKENS; i++) {
        input->right_cmd[i] = malloc(sizeof(char) * TOKEN_SIZE);
    }

    // Initialize flags;
    input->background = 0;
    input->first_inputr = 0;
    input->last_inputr = 0;
    input->first_outputr = 0;
    input->last_outputr = 0;
    input->pipe_pos = 0;
    input->builtin = 0;
    input->r = 0;

    return input;
}

void loop() {
    // Initialize process list, environment variables and read in profile.
    List* processes = initializeList(&dummyPrint, &deleteProcess, &dummyCompare);
    Env* env = malloc(sizeof(Env));
    init_env(env);
    read_profile(env);
    // Main execution loop.
    while (1) {
        Input* input = new_input();
        get_line(env, input);
        tokenize(input);
        find_builtin(input);
        // Find operators.
        fr_background(input);
        find_inputr(input);
        find_outputr(input);
        find_pipe(input);
        init_cmd(input);
        // Branches for redirection.
        if (input->first_inputr > 0 && input->first_outputr > 0) {
            init_doubler(input);
        } else if (input->first_inputr > 0) {
            init_inputr(input);
        } else if (input->first_outputr > 0) {
            init_outputr(input);
        }
        // Branch for piping.
        if (input->pipe_pos > 0) {
            init_pipe(input); 
        }
        // Branch for builtins.
        if (input->builtin > 0) {
            if (input->builtin == 1) {
                export(input, env);
            } else if (input->builtin == 2) {
                if (strcmp(input->tokens[1], "~") == 0) {
                    chdir(env->myHOME);
                } else {
                    chdir(input->tokens[1]);
                }
            } else if (input->builtin == 3) {
                history(env, input);
            } else if (input->builtin == 4) {
                echo(input, env);
            }
        } else {
            // Exec is called here.
            branch_to_execute(input, processes, env);
        }
        // Debug functions:
        // print_env(env);
        // printInput(input);
    }
}

/**
 * @brief Get the command from the user. Also handles
 * the user prompt.
 * 
 * @param env 
 * @param input 
 */
void get_line(Env* env, Input *input) {
    // Get current working directory.
    char* path = malloc(sizeof(char) * INPUT_SIZE);
    getcwd(path, INPUT_SIZE);
    // Print shell prompt and get input. 
    printf("%s$: ", path);
    fgets(input->line, INPUT_SIZE, stdin);
    // Add line to history file.
    add_history(env, input->line);
}


/**
 * @brief Copies the line string into a temporary string (since strtok() modifies its arguments)
 * and splits the copy into tokens using a single space as a delimiter. 
 * @param input 
 */
void tokenize(Input *input) {
    char* s = malloc(sizeof(char) * INPUT_SIZE); 
    char* t = malloc(sizeof(char) * TOKEN_SIZE);
    strcpy(s, input->line);

    // Replace newline with null character.
    s[strcspn(s, "\n")] = '\0';

    int pos = 0;
    t = strtok(s, " ");
    while (t) {
        input->tokens[pos] = t;
        pos++;
        t = strtok(NULL, " ");
    }

    // Append null line to end of tokens array.
    input->tokens[pos] = NULL;
}

/**
 * @brief Check if builtin command provided.
 * 
 * @param input 
 */
void find_builtin(Input* input) {
    if (strcmp(input->tokens[0], "export") == 0) {
        input->builtin = 1;
    }
    if (strcmp(input->tokens[0], "cd") == 0 ) {
        input->builtin = 2;
    }
    if (strcmp(input->tokens[0], "history") == 0) {
        input->builtin = 3;
    }
    if (strcmp(input->tokens[0], "echo") == 0) {
        input->builtin = 4;
    }

}

/**
 * @brief Check if command is to be run in the background.
 * 
 * @param input 
 */
void fr_background(Input* input) {
    for (int i = 0; input->tokens[i]; i++) {
        if (strcmp(input->tokens[i], "&") == 0) {
            input->background = 1;
            input->tokens[i] = NULL;
            break;
        }
    }
}

/**
 * @brief Find the locations of input redirection operators, 
 * if one or more exists.
 * 
 * @param input 
 */
void find_inputr(Input* input) {
    int first = 0;
    for (int i = 0; input->tokens[i]; i++) {
        if (strcmp(input->tokens[i], "<") == 0) {
            first = i;
            break;
        }
    }
    int last = 0;
    for (int i = 0; input->tokens[i]; i++) {
        if (strcmp(input->tokens[i], "<") == 0) {
            if (i > last) {
                last = i;
            }
        }
    }
    if (first == last) {
        input->first_inputr = first;
    } else {
        input->first_inputr = first;
        input->last_inputr = last;        
    }
}

/**
 * @brief Find the locations of output redirection operators,
 * if one or more exists.
 * 
 * @param input 
 */
void find_outputr(Input* input) {
    int first = 0;
    for (int i = 0; input->tokens[i]; i++) {
        if (strcmp(input->tokens[i], ">") == 0) {
            first = i;
            break;
        }
    }
    int last = 0;
    for (int i = 0; input->tokens[i]; i++) {
        if (strcmp(input->tokens[i], ">") == 0) {
            if (i > last) {
                last = i;
            }
        }
    }
    
    if (first == last) {
        input->first_outputr = first;
    } else {
        input->first_outputr = first;
        input->last_outputr = last;        
    }
}

/**
 * @brief Find the location of the pipe operator, if
 * it exists.
 * 
 * @param input 
 */
void find_pipe(Input* input) {
    int pipe_pos = 0;
    int i;
    for (i = 0; input->tokens[i]; i++) {
        if (strcmp(input->tokens[i], "|") == 0) {
            pipe_pos = i;
        }
    }
    input->pipe_pos = pipe_pos;
}

/**
 * @brief Initialize the cmd field of the input struct, based on
 * the presence and location of various operators.
 * 
 * @param input 
 */
void init_cmd(Input* input) {
    int i;
    if (input->first_outputr > 0 && input->first_inputr > 0) {
        for (i = 0; i < input->first_inputr; i++) {
            input->cmd[i] = input->tokens[i];
        }
        input->cmd[i] = NULL;
    } else if (input->first_outputr > 0) {
        for (i = 0; i < input->first_outputr; i++) {
            input->cmd[i] = input->tokens[i];
        }
        input->cmd[i] = NULL;
    } else if (input->first_inputr > 0) {
        for (i = 0; i < input->first_inputr; i++) {
            input->cmd[i] = input->tokens[i];
        }
        input->cmd[i] = NULL;
    } else if (input->pipe_pos > 0) {
        for (i = 0; i < input->pipe_pos; i++) {
            input->cmd[i] = input->tokens[i];
        }
        input->cmd[i] = NULL;
    } else {
        for (i = 0; input->tokens[i]; i++) {
            input->cmd[i] = input->tokens[i];
        }
        input->cmd[i] = NULL;
    }
}

/**
 * @brief Initialize input and output file fields in the input
 * struct, when there is both input and output redirection.
 * 
 * @param input 
 */
void init_doubler(Input* input) {
    strcpy(input->infile, input->tokens[input->first_inputr + 1]);
    strcpy(input->outfile, input->tokens[input->first_outputr + 1]);
}

/**
 * @brief Initialize the cmd and input file fields when there is solely input redirection.
 * 
 * @param input 
 */
void init_inputr(Input* input) {
    int i;
    if (input->last_inputr > input->first_inputr) {
        strcpy(input->infile, input->tokens[input->last_inputr + 1]);
        for (i = 0; i < input->first_inputr; i++) {
            input->cmd[i] = input->tokens[i];
        }
    } else {
        strcpy(input->infile, input->tokens[input->first_inputr + 1]);
        for (i = 0; i < input->first_inputr; i++) {
            input->cmd[i] = input->tokens[i];
        }
    }
    input->cmd[i] = NULL;
}

/**
 * @brief Initialize the cmd and output file fields when there is solely output redirection.
 * 
 * @param input 
 */
void init_outputr(Input* input) {
    char* filename;
    char** cmd;

    filename = malloc(sizeof(char) * FILENAME_MAX);
    cmd = malloc(sizeof(char*) * NUM_TOKENS);
    for (int i = 0; i < NUM_TOKENS; i++) {
        cmd[i] = malloc(sizeof(char) * TOKEN_SIZE);
    }

    int i;
    if (input->last_outputr > input->first_outputr) {
        strcpy(filename, input->tokens[input->last_outputr + 1]);
        for (i = 0; i < input->first_outputr; i++) {
            cmd[i] = input->tokens[i];
        }
    } else {
        strcpy(filename, input->tokens[input->first_outputr + 1]);
        for (i = 0; i < input->first_outputr; i++) {
            cmd[i] = input->tokens[i];
        }
    }
    cmd[i] = NULL;

    strcpy(input->outfile, filename);
    for (i = 0; cmd[i]; i++) {
        input->cmd[i] = cmd[i];
    }
    input->cmd[i] = NULL;
}

/**
 * @brief Initialize the cmd and right_cmd fields based on the location
 * of the pipe operator, as well as the possible presence of output and input
 * redirection operators.
 * 
 * @param input 
 */
void init_pipe(Input* input) {
    int i, j;
    // If no output redirection.
    if (input->first_outputr == 0) {
        j = 0;
        for (i = input->pipe_pos + 1; input->tokens[i]; i++) {
            input->right_cmd[j] = input->tokens[i];
            j++;
        }
        input->right_cmd[j] = NULL;
    } else if (input->first_outputr > 0) {
        j = 0;
        for (i = input->pipe_pos + 1; i < input->first_outputr; i++) {
            input->right_cmd[j] = input->tokens[i];
            j++;
        }
        input->right_cmd[j] = NULL;
    }
    // If no input redirection.
    if (input->first_inputr == 0) {
        for (i = 0; i < input->pipe_pos; i++) {
            input->cmd[i] = input->tokens[i];
        }
        input->cmd[i] = NULL;
    }
}

/**
 * @brief Main execution loops which determines the scenario for
 * which execv() will be called. Also checks on background processes 
 * and kills child processes before exiting.
 * 
 * @param input 
 * @param processes 
 * @param env 
 */
void branch_to_execute(Input* input, List* processes, Env* env) {
    pid_t pid1, pid2, pid3;
    FILE* fp1; 
    FILE* fp2;
    int status1, status2, status3;
    int pipefd[2];

    checkBackground(processes);
    if (strcmp(input->tokens[0], "exit") == 0) {
        killProcesses(processes);
        _exit(EXIT_SUCCESS);
    }

    pid1 = fork();
    if (pid1 < 0) {
        perror("fork 1");
    }
    if (pid1 == 0) {
        if (input->first_inputr > 0 && input->first_outputr > 0) {
            fp1 = freopen(input->infile, "r", stdin);
            if (!fp1) {
                perror("freopen stdin");
                _exit(EXIT_FAILURE);
            }
            fp2 = freopen(input->outfile, "w", stdout); 
            if (!fp2) {
                perror("freopen stdout");
                _exit(EXIT_FAILURE);
            }
        } else if (input->first_inputr > 0) {
            fp1 = freopen(input->infile, "r", stdin);
            if (!fp1) {
                perror("freopen stdin");
                _exit(EXIT_FAILURE);
            }
        } else if (input->first_outputr > 0) {
            fp2 = freopen(input->outfile, "w", stdout); 
            if (!fp2) {
                perror("freopen stdout");
                _exit(EXIT_FAILURE);
            }
        }
        // Branch for piping.
        if (input->pipe_pos > 0) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                _exit(EXIT_FAILURE);
            }
            pid2 = fork();
            if (pid2 < 0) {
                perror("fork 2");
                _exit(EXIT_FAILURE);
            }
            // Branch for left side of pipe.
            if (pid2 == 0) {
                if (dup2(pipefd[1], STDOUT_FILENO) == -1) { 
                    perror("dup2 stdout");
                    _exit(EXIT_FAILURE);
                }
                if (close(pipefd[0])) {
                    perror("close");
                    _exit(EXIT_FAILURE);

                }
                if (close(pipefd[1])) {
                    perror("close");
                    _exit(EXIT_FAILURE);
                }
                execute_command(input, env);
                _exit(EXIT_FAILURE);
            }

            pid3 = fork();
            if (pid3 < 0) {
                perror("fork 3");
                _exit(EXIT_FAILURE);
            }
            // Branch for right side of pipe.
            if (pid3 == 0) {
                if ((dup2(pipefd[0], STDIN_FILENO)) == -1) { 
                    perror("dup2 stdin");
                    _exit(EXIT_FAILURE);
                }
                if ((close(pipefd[0]))) {
                    perror("close");
                    _exit(EXIT_FAILURE);
                }
                if ((close(pipefd[1]))) {
                    perror("close");
                    _exit(EXIT_FAILURE);
                }
                input->r = 1;
                execute_command(input, env);
                _exit(EXIT_FAILURE);          
            }

            // Close pipe.
            if ((close(pipefd[0]))) {
                perror("close");
                _exit(EXIT_FAILURE);
            }
            if ((close(pipefd[1]))) {
                perror("close");
                _exit(EXIT_FAILURE);
            }

            if (pid2 > 0) { 
                if (input->background == 0) {
                    waitpid(pid2, &status2, 0);
                } else {
                Process* p = malloc(sizeof(Process));
                p->pid = pid2;
                p->pos = getLength(processes) + 1;
                p->cmd = malloc(sizeof(char) * INPUT_SIZE);
                p->cmd = input->cmd;
                insertBack(processes, p);
                printf("[%d] %d\n", p->pos, p->pid);
                }
                
            }
            if (pid3 > 0) { 
                if (input->background == 0) {
                    waitpid(pid3, &status2, 0);
                } else {
                    Process* p = malloc(sizeof(Process));
                    p->pid = pid3;
                    p->pos = getLength(processes) + 1;
                    p->cmd = malloc(sizeof(char) * INPUT_SIZE);
                    p->cmd = input->cmd;
                    insertBack(processes, p);
                    printf("[%d] %d\n", p->pos, p->pid);
                }
            }
            _exit(EXIT_SUCCESS);
        }
        execute_command(input, env);
        _exit(EXIT_FAILURE);
    }
    if (pid1 > 0) {
        if (input->background == 0) {
            waitpid(pid1, &status1, 0);
        } else {
            Process* p = malloc(sizeof(Process));
            p->pid = pid1;
            p->pos = getLength(processes) + 1;
            p->cmd = malloc(sizeof(char) * INPUT_SIZE);
            p->cmd = input->cmd;
            insertBack(processes, p);  
            printf("[%d] %d\n", p->pos, p->pid);
        }
    }
}

/**
 * @brief Parses the myPATH variable for the applicable path to the
 * desired command, and executes.
 * 
 * @param input 
 * @param env 
 */
void execute_command(Input *input, Env* env) {
    char* tmppath = malloc(sizeof(char) * INPUT_SIZE);
    strcpy(tmppath, env->myPATH);
    int cmd_status, rightcmd_status;
    // If command was entered with an absolute path (i.e. "./a.out").
    if (strstr(input->cmd[0], ".")) {
        // Copy command.
        char* c = malloc(sizeof(char) * TOKEN_SIZE);
        strcpy(c, input->cmd[0]);
        int len = strlen(c);
        char* s = malloc(sizeof(char) * TOKEN_SIZE);
        int j = 0;
        for (int i = 2; i < len; i++) {
            s[j] = c[i];
            j++;
        }
        char* x[2];
        x[0] = malloc(sizeof(char) * TOKEN_SIZE);
        strcpy(x[0], s);
        x[1] = NULL;
        cmd_status = execv(input->cmd[0], x);
        // if (status == -1) {
        //     printf("%s: command not found\n", input->cmd[0]);
        //     return;
        // }
    // If command is not in the current director (i.e. "ls").
    } else {
        // If myPATH is set to default.
        if (!(strstr(tmppath, ":"))) {
            strcat(tmppath, "/");
            // If piping required.
            if (input->r > 0) {
                strcat(tmppath, input->right_cmd[0]);
                rightcmd_status = execv(tmppath, input->right_cmd);
                // if (status == -1) {
                //     printf("%s: command not found\n", input->right_cmd[0]);
                //     return;
                // }
            } else {
                strcat(tmppath, input->cmd[0]);
                cmd_status = execv(tmppath, input->cmd);
                // if (status == -1) {
                //     printf("%s: command not found\n", input->cmd[0]);
                //     return;
                // }
            }
        // If myPATH has been modified.
        } else {
            char* token = strtok(tmppath, ":");
            while (token) {
                char* s = malloc(sizeof(char) * INPUT_SIZE);
                strcpy(s, token);
                strcat(s, "/");
                if (input->r > 0) {
                    strcat(s, input->right_cmd[0]);
                    // printf("s: %s\n", s);
                    rightcmd_status = execv(s, input->right_cmd);
                    token = strtok(NULL, ":");
                } else {
                    strcat(s, input->cmd[0]);
                    // printf("s: %s\n", s);
                    cmd_status = execv(s, input->cmd);
                    token = strtok(NULL, ":");
                }
            }
            // if (input->r > 0) {
            //     if (status == -1) {
            //         printf("%s: command not found\n", input->right_cmd[0]);
            //         return;
            //     } else {
            //         if (status == -1) {
            //             printf("%s: command not found\n", input->cmd[0]);
            //             return;
            //         }      
            //     }
            // }
        }
    }
    if (cmd_status == -1) {
        printf("%s: command not found\n", input->cmd[0]);
    } else if (rightcmd_status == -1) {
        printf("%s: command not found\n", input->right_cmd[0]);
    }
}

/**
 * @brief Loops through linked list of background processes and kills them.
 * 
 * @param processes 
 */
void killProcesses(List* processes) {
    int status;
    pid_t pid;

    void* elem;
    ListIterator iterator = createIterator(processes);
    while (elem = nextElement(&iterator)) {
        Process* p = (Process*) elem;
        status = kill(p->pid, SIGKILL);
        deleteDataFromList(processes, p);
    }
}

/**
 * @brief Loops through linked list of processes and calls 
 * waitpid() for each.
 * 
 * @param processes 
 */
void checkBackground(List* processes) {
    int status;
    pid_t pid;

    void* elem;
    ListIterator iterator = createIterator(processes);
    while (elem = nextElement(&iterator)) {
        Process* p = (Process*) elem;
        if ((pid = waitpid(p->pid, &status, WNOHANG)) > 0) {
            printf("[%d] Done: %s\n", p->pos, p->cmd[0]);
            deleteDataFromList(processes, p);
        }
    }
}

/**
 * @brief Initialize the environment variable struct with default values.
 * 
 * @param env 
 */
void init_env(Env* env) {
    env->myPATH = malloc(sizeof(char) * INPUT_SIZE);
    // Initialize to default value.
    strcpy(env->myPATH, "/bin");

    // Initialize to return value of getenv().
    env->myHOME = malloc(sizeof(char) * INPUT_SIZE);
    strcpy(env->myHOME, getenv("HOME"));

    // Initialize to default value.
    env->myHISTFILE = malloc(sizeof(char) * INPUT_SIZE);
    strcpy(env->myHISTFILE, getenv("HOME"));
    strcat(env->myHISTFILE, "/.myShell_history");
}

/**
 * @brief Reads in a profile file, parses it, and populates the
 * appropriate environment variables.
 * m
 * @param env 
 */
void read_profile(Env* env) {
    int num;
    FILE *fp;

    char* path = malloc(sizeof(char) * INPUT_SIZE);
    strcpy(path, env->myHOME);
    strcat(path, "/.myShell_profile");

    fp = fopen(path,"r");
    if(!fp) {
        printf("Can't open profile file\n");
        return;
    }

    char* s = malloc(sizeof(char) * INPUT_SIZE);
    while(fgets(s, INPUT_SIZE, fp)) {  
        if (strstr(s, "myPATH")) {
            char* path = strtok(s, "=");
            path = strtok(NULL, "\0");
            path[strcspn(path, "\n")] = '\0';        
            strcpy(env->myPATH, path);
        } else if (strstr(s, "myHISTFILE")) {
            char* hist = strtok(s, "=");
            hist = strtok(NULL, "\0");
            hist[strcspn(hist, "\n")] = '\0';
            strcpy(env->myHISTFILE, hist);
        } else if (strstr(s, "myHOME")) {
            char* home = strtok(s, "=");
            home = strtok(NULL, "\0");
            home[strcspn(home, "\n")] = '\0';
            strcpy(env->myHOME, home);
        }
    }
    fclose(fp); 
}

/**
 * @brief Command to change the value of environment variables at any time.
 * 
 * @param input 
 * @param env 
 */
void export(Input* input, Env* env) {
    char* s = malloc(sizeof(char) * INPUT_SIZE);
    strcpy(s, input->line);

    if (strstr(s, "myPATH")) {
        // Get the path from the export command.
        char* path = strtok(s, "=");
        path = strtok(NULL, "\0");
        path[strcspn(path, "\n")] = '\0';
        strcat(env->myPATH, ":");
        strcat(env->myPATH, path);
    } else if (strstr(s, "myHOME")) {
        char* home = strtok(s, "=");
        home = strtok(NULL, "\0");
        home[strcspn(home, "\n")] = '\0';
        strcpy(env->myHOME, home);
    } else if (strstr(s, "myHISTFILE")) {
        char* hist = strtok(s, "=");
        hist = strtok(NULL, "\0");
        hist[strcspn(hist, "\n")] = '\0';
        strcpy(env->myHISTFILE, hist);
    } 
}

/**
 * @brief Either calls function to print history,
 * or deletes the history file, depending on parameters.
 * 
 * @param env 
 * @param input 
 */
void history(Env* env, Input* input) {
    if (!input->cmd[1]) {
        get_history(env, NULL);
    } else if (strcmp(input->cmd[1], "-c") == 0) {
        FILE* fp = fopen(env->myHISTFILE, "w+");
        if (!fp) {
            printf("Error opening file.\n");
            return;
        }
        fclose(fp);
    } else {
        get_history(env, input->cmd[1]);
    }
}

/**
 * @brief Add command to history file.
 * 
 * @param env 
 * @param line 
 */
void add_history(Env* env, char* line) {
    FILE* fp;

    fp = fopen(env->myHISTFILE, "a");
    if (!fp) {
        printf("Can't open history file\n");
        return;
    }

    fprintf(fp, "%s", line);
    fclose(fp);
}

/**
 * @brief Prints the history of inputted commands. Up to
 * n, if provided.
 * 
 * @param env 
 * @param n 
 */
void get_history(Env* env, char* n) {
    FILE *fp;
    int num, i;
    char* s = malloc(sizeof(char) * INPUT_SIZE);
    if (n) {
        num = atoi(n);
        fp = fopen(env->myHISTFILE,"r");
        if(!fp) {
            printf("Can't open history file\n");
            return;
        }

        i = 0; 
        while (fgets(s, INPUT_SIZE, fp) && i < num) {
            printf(" %d  %s", i + 1, s);
            i++;
        }

        fclose(fp); 
    } else {
        fp = fopen(env->myHISTFILE,"r");
        if(!fp) {
            printf("Can't open history file\n");
            return;
        }
        i = 0;
        while(fgets(s, INPUT_SIZE, fp)) {  
            printf(" %d  %s", i + 1, s);
            i++;
        }
        fclose(fp); 
    }
}

/**
 * @brief Prints the value of desired environment variables.
 * 
 * @param input 
 * @param env 
 */
void echo(Input* input, Env* env) {
    char* s = malloc(sizeof(char) * INPUT_SIZE);
    strcpy(s, input->line);

    if (strstr(s, "$myPATH")) {
        printf("%s\n", env->myPATH);
    } else if (strstr(s, "$myHOME")) {
        printf("%s\n", env->myHOME);
    } else if (strstr(s, "$myHISTFILE")) {
        printf("%s\n", env->myHISTFILE);
    } 
}

void print_env(Env* env) {
    printf("myPATH=%s\n", env->myPATH);
    printf("myHOME=%s\n", env->myHOME);
    printf("myHISTFILE=%s\n", env->myHISTFILE);
}

void printInput(Input* input) {
    printf("cmd: ");
    for (int i = 0; input->cmd[i]; i++) {
        printf("%s ", input->cmd[i]);
    }
    printf("\n");
    printf("right cmd: ");
    for (int i = 0; input->right_cmd[i]; i++) {
        printf("%s ", input->right_cmd[i]);
    }
    printf("\n");
    printf("infile: %s\n", input->infile);
    printf("outfile: %s\n", input->outfile);
}

char* dummyPrint(void* toBePrinted) {
    return "";
}

void deleteProcess(void* toBeDeleted) {
    Process* p = (Process*) toBeDeleted;
    for (int i = 0; i < NUM_TOKENS; i++) {
        free(p->cmd[i]);
    }
    free(p->cmd);
}

int dummyCompare(const void* first,const void* second) {
    return 0;
}