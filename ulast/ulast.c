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

int main(int ac, char *av[])
{
	struct utmp	utbuf;	/* read info into here */
	int	   utmpfd;		/* read from this descriptor */
	char   *info; //= ( ac > 1 ? av[1] : UTMP_FILE);
    char   *usrName;

    // Check args
    if (ac < 2 || ac != 4 ) {
        fprintf(stderr, "Usage: ./ulast USER [-f FILENAME] \n");
        exit(-1);
    }
    if (ac == 2) {
        usrName = av[1];
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
    size_t buffSize = sizeof(*utbuf);
    pid_t  pid;

    //Move file offset to EOF
    filepos = lseek(utmpfd, 0, SEEK_END);
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
        filepos = lseek(utmpfd, filepos-buffSize, SEEK_SET);
        if (strncmp(utbuf->ut_name, usrName, UT_NAMESIZE) == 0) {
            show_info( utbuf );
        }
    }
}



/*
 *	show info()
 *			displays the contents of the utmp struct
 *			in human readable form
 *			* displays nothing if record has no user name
 */
void show_info( struct utmp *utbufp )
{
	if ( utbufp->ut_type != USER_PROCESS ) //&& utbufp->ut_type != DEAD_PROCESS )
		return;

	printf("%-8.32s", utbufp->ut_name);		/* the logname	*/
		/* better: "%-8.*s",UT_NAMESIZE,utbufp->ut_name) */
	printf(" ");					/* a space	*/
 	printf("%-12.12s", utbufp->ut_line);		/* the tty	*/
	
    if ( utbufp->ut_host[0] != '\0' )		/* if not ""	*/
    printf(" %-16.256s", utbufp->ut_host);	/*    show host	*/
    printf(" ");					/* a space	*/
	showtime( utbufp->ut_time , DATE_FMT);		/* display time	*/

	printf("\n");					/* newline	*/
}

#define	MAXDATELEN	100

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
		
