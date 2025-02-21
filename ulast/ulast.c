#include	<stdio.h>
#include	<unistd.h>
#include	<sys/types.h>
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

int main(int ac, char *av[])
{
	struct utmp	utbuf;	/* read info into here */
	int	   utmpfd;		/* read from this descriptor */
	char   *info, *usrName; //= ( ac > 1 ? av[1] : UTMP_FILE);
    int    showStats = 0, hasFilename=0, hasUsrName=0;
    int a[2];
  
    // Check args
    if (ac < 2 || ac > 5 ) {
        fprintf(stderr, "Usage: ./ulast USER [-f FILENAME]\n");
        exit(-1);
    }
    if (ac == 2) {
        usrName = av[1];
        info = UTMP_FILE;
        hasUsrName = 1;
        hasFilename = 1;
    }
    else {
        for (int i=1; i<ac; i++) {
            if (strcmp(av[i], "-f")==0 && i<ac-1) {
                info = av[++i];
                hasFilename = 1;
            }
            else if (strcmp(av[i], "-e")==0) {
                showStats = 1;
            }
            else {
                usrName = av[i];
                hasUsrName = 1;
            }
        }
    }
    if (!hasFilename || !hasUsrName || (ac==5 && !showStats)) {
        fprintf(stderr, "Usage: ./ulast USER [-f FILENAME]\n");
        exit(-1);        
    }

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
    int    numRecs, currentRec;
    numRecs = currentRec = utmp_len();

    if (numRecs < 1) {
        printf("Error reading file");
        exit(-1);
    }

    // Start reading file from end to beginning

	while (--currentRec >= 0) { //&& read( utmpfd, utbuf, buffSize) == buffSize ) {
        utbuf = utmp_getrec(currentRec);
        if (strncmp(utbuf->ut_name, usrName, UT_NAMESIZE) == 0 && utbuf->ut_type==USER_PROCESS) {
            show_info( utbuf );
            appendLogout(*utbuf, currentRec, numRecs);
        }
    }
}

void appendLogout(struct utmp utbuf, int currentRec, int numRecs)
{
    struct utmp utp;
    time_t logoutTime, diff, days, hours, minutes;

    while (++currentRec < numRecs) {
        utp = *utmp_getrec(currentRec);
        if (utp.ut_pid == utbuf.ut_pid || utp.ut_type==BOOT_TIME) {
            logoutTime = utp.ut_time;
            printf(" - ");
            if (utp.ut_type==BOOT_TIME) {
                printf("crash");
            }
            else {
                showtime(logoutTime, "%H:%M");
            }

            diff = logoutTime-utbuf.ut_time;
            minutes = diff % 3600 / 60;
            hours   = diff / 3600 % 24;
            days    = diff / 3600 / 24;
            if (days > 0) {
                printf(" (%ld+%02ld:%02ld)\n",days,hours,minutes);
            }
            else {
                printf("  (%02ld:%02ld)\n", hours, minutes);
            }
            return;
        }
    }
    printf("   still logged in");
	printf("\n");					/* newline	*/
}

/*
 *	show info()
 *			displays the contents of the utmp struct
 *			in human readable form
 *			* displays nothing if record has no user name
 */
void show_info( struct utmp *utbufp )
{
	printf("%-8.*s",UT_NAMESIZE,utbufp->ut_name);		/* the logname	*/
		/* better: "%-8.*s",UT_NAMESIZE,utbufp->ut_name) */
	printf(" ");					/* a space	*/
 	printf("%-12.*s",UT_LINESIZE,utbufp->ut_line);		/* the tty	*/
	
    //if ( utbufp->ut_host[0] != '\0' )		/* if not ""	*/
    printf(" %-16.*s",UT_HOSTSIZE,utbufp->ut_host);	/*    show host	*/
    printf(" ");					/* a space	*/
	showtime( utbufp->ut_time , DATE_FMT);		/* display time	*/
}

void showtime( time_t timeval , char *fmt )
/*
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