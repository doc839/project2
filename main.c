/* 
 * File:   main.c
 * Author: dennis o'connor
 *
 * Created on May 12, 2013, 4:41 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tokenizer.h"
#include "utils.h"
#include "global.h"
#include "jobs.h"

#define BUF_SIZE 5
#define CMDSIZE 20

pid_t childPID = -1;
pid_t rtPid;

void sigHandler( int signal ) {
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
    
    if (signal == SIGCHLD ) {
        
        if (WIFEXITED(status)) {
            tcsetpgrp( 0, myShellGpid);
            if (pid == -1) {
                jobDelete(fgPid);
            }
            else {
                jobStatus(pid, 0);
                jobPrint();
                write( 1, "gort-> ", 7);
            }
        }
        else if (WIFSIGNALED(status)) {
            tcsetpgrp( 0, myShellGpid);
            if (pid == -1) {
                jobDelete(fgPid);
            }
            else {
                jobStatus(pid, 0);
                jobPrint();
                write( 1, "gort-> ", 7);
            }
            
        }
        else if (WIFSTOPPED(status)) {
            tcsetpgrp( 0, myShellGpid);
            jobStatus( pid, 2);
            jobPrint();
            write( 1, "gort-> ", 7);
        }
    }
    
    if (signal == SIGTSTP) {
        killpg( fgPid, SIGTSTP);
    }
    
    if (signal == SIGINT) {
        killpg( fgPid, SIGINT);
        jobStatus(fgPid, 0);
        jobPrint();
        write( 1, "gort-> ", 7);
    }
}
/*
 * 
 */
int main(int argc, char** argv) {

    TOKENIZER *tokenizer;    
    pid_t gpid;
    struct cmdList *cmdPtr;
    char *string;
    char *tok;
    char **args;
    char *stdOutFile = NULL;
    char *stdInFile = NULL;
    int rdStdIn = -1;
    int rdStdOut = -1;
    int savedStdOut;
    int sErrorNo;
    int piped;   // 0 if no pipeline, 1 if there is a pipeline
    int i;
    int fg;
        
    signal( SIGCHLD, sigHandler);
    signal( SIGINT, sigHandler);
    signal( SIGTSTP, sigHandler);
    
    // set up shell
    // set the shell process group
    myShellGpid = getpgrp();
    jobAdd(myShellGpid, "shell");

    while (1) {

        sErrorNo = 0; // set initial error condition to 0 (No errors)  
        piped = 0;   // set intial pipe status to 0
        fg = 1;       // set initial state for foreground processing;
        
        string = getInput(BUF_SIZE); // get user input

        if (myStrCmp(string, "exit") == 0) {
            free(string);
            exit(0);
        }
        
        if (myStrCmp(string, "jobs") == 0) {
            jobPrint();
            continue;
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
            if (fg == 0) fg = 1;  // & was not at the end of the command
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
                    if (piped > 0) sErrorNo = 3; // illegal redirect
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
                    
                    if (stdOutFile != NULL) sErrorNo = 3;
                    piped = 1;
                    break;
                case '&':
                    fg = 0;
                    break;
                default:
                    if (myStrCmp(tok, "kill") == 0) {
                        free(tok);
                        if ((tok = get_next_token(tokenizer)) != NULL) {
                            jobKill(tok);
                        }
                        sErrorNo = 10;
                    }
                    else {
                        args[i] = (char *) malloc(myStrLen(tok) * sizeof (char));
                        if (args[i] == NULL) {
                            perror("malloc");
                            exit(0);
                        }
                        myStrCpy(args[i], tok);
                        i++;
                        args[i] = NULL;
                    }
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
                sErrorNo = 1;  // error openning file for read
            }
        }
        
        if (stdOutFile != NULL && sErrorNo == 0) {
            rdStdOut = open(stdOutFile, O_WRONLY | O_CREAT, 0644);
            if (rdStdOut == -1) {
                sErrorNo = 2;  // error openning file for write
            }
        }
        
        // initialize group process id to 0
        gpid = 0;

        if (sErrorNo == 0 && stdOutFile != NULL) {
            savedStdOut = dup(1);  /* save stdout */
            dup2(rdStdOut, STDOUT_FILENO);
            close(rdStdOut);
            jobPipes(cmdPtr, rdStdIn, gpid, fg);
            dup2(savedStdOut, 1);
            close(savedStdOut);  /* restore stdout */
        } 
        else if (sErrorNo == 0 && stdOutFile == NULL) {
            jobPipes(cmdPtr, rdStdIn, gpid, fg);
        }
        
        /*
         * Print error message if sErrorNo greater than 0 
         */
        if (sErrorNo > 0)
            shellError(sErrorNo);
        
        
        
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
    }
    return 0;
}

