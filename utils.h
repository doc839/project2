/* 
 * File:   utils.h
 * Author: dennis
 *
 * Created on May 19, 2013, 11:27 AM
 */

#ifndef UTILS_H
#define	UTILS_H

#include <stdio.h>
#include <stdlib.h>
    
int myStrLen(char *str);
char *myStrCat( char *d, const char *s);
char *myStrCpy(char *d, const char *s);
int myStrCmp(const char *d, const char *s);
char *getInput(int bufSize);


#endif	/* UTILS_H */

