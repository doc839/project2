/* 
 * File:   main.c
 * Author: dennis o'connor
 *
 * Created on May 12, 2013, 4:41 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tokenizer.h"
#include "utils.h"

#define BUF_SIZE 5
#define CMDSIZE 20

pid_t childPID = -1;

/*
 * 
 */
int main(int argc, char** argv) {

  TOKENIZER *tokenizer;
  char *string;
  char *tok;
  char **args;
  char *stdOutFile = NULL;
  char *stdInFile = NULL;
  int i;

  while(1) {
        
    string = getInput(BUF_SIZE);
    
    if(myStrCmp(string, "exit") == 0) {
        free(string);
        exit(0);
    }
    
    /* set args array */
    args = malloc(CMDSIZE * sizeof(char*));
    if (args == NULL) {
        perror("malloc");
        exit(0);
    }
    
    /* tokenize string */
    tokenizer = init_tokenizer( string );
    i = 0;
    while( (tok = get_next_token( tokenizer )) != NULL ) {
        switch(tok[0]) {
            case '<':
                free(tok);
                if ( (tok = get_next_token( tokenizer )) != NULL ) {
                    stdInFile = (char *) malloc(myStrLen(tok) * sizeof(char));
                    if (stdInFile == NULL) {
                        perror("malloc");
                        exit(0);
                    }
                    myStrCpy(stdInFile, tok);
                }
                break;
            case '>' :
                free(tok);
                if ( (tok = get_next_token( tokenizer )) != NULL ) {
                    stdOutFile = (char *) malloc(myStrLen(tok) * sizeof(char));
                    if (stdOutFile == NULL) {
                        perror("malloc");
                        exit(0);
                    }
                    myStrCpy(stdOutFile, tok);
                }
                break;
            case'|':
                printf("Got |\n");
                break;
            case '&':
                printf("Got &\n");
                break;
            default:
                args[i] = (char *) malloc(myStrLen(tok) * sizeof(char));
                if (args[i] == NULL) {
                    perror("malloc");
                    exit(0);
                }
                myStrCpy(args[i], tok);
                i++;
                args[i] = NULL;
                break;
        }
     
      free( tok );    /* free the token now that we're done with it */
    }
    free_tokenizer( tokenizer ); /* free memory */
    free(string);
    
    switch(childPID = fork()) {
        case -1:
            perror("fork");
            exit(0);
        case 0:
             execvp(args[0], args);
             perror("execvp");
             exit(0);
             break;
        default:
            wait();
            break;
    }
    
    /* free memory for args*/
    for (int j = 0; args[j] != NULL; j++)
        free(args[j]);
    
    free(args);
    
    printf("in = %s \n", stdInFile);
    printf("out = %s \n", stdOutFile);
    
    /* free standard in and out file names */
    if (stdInFile != NULL) {
        free(stdInFile);
        stdInFile = NULL;
    }
    if (stdOutFile != NULL) {
        free(stdOutFile);
        stdOutFile = NULL;
    }
  }

  printf( "\nBye!\n" );
  return 0;			/* all's well that end's well */
}

