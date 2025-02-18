#include	<stdio.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<utmp.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include    <string.h>

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
void appendLogout(int, struct utmp, off_t, size_t);

int main(int ac, char *av[])
{
	struct utmp	utbuf;	/* read info into here */
	int	   utmpfd;		/* read from this descriptor */
	char   *info; //= ( ac > 1 ? av[1] : UTMP_FILE);
    char   *usrName;

    // Check args
    if (ac < 2 && ac != 4 ) {
        fprintf(stderr, "Usage: ./ulast USER [-f FILENAME]\n");
        exit(-1);
    }
    if (ac == 2) {
        usrName = av[1];
        info = UTMP_FILE;
    }
    else if (ac == 4) {
        if (strcmp(av[1], "-f") == 0) {
            info = av[2];
            usrName = av[3];
        }
        else {
            usrName = av[1];   
            if (strcmp(av[2], "-f") == 0) {
                info = av[3];         
            }
            else {
                fprintf(stderr, "Usage: ./ulast USER [-f FILENAME] \n");
                exit(-1);                
            }
        }
    }

	if ( (utmpfd = open( info, O_RDONLY )) == -1 ){
		fprintf(stderr,"%s: cannot open %s\n", *av, info );
		exit(1);
	}
    
    read_file(utmpfd, &utbuf, usrName);
 	close( utmpfd );
	return 0;
}

void read_file(int utmpfd, struct utmp *utbuf, char *usrName)
{
    off_t  filepos;
    size_t filesize;
    size_t buffSize = sizeof(*utbuf);

    //Move file offset to EOF
    filepos = lseek(utmpfd, 0, SEEK_END);
    filesize = filepos;
    if (filepos < buffSize) {
        printf("Error reading file");
        exit(-1);
    }

    // Start reading file from end to beginning
    if (filepos % buffSize != 0) {                 // Check for corrupted final entry
        printf("skipping incomplete entry!\n");
        filepos = lseek(utmpfd, -(filepos % buffSize), SEEK_CUR);
    }
    filepos = lseek(utmpfd, -buffSize, SEEK_CUR);  // Set filepos to beginning of last entry

	while (filepos >= 0 && read( utmpfd, utbuf, buffSize) == buffSize ) {
        if (strncmp(utbuf->ut_name, usrName, UT_NAMESIZE) == 0 && utbuf->ut_type==USER_PROCESS) {
            show_info( utbuf );
            appendLogout(utmpfd, *utbuf, filepos, filesize);
        }
        filepos = lseek(utmpfd, filepos-buffSize, SEEK_SET);
    }
}

void appendLogout(int fd, struct utmp utbuf, off_t pos, size_t fsize)
{
    size_t buffSize = sizeof(utbuf);
    pid_t pid = utbuf.ut_pid;
    time_t loginTime = utbuf.ut_time;
    time_t logoutTime;
    time_t diff;

    while (pos < fsize && read( fd, &utbuf, buffSize) == buffSize ) {
        if (utbuf.ut_pid == pid || utbuf.ut_type==BOOT_TIME) {
            logoutTime = utbuf.ut_time;
            printf(" - ");
            showtime(logoutTime, "%H:%M");
            diff = logoutTime-loginTime;
            printf("  (%02ld:%02ld)\n", (diff / 3600), (diff % 3600 / 60));
            return;
        }
        pos = lseek(fd, pos+buffSize, SEEK_SET);
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
		
