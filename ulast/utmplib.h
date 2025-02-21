#include	<stdio.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<utmp.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include    <string.h>
#include    <sys/stat.h>

/**********************************************************
 *    utmplib.c                                           *
 *    Contains functions to buffer reads from utmp file   * 
 **********************************************************/

int utmp_open(char *filename);
int utmp_len();
void utmp_close();
struct utmp* utmp_getrec(int index);
void utmp_stats(int a[2]);