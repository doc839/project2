#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "utils.h"

int myStrLen(char *str) {
    unsigned long count = 0;
    
    while (*str++) count++; 
    
    return count;
}

char *myStrCat(char *d, const char *s) {
    char *r = d;
    
    if (d == NULL || s == NULL) 
        return d;
    
    while (*d) 
        d++;
    while (*d++ = *s++)
        ;
    
    return r;
}

char *myStrCpy(char *d, const char *s) {
    char *r = d;
    
    while(*d++ = *s++)
        ;
    
    return r;
}

int myStrCmp(const char *d, const char *s) {
    
    while( *d == *s) {
        if (*s == '\0')
            return 0;
        *d++;
        *s++;
    }
    
    return(*d - *s);
}

char *getInput(int bufSize) {
    char *string, *tmp;
    char buf[bufSize];
    int br, i, j;
    
    string = (char *) malloc(1);
    if (string == NULL) {
        perror("string init");
        exit(1);
    }
    
    string[0] = '\0';
    write( STDOUT_FILENO, "gort-> ", 7);  /* output prompt */
    while ((br = read( STDIN_FILENO, buf, bufSize )) > 0) {
      if (br <= 1 && myStrLen(string) == 0)
        continue;
      else if (br == bufSize && buf[br-1] != '\n') {
          tmp = (char *) realloc(string, bufSize + myStrLen(string));
          if (tmp == NULL) {
              perror("realloc");
              exit(0);
          } 
          else {
              string = tmp;
          }
          
          for ( i = 0 ,j = myStrLen(string); i < bufSize; i++, j++) {
              string[j] = buf[i];
          }
          string[j] = '\0';
          continue;
      } else {
          buf[br-1] = '\0';
          tmp = (char *) realloc(string, br+myStrLen(string));
          if (tmp == NULL) {
              perror("realloc");
              exit(0);
          }
          else {
              string = tmp;
          }
          myStrCat(string, buf);
      }
      return string;
    }
    return string;
}

/*
 * Routine for creating linked list of arguments for a pipe
 * command
 */
void createArgsList(char **args) {
    
    struct cmdList *ptr = (struct cmdList *) malloc(sizeof(struct cmdList *));
    if (ptr == NULL) {
        perror("malloc");
        exit(1);
    }
        
    ptr->args = args;
    ptr->next = NULL;
    
    cmdsHead = cmdsCurr = ptr;
}

/*
 * Routine for adding a new list of arguments to the linked list of
 * arguments
 */
void addArgsList(char **args) {
    
    // see if list has been created
    if (cmdsHead == NULL) {
        createArgsList(args);
        return;
    }
    
    struct cmdList *ptr = (struct cmdList *) malloc( sizeof( struct cmdList * ));
    if ( ptr == NULL) {
        perror("malloc");
        exit(1);
    }
    
    ptr->args = args;
    ptr->next = NULL;
    
    cmdsCurr->next = ptr;
    cmdsCurr = ptr;
}


/*
 * Routine for creating linked list of arguments for a pipe
 * command
 */
void createJobsList(int job) {
    
    struct jobList *ptr = (struct jobList *) malloc(sizeof(struct jobList *));
    if (ptr == NULL) {
        perror("malloc");
        exit(1);
    }
        
    ptr->job = job;
    ptr->next = NULL;
    ptr->prev = NULL;
    
    jobHead = jobTail = ptr;
}

/*
 * Routine for adding a new job to the linked list job queue
 */
void addJob(int job) {
    
    // see if list has been created
    if (jobHead == NULL) {
        createJobsList(job);
        return;
    }
    
    struct jobList *ptr = (struct jobList *) malloc( sizeof( struct jobList * ));
    if ( ptr == NULL) {
        perror("malloc");
        exit(1);
    }
    
    ptr->job = job;
    ptr->next = NULL;
    
    jobTail->next = ptr;
    ptr->prev = jobTail;
    jobTail = ptr;
}

/*
 * Find a job in the job queue, return NULL if not found
 */
struct jobList *jobFind( int job ) {
    struct jobList *ptr = jobHead;
    
    while (ptr != NULL) {
        if (ptr->job == job) {
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;
}

/*
 * Deletes job from job queue
 */
int jobDelete( int job ) {
    struct jobList *ptr;
    
    if ((ptr = jobFind(job)) == NULL) {
        return -1;
    }
    
    if (ptr->prev == NULL && ptr->next == NULL) { /* last member of list */
        jobHead = jobTail = NULL;
        free(ptr);
        return 1;
    }
    
    if (ptr->prev == NULL) {  /* ptr is pointing to head of list */
        ptr->next->prev == NULL;
        jobHead = ptr->next;
        free(ptr);
        return 1;
    }
    
    if (ptr->next == NULL) {   /* ptr is pointing to tail of list */
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
 *  Recursive fork  
 */
void myPipes(struct cmdList *cmd, int in) {
  int pf[2] = { -1, -1 };
  int status;
  pid_t pid;
  
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
  }
  else { /* parent */
    if (in != -1) {
      close(in);
    }
    if (pf[1] != -1) {
      close(pf[1]);
    }
    wait(&status);
    if (cmd->next != NULL) {
      myPipes(cmd->next, pf[0]);
    }
  }
}

/*
 *  Write error messages to stdout
 */
int shellError( int choice ) {
    char error[100];
    
    switch(choice) {
        case 1 :
            myStrCpy(error, "Error opening file for read...\n");
            break;
        case 2 :
            myStrCpy(error, "Error opening file for write...\n");
            break;
        default :
            myStrCpy(error, "Invalid error code\n") ;
            break;
    }
    
    write( STDOUT_FILENO, error, myStrLen(error));
    
    return choice;
}