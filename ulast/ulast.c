#include	<stdio.h>
#include	<unistd.h>
#include	<sys/types.h>
#include    <linux/limits.h>
#include	<utmp.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include    <string.h>
#include    "utmplib.h"

/****************************************************
 *   ulast.c                                        *
 * - Reads login info file in reverse order, record *
 *   by record, and prints the info to the screen.  *                                           
 ****************************************************/
/* UTMP_FILE is a symbol defined in utmp.h */
/* note: compile with -DTEXTDATE for dates format: Feb  5, 1978 	*/
/*       otherwise, program defaults to NUMDATE (1978-02-05)		*/

#define TEXTDATE
#define	MAXDATELEN	100
#ifndef	DATE_FMT
    #ifdef	TEXTDATE
        #define	DATE_FMT	"%a %b %e %H:%M"  /* text format */
    #else
        #define	DATE_FMT	"%Y-%m-%d %H:%M"  /* the default */
    #endif
#endif
void showtime( time_t , char * );
void show_info( struct utmp *);
void read_file(int, struct utmp *, char *);
void appendLogout(struct utmp, int, int);
void printLogout(time_t login, time_t logout);
int checkArgs(int ac, char*av[], char info[], char usrName[]);

int main(int ac, char *av[])
{
	struct utmp	utbuf;                                 // read info into here
	int	   utmpfd;		                               // read from this fd 
	char  info[PATH_MAX], usrName[256];                // filename, user name
    int   showStats = 0;//, hasFilename=0, hasUsrName=0;  // flags for user input
    int a[2];                                          // array for stats
  
    /*if (ac < 2 || ac > 5 ) {                           // Check args
        fprintf(stderr, "Usage: ./ulast USER [-f FILENAME]\n");
        exit(-1);
    }
    if (ac == 2) {                                     // User name is only arg
        usrName = av[1];
        info = UTMP_FILE;
        hasUsrName = 1;
        hasFilename = 1;
    }
    else {
        for (int i=1; i<ac; i++) {
            if (strcmp(av[i], "-f")==0 && i<ac-1) {    // Look for file arg
                info = av[++i];                        // Next arg is filename
                hasFilename = 1;
            }
            else if (strcmp(av[i], "-e")==0) {         // Stats arg
                showStats = 1;
            }
            else {
                usrName = av[i];                       // Arg is user name
                hasUsrName = 1;
            }
        }
    }
    if (!hasFilename || !hasUsrName || (ac==5 && !showStats)) {
        fprintf(stderr, "Usage: ./ulast USER [-f FILENAME]\n");
        exit(-1);        
    }*/
    showStats = checkArgs(ac, av, info, usrName);

	if ( (utmpfd = utmp_open(info)) == -1 ){
		fprintf(stderr,"%s: cannot open %s\n", *av, info );
		exit(1);
	}
    read_file(utmpfd, &utbuf, usrName);
    if(showStats) {
        utmp_stats(a);
        fprintf(stderr, "%d records read, %d buffer misses\n", a[0], a[1]);
    }
 	utmp_close();
	return 0;
}

void read_file(int utmpfd, struct utmp *utbuf, char *usrName)
{
    int numRecs, currentRec;
    numRecs = currentRec = utmp_len();                 // Set to last entry

    if (numRecs < 1) {                                 // At least 1 entry?
        printf("Error reading file");
        exit(-1);
    }

    // Start reading file from end to beginning
	while (--currentRec >= 0) {
        utbuf = utmp_getrec(currentRec);
        if (strncmp(utbuf->ut_name, usrName, UT_NAMESIZE) == 0 && 
                    utbuf->ut_type==USER_PROCESS) {
            show_info( utbuf );
            appendLogout(*utbuf, currentRec, numRecs);
        }
    }
}

