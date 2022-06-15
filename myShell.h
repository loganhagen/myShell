#ifndef SHELL_H
#define SHELL_H
#include <stdio.h>       
#include <stdlib.h>      
#include <string.h>     
#include <unistd.h>      
#include <sys/types.h>   
#include <sys/wait.h>    
#include <errno.h>   
#include <fcntl.h>
#include <LinkedListAPI.h>
#define NUM_TOKENS 64
#define TOKEN_SIZE 32
#define INPUT_SIZE 512

typedef struct {
    char* line;
    char* outfile;
    char* infile;                        
    char** tokens;
    char** cmd;  
    char** right_cmd;                                        
    int first_outputr;
    int last_outputr;
    int first_inputr;
    int last_inputr;
    int pipe_pos;                       
    int background;   
    int builtin;     
    int r;             
} Input;

typedef struct {
    pid_t pid;
    int pos;
    char** cmd;
} Process;

typedef struct {
    char* myPATH;
    char* myHISTFILE;
    char* myHOME;
} Env;

Input* new_input();
void loop();
void init_env(Env* env);
void read_profile(Env* env);
void get_line(Env* env, Input* input);
void add_history(Env* env, char* line);
void get_history(Env* env, char* n);
void history(Env* env, Input* input);
void tokenize(Input* input);
void find_builtin(Input* input);
void fr_background(Input* input);
void init_doubler(Input* input);
void find_inputr(Input* input);
void init_inputr(Input* input);
void find_outputr(Input* input);
void find_pipe(Input* input);
void init_cmd(Input* input);  
void export(Input* input, Env* env);
void echo(Input* input, Env* env);
void branch_to_execute(Input* input, List* processes, Env* env);
void init_outputr(Input* input);
void init_pipe(Input* input);
void check_for_operators(Input *input);
void initialize_pipe(Input* input);
void execute_command(Input *input, Env* env);
void checkBackground(List* processes);
void killProcesses(List* processes);
void printInput(Input* input);
void print_env(Env* env);
char* dummyPrint(void* toBePrinted);
void deleteProcess(void* toBeDeleted);
int dummyCompare(const void* first,const void* second);

#endif