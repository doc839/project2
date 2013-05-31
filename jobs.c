#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "utils.h"
#include "global.h"
#include "jobs.h"

/*
 * Routine for creating linked list of arguments for a pipe
 * command
 */
void createJobsList(pid_t pid, char *name) {

    char *tmp;

    struct jobList *ptr = (struct jobList *) malloc(sizeof (struct jobList));
    if (ptr == NULL) {
        perror("malloc");
        exit(1);
    }

    ptr->id = 0; // first job in list is the shell
    ptr->pid = pid;

    tmp = malloc(myStrLen(name));
    if (tmp == NULL) {
        perror("malloc");
        exit(1);
    }

    ptr->name = myStrCpy(tmp, name);
    ptr->status = -1;

    ptr->next = NULL;
    ptr->prev = NULL;

    jobHead = jobTail = ptr;
}

/*
 * Routine for adding a new job to the linked list job queue
 */
void jobAdd(pid_t pid, char *name) {

    // see if list has been created
    if (jobHead == NULL) {
        createJobsList(pid, name);
        return;
    }

    if (jobFind(pid) == NULL) { /* only add new job if doesn't already exist */

        struct jobList *ptr = (struct jobList *) malloc(sizeof ( struct jobList));
        if (ptr == NULL) {
            perror("malloc");
            exit(1);
        }

        ptr->id = jobTail->id + 1;
        ptr->pid = pid;

        char *tmp = (char *) malloc(myStrLen(name) * sizeof (char));
        if (tmp == NULL) {
            perror("malloc");
            exit(1);
        }

        ptr->name = myStrCpy(tmp, name);
        ptr->status = 1; /* runninig */

        ptr->next = NULL;

        jobTail->next = ptr;
        ptr->prev = jobTail;
        jobTail = ptr;
    }
}

/*
 * Find a job in the job queue, return NULL if not found
 */
struct jobList *jobFind(pid_t pid) {
    struct jobList *ptr = jobHead;

    while (ptr != NULL) {
        if (ptr->pid == pid) {
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;
}

/*
 * Deletes job from job queue
 */
int jobDelete(pid_t pid) {
    struct jobList *ptr;

    if ((ptr = jobFind(pid)) == NULL) {
        return 0;
    }

    free(ptr->name);

    if (jobHead == jobTail) { /* last member of list */
        free(ptr);
        jobHead = jobTail = NULL;
        return 1;
    }

    if (ptr->prev == NULL) { /* ptr is pointing to head of list */
        ptr->next->prev = NULL;
        jobHead = ptr->next;
        free(ptr);
        return 1;
    }

    if (ptr->next == NULL) { /* ptr is pointing to tail of list */
        ptr->prev->next = NULL;
        jobTail = ptr->prev;
        free(ptr);
        return 1;
    }

    ptr->prev->next = ptr->next;
    ptr->next->prev = ptr->prev;
    free(ptr);
    return 1;
}

/*
 * Prints list of jobs
 */
void jobPrint() {
    static char buf[60];
    struct jobList *ptr; 
    struct jobList *del;
    char *fb = "[";
    char *rb = "]";
    char *string = buf;

    ptr = jobHead->next;

    while (ptr != NULL) {
        myStrCpy(string, "\n");
        myStrCat(string, fb);
        myStrCat(string, myItoa(ptr->id));
        myStrCat(string, rb);
        myStrCat(string, "  ");
        myStrCat(string, myItoa(ptr->pid));
        
        switch(ptr->status) {
            case 0 :
                myStrCat(string, " - Done");
                break;
            case 1 :
                myStrCat(string, " - Running");
                break;
            case 2 :
                myStrCat(string, " - Stopped");
                break;
            default :
                myStrCat(string, " - No Status");
                break;      
        }
       
        write(1, string, myStrLen(string));
        ptr = ptr->next;
    }
    
    write(1, "\n", 1);
    
    ptr = jobHead->next;
    del = NULL;
    while(ptr != NULL) {
        if ( ptr->status == 0) {
            del = ptr;
        }
        ptr = ptr->next;
        if (del != NULL) {
            jobDelete(del->pid);
            del = NULL;
        }
    }
}

/*
 *  Updates status in job queue
 */
void jobStatus(pid_t pid, int status) {
    struct jobList *ptr;

    if ((ptr = jobFind(pid)) != NULL) {
        ptr->status = status;
    }
}

/*
 *  Kill process either by id or pid
 */
void jobKill( char *s ) {
    struct jobList *ptr;
    char *p;
    int i;
    
    if (*s == '%') {
        i = myAtoi( ++s );
        
        ptr = jobHead;
        while(ptr != NULL) {
            if (i == ptr->id) {
                killpg(ptr->pid, SIGINT);
                return;
            }
            ptr = ptr->next;
        }
    }
    else {
        i = myAtoi( s );
        killpg(i, SIGINT);
        return;
    }
}

/*
 *  Recursive fork  
 */
void jobPipes(struct cmdList *cmd, int in, pid_t gpid, int fg) {
    int pf[2] = {-1, -1};
    pid_t pid;
    int status;

    if (cmd->next != NULL) {
        pipe(pf);
    }

    if ((pid = fork()) == 0) { /* child */
        
        if (in != -1) {
            dup2(in, STDIN_FILENO);
            close(in);
        }

        if (pf[1] != -1) {
            dup2(pf[1], STDOUT_FILENO);
            close(pf[1]);
        }

        if (pf[0] != -1) {
            close(pf[0]);
        }
        
        execvp(cmd->args[0], cmd->args);
        exit(1);
    } else { /* parent */

        if (in != -1) {
            close(in);
        }
        if (pf[1] != -1) {
            close(pf[1]);
        }

        jobAdd(pid, cmd->args[0]);       

        if (setpgid(pid, pid) < 0) {
            perror("setpgid 1");
            exit(1);
        }
                
        fgPid = 0;
        if (fg) {
            fgPid = pid;
            wait(&status);
            jobDelete(pid);
        }
        //tcsetpgrp( 0, getpid());
        
        if (cmd->next != NULL) {
            jobPipes(cmd->next, pf[0], gpid, fg);
        }
    }
}
