#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    
    while(*d++ = *s++);
    
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
    if (string == NULL) 
        perror("string init");
    
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
          tmp = (char *) realloc(string, myStrLen(buf)+myStrLen(string));
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

char **setUpCmd( char **arg, char *s) {
    char **a = *arg;
    while(*a != NULL) *a++;
    
    a = (char *) malloc(myStrLen(s));
    if (a == NULL) {
        perror("malloc");
        exit(0);
    }
    
    return a;
}

