#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define SAVE_FILE "result.txt"
#define EXT ".usp"
#define BUFFER_SIZE 1024


/**Calculates mathematical operations for +-/*, result is 0 if operator is invalid **/
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

/**Iterate through current directory and look for files ending
 * with .usp and returns the count of .usp files in the directory**/
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


/**function get_files retrieves all files with the extension .usp
 * uses the function calls and structs from dirent.h**/
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
                    printf("%s\n", ent->d_name);
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

    /**Pipe for each usp file
     * https://www.youtube.com/watch?v=8Q9CPWuRC6o&t=733s&ab_channel=CodeVault **/
    int **pipe_fds; /**Parent -> Child**/
    int **pipe_fds_2; /**Child -> Parent**/
    pipe_fds= (int**)malloc(sizeof(int*) * size);
    pipe_fds_2 = (int**)malloc(sizeof(int*) * size);
    for(int i=0;i<size;i++) {
        pipe_fds[i] = (int*)malloc(sizeof(int) * 2);
        pipe_fds_2[i] = (int*)malloc(sizeof(int) * 2);
        if(pipe(pipe_fds[i])) {
            perror("Pipe error.\n");
            exit(EXIT_FAILURE);
        }
        if(pipe(pipe_fds_2[i])) {
            perror("Pipe error.\n");
            exit(EXIT_FAILURE);
        }
    }

    int status;
    /**Creating child processes**/
    for(int i=0;i<size;i++) {
        pid = fork();

        /**Fork fail**/
        if(pid < 0) {
            perror("Fork error, failed to fork a child process\n");
            exit(EXIT_FAILURE);
        }

        /**Child process**/
        else if(pid == 0) { 
            char message[BUFFER_SIZE];
            ssize_t bytesRead =read(pipe_fds[i][0], message,sizeof(message)-1);
            if(bytesRead == -1) {
                perror("Failed to read from pipe.\n");
                exit(1);
            }

            /**add null terminate**/
            message[bytesRead] = '\0';
            printf("Child No. %d, File passed: %s\n",i, message);
            close(pipe_fds[i][0]);

            char buf;
            int fd_one;

            fd_one = open(message, O_RDONLY);
            if (fd_one == -1)
            {
                printf("Error opening file: %s\n", message);
                close(fd_one);
                exit(1);
            }

            ssize_t total_bytes_read;
            char buffer[BUFFER_SIZE];
            int num_one;
            int num_two;
            char operator;
            char reply[1024];
            char* token;
            char* id;
            total_bytes_read = read(fd_one, buffer, BUFFER_SIZE);

            if(total_bytes_read == -1) {
                perror("Error reading line");
                exit(1);
            }

            /**Get the first line using strtok, newline delimiter to split file contents**/
            token = strtok(buffer,"\n");

            strcpy(reply,token);

            token = strtok(NULL, "\n");
            num_one = atoi(token);

            token = strtok(NULL, "\n");
            operator = token[0];

            token = strtok(NULL, "\n");
            num_two = atoi(token);

            int answer = arithmatic_calc(num_one,num_two,operator);

            /**compilation error due, implicit declaration of itoa
             * alternative solution found: https://stackoverflow.com/questions/10162465/implicit-declaration-of-function-itoa-is-invalid-in-c99 **/
            char result_string[20];
            sprintf(result_string, "%d", answer);
            strcat(reply,":");
            strcat(reply,result_string);
            close(fd_one);


            /**write back to parent with results and id**/
            close(pipe_fds_2[i][0]);
            ssize_t bytes_written = write(pipe_fds_2[i][1],reply,strlen(reply));
            if (bytes_written == -1) {
                perror("Failed to write to pipe from child");
                exit(1);
            }
            close(pipe_fds_2[i][1]);
            exit(0);
        }
    }

    /**Parent process**/
    for(int i=0;i<size;i++) {
        close(pipe_fds[i][0]);
        close(pipe_fds_2[i][1]);

        char message[BUFFER_SIZE];
        strcpy(message, files_list[i]);
        ssize_t bytesWritten = write(pipe_fds[i][1],message, strlen(message));
        if(bytesWritten == -1) {
            perror("Failed to write to pipe. from parent\n");
            return 1;
        }
        close(pipe_fds[i][1]);
    }

    /**Open result.txt, append resultto file, create file if not exists**/
    int fd_write_to_file = open(SAVE_FILE, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    /**Read child writes()**/
    for(int i=0;i<size;i++) {
        close(pipe_fds_2[i][1]);
        char response[BUFFER_SIZE];
        ssize_t bytesRead = read(pipe_fds_2[i][0], response, sizeof(response) - 1);
        if (bytesRead == -1) {
            perror("Failed to read from pipe from parent");
            return 1;
        }
        response[bytesRead] = '\0';  // Manually null-terminate the received data

        // Print the received response from the child
        printf("Parent process received response from child %d: %s\n", i, response);
        write(fd_write_to_file,&response, strlen(response));

        close(pipe_fds_2[i][0]);
    }


    /**Close file and wait for children**/
    close(fd_write_to_file);
    for(int i=0;i<size;i++) {
        wait(NULL);
    }


    /**Deallocate resources for files and pipes**/
    free_list(files_list, size);
    for(int i=0;i<size;i++) {
        free(pipe_fds[i]);
        free(pipe_fds_2[i]);
    }
    free(pipe_fds);
    free(pipe_fds_2);
    return 0;
}