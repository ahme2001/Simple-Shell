#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void setup_environment();
void evaluate_expression();
void execute_command();
void execute_builtin();
void shell();
void on_child_exit();

char * a[100];
char * b[300] , * export_arr[50];
char input[500];
int flag = 1 , n_export=0 , background_flag = 0;
FILE * log_file;

int main() {
    signal(SIGCHLD,on_child_exit);
    setup_environment();
    shell();
   return 0;
}

void on_child_exit()
{
    int status;
    pid_t pid;
    // reap all zombie child.
    while((pid = waitpid(-1,&status,WNOHANG)) > 0){
        // do nothing
    }
    log_file = fopen("log_file.txt", "a");
    fprintf(log_file, "Child process was terminated\n");
    fclose(log_file);
}
void setup_environment()
{
    // get current directory
    chdir(getcwd(NULL,0));
}
void shell()
{
    memset(export_arr,0,100);
    do {
        memset(input,0,200); // empty array of input after every operation
        memset(a,0,100); // empty array of command after every operation
        gets(input);
        evaluate_expression();
        if(flag == 1)
        {
            execute_builtin();
        }else{
            execute_command();
        }
    } while (strcmp(input,"exit") != 0);
    exit(1);
}
void evaluate_expression()
{
    int j=0;
    for(j;j< strlen(input);j++)
    {// check "$" is found in user input or not
        if(input[j] == '$')
        {
            // if found remove it
            strcpy(&input[j],&input[j+1]);
            char temp[250]="";
            // replace substring after it by another one is stored
            // store in export array come from export command
            strcat(temp,&input[j]);
            char *piece3 = strtok(temp," ");
            char *piece2 = strtok(piece3,"\"");
            for(int q=0;q<n_export;q++)
            {
                if(strcmp(piece2,&export_arr[q])==0)
                {// replace substring after "$"
                    char * substring_source = strstr(input,piece2);
                    memmove(substring_source + strlen(&b[q]),
                            substring_source + strlen(piece2),
                             strlen(substring_source) - strlen(piece2) + 1);
                    memcpy(substring_source , &b[q], strlen(&b[q]));
                    break;
                }
            }
        }
    }
    int i=0;
    char* piece = strtok(input," ");
    a[i] = piece;
    if(strcmp(piece, "cd") == 0 || (strcmp(piece, "export") == 0) || (strcmp(piece, "echo") == 0)) {
        flag = 1;
        return;
    }else
        flag = 0;
    // split input user using space and store it in array a
    while( piece != NULL)
    {
        //piece = NULL;
        piece = strtok(NULL," ");
        if(piece == NULL)
            break;
        if(strcmp(piece,"&") == 0){
            background_flag = 1;
            return;
        }
        i++;
        a[i] = piece;
    }
}
void execute_builtin()
{
    char *piece =  a[0];
    if(strcmp( piece, "cd") == 0) {
        a[1] = strtok(NULL,"");
        if(chdir(a[1]) != 0)
            printf("error in cd");
    }else if(strcmp(piece, "export") == 0) {
        char* piece = strtok(NULL,"=");
        strcpy(&export_arr[n_export],piece);
        piece = strtok(NULL,"\"");
        strcpy(&b[n_export],piece);
        n_export++;
    }else if(strcmp(piece, "echo") == 0) {
        char* piece = strtok(NULL,"\"");
        printf("%s\n",piece);
    }
}
void execute_command() {
    pid_t child_id = fork();
    if (child_id == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
        return;
    }
    if(child_id == 0)
    {
        // child process
        if(execvp(a[0],a) == -1)
        {
            if(strcmp(input,"exit") !=0)
                printf("ERROR\n");
            exit(EXIT_FAILURE);
        }
    }else{
        // parent process
        if(background_flag == 0)
            waitpid(child_id,NULL,WUNTRACED);
    }
}
