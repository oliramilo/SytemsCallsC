#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

char* EXT = ".usp";

int arithmatic_calc(int num1, int num2, char op) {
    int result;
    switch(op) {
        case '+':
            result = num1 + num2;
            break;
        case '-':
            result = num1 - num2;
            break;
        case 'x':
        case 'X':
        case '*':
            result = num1 * num2;
            break;
        case '/':
            result = num1 / num2;
            break;
        default:
            printf("Invalid operator\n");
            result = 0;
            break;
    }
    return result;
}

/**Iterate through current directory and look for files ending with .usp**/
int find_files() {
    DIR * dir;
    struct dirent *ent;
    int num_files = 0;
    /**Read current directory**/
    if ((dir = opendir(".")) != NULL) { // "." represents the current directory
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) { // check if the entry is a regular file
                 if (strstr(ent->d_name, EXT) != NULL && strcmp(strstr(ent->d_name, EXT), EXT) == 0) {
                    num_files++;
                }
                
            }
        }
        closedir(dir);
    } else {
        perror("Failed to open directory");
        return -1;
    }
    return num_files;
}

char** get_files(int size) {
    DIR * dir;
    struct dirent *ent;
    int ptr = 0;
    char **files_list= (char**)malloc(size*sizeof(char*));
    /**Read current directory and store each filename in our files_list array**/
    if ((dir = opendir(".")) != NULL) { 
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) { /**Check for file type**/
                 if (strstr(ent->d_name, EXT) != NULL && strcmp(strstr(ent->d_name, EXT), EXT) == 0) {
                    files_list[ptr] = (char*)malloc(sizeof(char)*strlen(ent->d_name));
                    strcpy(files_list[ptr], ent->d_name);
                    ptr++;
                }
            }
        }
        closedir(dir);
    } else {
        perror("Failed to open directory");
        return NULL;
    }
    return files_list;
}

/**Free the allocated memory for each files ending with .usp**/
void free_list(char** list,int size) {
    for(int i=size-1;i==0;i--) {
        free(list[i]);
    }
    free(list);
}

int main() {
    int size = find_files();
    pid_t pid;
    char** files_list;
    if(size == -1) {
        printf("An error occured while looking for files\n");
        return 0;
    }
    files_list = get_files(size);
    if(files_list == NULL) {
        printf("An error occurred while obtaining file names.");
        return 0;
    }

    /**Pipe for each usp file**/
    int **pipe_fds;
    pipe_fds= (int**)malloc(sizeof(int*) * size);
    for(int i=0;i<size;i++) {
        pipe_fds[i] = (int*)malloc(sizeof(int) * 2);
        if(pipe(pipe_fds[i]) == -1) {
            perror("Pipe error.\n");
            exit(EXIT_FAILURE);
        }
    }

    /**Creating child processes**/
    for(int i=0;i<size;i++) {
        pid = fork();
        if(pid < 0) {
            perror("Fork error, failed to fork a child process\n");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0) {
            close(pipe_fds[i][0]);   
        }
    }

    for(int i=0;i<size;i++) {
        wait(NULL);
    }

    free_list(files_list, size);
    printf("Successfully reached end of program.\n");
    return 0;
}