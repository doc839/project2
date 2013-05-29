/* 
 * File:   main.c
 * Author: dennis o'connor
 *
 * Created on May 12, 2013, 4:41 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
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
    pid_t pid;
    struct cmdList *cmdPtr;
    char *string;
    char *tok;
    char **args;
    char *stdOutFile = NULL;
    char *stdInFile = NULL;
    int rdStdIn = -1;
    int rdStdOut = -1;
    int savedStdOut;
    int savedStdIn;
    int sErrorNo;
    int i;

    while (1) {

        sErrorNo = 0; // set initial error condition to 0 (No errors)  
        string = getInput(BUF_SIZE); // get user input

        if (myStrCmp(string, "exit") == 0) {
            free(string);
            exit(0);
        }

        /* set args array */
        args = malloc(CMDSIZE * sizeof (char *));
        if (args == NULL) {
            perror("malloc");
            exit(0);
        }

        /* tokenize string */
        tokenizer = init_tokenizer(string);
        i = 0;
        while ((tok = get_next_token(tokenizer)) != NULL) {
            switch (tok[0]) {
                case '<':
                    free(tok);
                    if ((tok = get_next_token(tokenizer)) != NULL) {
                        stdInFile = (char *) malloc(myStrLen(tok) * sizeof (char));
                        if (stdInFile == NULL) {
                            perror("malloc");
                            exit(0);
                        }
                        myStrCpy(stdInFile, tok);
                    }
                    break;
                case '>':
                    free(tok);
                    if ((tok = get_next_token(tokenizer)) != NULL) {
                        stdOutFile = (char *) malloc(myStrLen(tok) * sizeof (char));
                        if (stdOutFile == NULL) {
                            perror("malloc");
                            exit(0);
                        }
                        myStrCpy(stdOutFile, tok);
                    }
                    break;
                case'|':
                    addArgsList(args); // add previous set of arguments to list

                    /* set args array */
                    args = malloc(CMDSIZE * sizeof (char *));
                    if (args == NULL) {
                        perror("malloc");
                        exit(0);
                    }
                    i = 0; // reset index for args list
                    break;
                case '&':
                    printf("Got &\n");
                    break;
                default:
                    args[i] = (char *) malloc(myStrLen(tok) * sizeof (char));
                    if (args[i] == NULL) {
                        perror("malloc");
                        exit(0);
                    }
                    myStrCpy(args[i], tok);
                    i++;
                    args[i] = NULL;
                    break;
            }

            free(tok); /* free the token now that we're done with it */
        }
        free_tokenizer(tokenizer); /* free memory */
        free(string);

        addArgsList(args);
        cmdPtr = cmdsHead;

        // Setup redirection of stdin and stdout
        if (stdInFile != NULL) {
            rdStdIn = open(stdInFile, O_RDONLY);
            if (rdStdIn == -1) {
                sErrorNo = shellError(1);
            }
        }
        
        if (stdOutFile != NULL) {
            rdStdOut = open(stdOutFile, O_WRONLY | O_CREAT, 0644);
            if (rdStdOut == -1) {
                sErrorNo = shellError(2);
            }
        }

        if (sErrorNo == 0 && stdOutFile != NULL) {
            savedStdOut = dup(1);  /* save stdout */
            dup2(rdStdOut, STDOUT_FILENO);
            close(rdStdOut);
            myPipes(cmdPtr, rdStdIn);
            dup2(savedStdOut, 1);
            close(savedStdOut);  /* restore stdout */
        } 
        else if (sErrorNo == 0 && stdOutFile == NULL) {
            myPipes(cmdPtr, rdStdIn);
        }

        /* free memory for args*/
        cmdPtr = cmdsHead;
        while (cmdPtr != NULL) {
            for (int j = 0; cmdPtr->args[j] != NULL; j++) {
                free(cmdPtr->args[j]);
            }
            cmdPtr = cmdPtr->next;
        }

        cmdsHead = cmdsCurr = NULL;

        /* free standard in and out file names */
        if (stdInFile != NULL) {
            free(stdInFile);
            stdInFile = NULL;
            rdStdIn = -1;
        }
        if (stdOutFile != NULL) {
            free(stdOutFile);
            stdOutFile = NULL;
            rdStdOut = -1;
        }
        
        jobPrint();
        
        while (jobHead != NULL)
            jobDelete(jobHead->job);
    }

    printf("\nBye!\n");
    return 0;
}

