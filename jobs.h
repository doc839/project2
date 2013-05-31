/* 
 * File:   jobs.h
 * Author: dennis
 *
 * Created on May 30, 2013, 9:48 AM
 */

#ifndef JOBS_H
#define	JOBS_H

/*
 * Structure for simple job queue
 */
struct jobList
{
    int id;
    pid_t pid;
    char *name;
    int status;
    
    struct jobList *next;
    struct jobList *prev;
};

struct jobList *jobHead;
struct jobList *jobTail;

void jobAdd( pid_t pid, char *name);
void createJobsList(  pid_t pid, char *name);
struct jobList *jobFind( pid_t pid );
int jobDelete( pid_t pid );
void jobPrint();
void jobKill( char *s);

void jobPipes(struct cmdList *cmd, int input, pid_t gprd, int fg);

#endif	/* JOBS_H */

