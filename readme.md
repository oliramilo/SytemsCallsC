# Unix Systems Programming Assignment

## Author: Olimar Ramilo 
### Student ID: 19763211

## Compilation
- make calculator
- gcc -o calculator program.c
  
## Libraries used:
- <stdlib.h>
- <dirent.h>
- <stdio.h>
- <string.h>
- <unistd.h>
- <sys/types.h>
- <sys/wait.h>
- <fcntl.h>

## Unfixed bugs and issues
- Results saves to file but the #id breaks newline and the line below is t he calculated result 

    example: 
    ```
        #00152955:
        54
    ```
    instead of:
    ```
        #00152955:54
    ```
- itoa resulting in implicit declaration, alternatively used sprintf to convert integers to string