int checkArgs(int ac, char*av[], char info[], char usrName[])
{
    int   showStats = 0, hasFilename=0, hasUsrName=0;  // flags for user input
  
    if (ac < 2 || ac > 5 ) {                           // Check args
        fprintf(stderr, "Usage: ./ulast USER [-f FILENAME]\n");
        exit(-1);
    }
    if (ac == 2) {                                     // User name is only arg
        strcpy(usrName, av[1]);
        strcpy(info, UTMP_FILE);
        hasUsrName = 1;
        hasFilename = 1;
    }
    else {
        for (int i=1; i<ac; i++) {
            if (strcmp(av[i], "-f")==0 && i<ac-1) {    // Look for file arg
                strcpy(info, av[++i]);                 // Next arg is filename
                hasFilename = 1;
            }
            else if (strcmp(av[i], "-e")==0) {         // Stats arg
                showStats = 1;
            }
            else {
                strcpy(usrName, av[i]);                       // Arg is user name
                hasUsrName = 1;
            }
        }
    }
    if (!hasFilename) {
        strcpy(info, UTMP_FILE);
    }
    if (!hasUsrName || (ac==5 && !showStats)) {
        fprintf(stderr, "Usage: ./ulast USER [-f FILENAME]\n");
        exit(-1);        
    }
    return showStats;
}


void appendLogout(struct utmp utbuf, int currentRec, int numRecs)
{
    struct utmp utp;

    while (++currentRec < numRecs) {
        utp = *utmp_getrec(currentRec);
        if (utp.ut_pid == utbuf.ut_pid || utp.ut_type==BOOT_TIME) {
            printf(" - ");
            if (utp.ut_type==BOOT_TIME) {
                printf("crash");
            }
            else {
                showtime(utp.ut_time, "%H:%M");
            }
            printLogout(utbuf.ut_time, utp.ut_time);

            return;
        }
    }
    printf("   still logged in");
	printf("\n");					/* newline	*/
}

/*
 *	show info() (!!Borrowed from class example!!)
 *			displays the contents of the utmp struct
 *			in human readable form
 *			* displays nothing if record has no user name
 */
void show_info( struct utmp *utbufp )
{
	printf("%-8.*s",UT_NAMESIZE,utbufp->ut_name);		/* the logname	*/
	printf(" ");					                    /* a space	    */
 	printf("%-12.*s",UT_LINESIZE,utbufp->ut_line);		/* the tty	    */
    printf(" %-16.*s",UT_HOSTSIZE,utbufp->ut_host);	    /* show host    */
    printf(" ");					                    /* a space	    */
	showtime( utbufp->ut_time , DATE_FMT);		        /* display time	*/
}

void showtime( time_t timeval , char *fmt )
/*
 * (!!Borrowed from class example!!)
 * displays time in a format fit for human consumption.
 * Uses localtime to convert the timeval into a struct of elements
 * (see localtime(3)) and uses strftime to format the data
 */
{
	char	result[MAXDATELEN];
	struct tm *tp = localtime(&timeval);		/* convert time	*/
	strftime(result, MAXDATELEN, fmt, tp);		/* format it	*/
	fputs(result, stdout);
}

/******************************************************************
 * printLogout() - calculates duration from login and logout time *
 *               - prints days, hours, and minutes of session     *
 ******************************************************************/
void printLogout(time_t login, time_t logout)
{
    time_t diff    = logout-login;                     // Calculate diff
    time_t minutes = diff % 3600 / 60;                 // Calculate minutes
    time_t hours   = diff / 3600 % 24;                 // Calculate hours
    time_t days    = diff / 3600 / 24;                 // Calculate days
    if (days > 0) {
        printf(" (%ld+%02ld:%02ld)\n",days,hours,minutes);
    }
    else {
        printf("  (%02ld:%02ld)\n", hours, minutes);
    }
    // Probably a better option - fix if time permits
    //struct tm *dTime = gmtime(&diff);
    //printf("%d:%02d:%02d\n", dTime->tm_yday, dTime->tm_hour, dTime->tm_min); 
}