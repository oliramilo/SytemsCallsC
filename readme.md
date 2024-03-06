# Calculator Programme using System Calls in C
[Unix Systems Programming Assignment](https://www.curtin.edu.au/study/offering/unit-ug-unix-systems-programming--comp2002/)

## Author: Olimar Ramilo 

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

Program reads all .usp files within current directory
.usp files contains contents in the following format:
    
        .usp Format:
          File ID
          
          Number
          
          Operator
          
          Number
        
        Results in:
          #00152955:54
