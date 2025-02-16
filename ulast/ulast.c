#include	<stdio.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<utmp.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>

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
                        #define	DATE_FMT	"%b %e %H:%M"	 /* text format	*/
                #else
                        #define	DATE_FMT	"%Y-%m-%d %H:%M" /* the default	*/
                #endif
        #endif

void show_info( struct utmp *);
int main(int ac, char *av[])
{
	struct utmp	utbuf;	/* read info into here */
	int	   utmpfd;		/* read from this descriptor */
	char   *info = ( ac > 1 ? av[1] : UTMP_FILE);
    off_t  filepos;
	if ( (utmpfd = open( info, O_RDONLY )) == -1 ){
		fprintf(stderr,"%s: cannot open %s\n", *av, info );
		exit(1);
	}
    filepos = lseek(utmpfd, 0, SEEK_END);
    if (filepos < sizeof(utbuf)) {
        printf("Error reading file");
        exit(-1);
    }
    if (filepos % sizeof(utbuf) != 0) {
        printf("skipping incomplete entry!\n");
        filepos = lseek(utmpfd, -(filepos % sizeof(utbuf)), SEEK_CUR);
    }
    filepos = lseek(utmpfd, -sizeof(utbuf), SEEK_CUR);  // Set filepos to beginning of last entry
    //printf("%ld\n", filepos);
    

	while (filepos >= 0 && read( utmpfd, &utbuf, sizeof(utbuf) ) == sizeof(utbuf) ) {
        //filepos = lseek(utmpfd, -(2*(sizeof(utbuf))), SEEK_CUR);
        filepos = lseek(utmpfd, filepos-sizeof(utbuf), SEEK_SET);
        show_info( &utbuf );
    }
 	close( utmpfd );
	return 0;
}

    /*
 *	show info()
 *			displays the contents of the utmp struct
 *			in human readable form
 *			* displays nothing if record has no user name
 */
void show_info( struct utmp *utbufp )
{
	void showtime(time_t, char *);

	if ( utbufp->ut_type != USER_PROCESS && utbufp->ut_type != BOOT_TIME )
		return;

	printf("%-8.32s", utbufp->ut_name);		/* the logname	*/
		/* better: "%-8.*s",UT_NAMESIZE,utbufp->ut_name) */
	printf(" ");					/* a space	*/
 	printf("%-12.12s", utbufp->ut_line);		/* the tty	*/
	printf(" ");					/* a space	*/
	//showtime( utbufp->ut_time , DATE_FMT);		/* display time	*/
	if ( utbufp->ut_host[0] != '\0' )		/* if not ""	*/
		printf(" (%.256s)", utbufp->ut_host);	/*    show host	*/
	printf("\n");					/* newline	*/
}
		
