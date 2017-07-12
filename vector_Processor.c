#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
volatile sig_atomic_t stop;

/*
Author: Vincent Ball
Assignment: CS 452 Program 2
Streamed Vector Processing
Due Date: 2-16-2017
*/

char * complementer(char str[]);
char * incrementer(char str[]);
char * adder(char str1[], char str2[]);
void sigintHandler(int sig_num);

int main(int argc, char* argv[]) {
    FILE *fin;
    FILE *InputA;
    FILE *result;

    pid_t child_a, child_b;
    int pipe_a[2],pipe_b[2];
    int stringSize;
    int numLines;
    char aline[96];
    char aline2[96];
    int counter = 0;
    char buf[128];

    printf("Enter bit size of number: ");
    /* User enters size of binary string */
    scanf("%d", &stringSize);
    printf("Enter number of lines in file: ");
    /* User enters number of lines in file */
    scanf("%d", &numLines);

    int first_pipe = pipe(pipe_a);
    int second_pipe = pipe(pipe_b);

    /*Open files */
    fin = fopen(argv[1],"r");
    InputA = fopen(argv[2],"r");
    result = fopen(argv[3],"w");

    if(first_pipe == -1 || second_pipe == -1 ){
        perror("pipe");
        exit(1);
    }

    fprintf(stderr, "Spawning process #1 and #2...\n\n");
    child_a = fork(); /* Spawn process 2*/

    signal(SIGINT, sigintHandler); /* Press CTRL C to break out of pause */

    if (child_a == 0) {
        /* Child A */
        fprintf(stderr, "Reading from complementer...\n\n");
        int a;
        for(a = 0; a < numLines; a++){
          /* Read from complementer */
          read(pipe_a[0],buf, stringSize + 1);
          char * increment;
          increment = incrementer(buf);
          /* Write output to adder */
          write(pipe_b[1],increment, strlen(increment) + 1);
        }

        /*Close file descriptors */
        close(pipe_b[1]);
        fprintf(stderr, "Sending output to adder...\n\n");
        close(pipe_a[0]);

    } else {
        fprintf(stderr, "Spawning process #3...\n\n");
        child_b = fork();  /* Create second child process */
        pause();
        if (child_b == 0) {
            /* Child B  */
            int c;
            fprintf(stderr, "Reading from incrementer...\n\n" );
            fprintf(stderr, "Reading from InputA.dat...\n\n" );
            fprintf(stderr, "Computing final result...\n\n" );

            for(c = 0; c < numLines; c++){
              /* Read from InputA.dat */
              while (fscanf(InputA, " %1023s", aline2) == 1 ){
              /* Read input from incrementer */
              read(pipe_b[0], buf,stringSize + 1);

              char * final = adder(buf, aline2);
              fprintf(stdout, "Final Output: %s\n", final);
              /* Write result to results.txt */
              fprintf(result, "%s\n", final);
            }
          }
        } else {
            //pause();
            /* Parent process */
            fprintf(stderr, "Reading from inputB.dat...\n\n");
            fprintf(stderr, "Sending output to incrementer...\n\n");
            /* Read from InputB.dat */
            while (fscanf(fin, " %1023s", aline) == 1) {
              counter++;
              char * complement;
              complement = complementer(aline); //complemenet of number
              /* Write ouput to incrementer */
              write(pipe_a[1], complement, strlen(complement)+1);
            }

            /* Close file descriptor */
            close(pipe_a[1]);
        }
    }

    return 0;
}

/*
Purpose: Take the complement of binaryString
Flip all the bits
Inputs: binaryString
Outputs: Returns complement of binaryString
*/
char * complementer(char binaryString[]){
  //strlen() to find last char of string
  int i;

  int len = strlen(binaryString);
  for(i = 0; i < len; i++){
    if(binaryString[i] == '1'){
      binaryString[i] = '0'; //flip bit
    }
    else if(binaryString[i] == '0'){
      binaryString[i] = '1'; //flip bit
    }
  }

  return binaryString;
}

/*
Purpose: Increment the binaryString by 1
Add 1
Inputs: binaryString
Outputs: Returns binaryString with 1 added
*/
char * incrementer(char binaryString[]){
  //strlen() to find last char of string
  int i;
  int leftOver = 0;
  int len = strlen(binaryString);

  /* If leftOver is 0, add the bits */
  if(leftOver == 0){
    if(binaryString[len-1] == '0'){
      binaryString[len-1] = '1';
    }
    else if(binaryString[len - 1] == '1'){
      binaryString[len-1] = '0';
      leftOver = 1;
    }
  } //leftOver = 0

  /* If leftOver is 1, add the bits and keep track of carry */
  if (leftOver == 1){
    for(i = 2; i < len + 1; i++){
      if(binaryString[len-i] == '0'){
        binaryString[len-i] = '1';
        break;
      }
      else if(binaryString[len - i] == '1'){
        binaryString[len-i] = '0';
        leftOver = 1;
      }
    }
  } //leftOver = 1

  return binaryString;
}

/*
Purpose: Added the two binaryStrings together
Add bit by bit and keep track of leftOver
Inputs: binaryString1, binaryString2
Outputs: Returns binaryString2
*/
char * adder(char binaryString1[], char binaryString2[]){

  int i;
  int leftOver = 0; //keep track of carry
  int len = strlen(binaryString1);

  for(i = 1; i < len + 1; i++){
    /* If leftOver is 0, add the bits */
    if(leftOver == 0){
      if (binaryString1[len - i] == '0' && binaryString2[len - i] == '0') {
        binaryString2[len - i] = '0';
      }
      else if (binaryString1[len - i] == '1' && binaryString2[len - i] == '0') {
        binaryString2[len - i] = '1';
      }
      else if (binaryString1[len - i] == '0' && binaryString2[len - i] == '1') {
        binaryString2[len - i] = '1';
      }
      else if (binaryString1[len - i] == '1' && binaryString2[len - i] == '1') {
        binaryString2[len - i] = '0';
        leftOver = 1;
      }
    }//LEFTOVER = 0

    /* If leftOver is 1, add the bits and keep track of carry */
    else if(leftOver == 1){
      if (binaryString1[len - i] == '0' && binaryString2[len - i] == '0') {
        binaryString2[len - i] = '1';
        leftOver = 0;
      }
      else if (binaryString1[len - i] == '1' && binaryString2[len - i] == '0') {
        binaryString2[len - i] = '0';
        leftOver = 1;
      }
      else if (binaryString1[len - i] == '0' && binaryString2[len - i] == '1') {
        binaryString2[len - i] = '0';
        leftOver = 1;
      }
      else if (binaryString1[len - i] == '1' && binaryString2[len - i] == '1') {
        binaryString2[len - i] = '1';
        leftOver = 1;
      }
    }//LEFTOVER = 1
  }

  return binaryString2;
}

/*
Purpose: Handle a CTRL C
Inputs: signal number
Outputs: None
*/
void sigintHandler(int sig_num)
{
    signal(SIGINT, sigintHandler);
    printf("Breaking out of pause, continuing program... \n");
}
