/* 
 * File:   utils.h
 * Author: dennis o'connor
 *
 * Created on May 19, 2013, 11:27 AM
 */

#ifndef UTILS_H
#define	UTILS_H

#include <stdio.h>
#include <stdlib.h>

/*
 * Created my own string functions since we can't use any of
 * the standard c library string functions
 */
int myAtoi(char *s);
int myStrLen(char *str);
char *myStrCat( char *d, const char *s);
char *myStrCpy(char *d, const char *s);
int myStrCmp(const char *d, const char *s);
char *getInput(int bufSize);

/*
 * Structure for handling multiple commands generated by
 * a pipe command
 */
struct cmdList
{
    char **args;
    struct cmdList *next;
};

struct cmdList *cmdsHead;
struct cmdList *cmdsCurr;

void createArgsList( char **args);
void addArgsList( char **args);

int shellError( int choice );


#endif	/* UTILS_H */

