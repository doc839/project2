#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "utils.h"
#include "global.h"
#include "jobs.h"

#define DIGITS 12


/*
 * Function returns integer or 
 *     -1 if not an number
 *      0 if number of digits greater that 4
 * 
 * Created my own atoi functionality (5 extra credit points)
 */
 int myAtoi( char *s )
 {
     int i = 0;        // number to be returned
     int c = 0;        // count of digits in number
     
     while( (*s) ) {
         
         c++;    // increment count of digits
         
        if ( c > 5 )
            return 0;  //  exceeded limit of digits
         
         i = i * 10 ;    // shift digits to the left
         
         if ( (*s) > 57 || (*s) < 48)
            return -1;  // not a positive integer
            
        i = i + ( (*s) - 48 );

        s++;
    }
    
    return i;
}

 /*
  * Got this from:
  * http://www.opensource.apple.com/source/groff/groff-11/groff/libgroff/itoa.c
  */

char *myItoa( int i)
{
  static char buf[DIGITS + 2];
  char *p = buf + DIGITS + 1;	
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {			
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
} 

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
    
    string = (char *) malloc(bufSize);
    if (string == NULL) {
        perror("string init");
        exit(1);
    }
    
    string[0] = '\0';
    write( STDOUT_FILENO, "gort-> ", 7);  /* output prompt */
    while ((br = read( STDIN_FILENO, buf, bufSize )) > 0) {
      if (br <= 1 && myStrLen(string) == 0) {
          write( STDOUT_FILENO, "gort-> ", 7);  /* output prompt */
          continue;
      }
      else if (br == bufSize && buf[br-1] != '\n') {
          tmp = (char *) malloc(bufSize + myStrLen(string));
          if (tmp == NULL) {
              perror("malloc");
              exit(0);
          } 
          else {
              myStrCpy(tmp, string);
          }
          
          for ( i = 0 ,j = myStrLen(string); i < bufSize; i++, j++) {
              tmp[j] = buf[i];
          }
          tmp[j] = '\0';
          free(string);
          string = tmp;
          continue;
      } else {
          buf[br-1] = '\0';
          tmp = (char *) malloc(br+myStrLen(string));
          if (tmp == NULL) {
              perror("malloc");
              exit(0);
          }
          else {
              myStrCpy(tmp, string);
          }
          myStrCat(tmp, buf);
      }
      free(string);
      string = tmp;
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
        case 3 :
            myStrCpy(error, "Invalid use of redirection...");
            break;
        case 10 :
            return 0;
            break;
        default :
            myStrCpy(error, "Invalid error code\n") ;
            break;
    }
    
    write( STDOUT_FILENO, error, myStrLen(error));
    
    return choice;
}