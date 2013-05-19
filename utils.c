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
    char *string;
    char buf[bufSize];
    int br, i, j;

    string = (char *) malloc(1);
    string[0] = '\0';
    int m = myStrLen(string);
    write( STDOUT_FILENO, "gort-> ", 7);
    while ((br = read( STDIN_FILENO, buf, bufSize )) > 0) {
      if (br <= 1 && myStrLen(string) == 0)
        continue;
      else if (br == bufSize && buf[br-1] != '\n') {
          string = (char *) realloc(string, bufSize + myStrLen(string));
          for ( i = 0 ,j = myStrLen(string); i < bufSize; i++, j++) {
              string[j] = buf[i];
          }
          string[j] = '\0';
          continue;
      } else {
          buf[br-1] = '\0';
          string = (char *) realloc(string, myStrLen(buf)+myStrLen(string));
          myStrCat(string, buf);
      }
      return string;
    }
    return string;
}
