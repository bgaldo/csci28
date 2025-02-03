/*  more03.c  - version 0.3 of more
 *      read and print one page then pause for a few special commands
 *      v03: 
 */
#include <stdio.h>
#include <stdlib.h>
#include "termfuncs.h"     
#define  ERROR          1
#define  SUCCESS        0
#define  has_more_data(x)   (!feof(x))
#define CTL_DEV "/dev/tty"              /* source of control commands   */

int  do_more(FILE *, int *pagelen);
int  how_much_more(FILE *, int *pagelen);
void print_one_line(FILE *, int *pagelen);
void getPageSize(int *pagelen);

int main( int ac , char *av[] )
{
        int  pagelen[2];		/* val[0]=rows, val[1]=cols     */
	FILE    *fp;                    /* stream to view with more     */
        int     result = SUCCESS;       /* return status from main      */
	getPageSize(pagelen);           /* get initial teminal size     */
        if ( ac == 1 )
                result = do_more( stdin, pagelen );
        else
                while ( result == SUCCESS && --ac )
                        if ( (fp = fopen( *++av , "r" )) != NULL ) {
                                result = do_more( fp, pagelen ) ; 
                                fclose( fp );
                        }
                        else
                                result = ERROR;
        return result;
}
/*  do_more -- show a page of text, then call how_much_more() for instructions
 *      args: FILE * opened to text to display
 *      rets: SUCCESS if ok, ERROR if not
 */
int do_more( FILE *fp, int *pagelen )
{
        int     space_left = pagelen[0] - 1;     /* space left on screen */
        int     reply;                           /* user request         */
        FILE    *fp_tty;                         /* stream to keyboard   */

        fp_tty = fopen( CTL_DEV, "r" );          /* connect to keyboard  */
        while ( has_more_data( fp ) ) {          /* more input   */
                if ( space_left <= 0 ) {         /* screen full? */
			printf("\033[7m more? \033[m"); /*reverse on a vt100 */
                        reply = how_much_more(fp_tty, pagelen); /* ask user  */
			fprintf(stdout, "\r                          \r");
                        if ( reply == 0 )        /*    n: done    */
                                exit(0);	 /* exit if user enters 'q'  */
                        space_left = reply;      /* reset count   */
                }
		print_one_line( fp, pagelen );
                space_left--;		         /* count it      */     
        }
        fclose( fp_tty );                       /* disconnect keyboard   */
	pagelen[0] = 0;				/* set to 0 for next file*/
        return SUCCESS;                         /* EOF => done           */
}

/*  print_one_line(fp) -- copy data from input to stdout until \n or EOF */
void print_one_line( FILE *fp, int* pagelen)
{
        int     c;
	int 	charCount = 0; /* chars printed (doesn't account for tabs)*/
        while( ( c = getc(fp) ) != EOF && c != '\n' )
	{
		putchar(c);
		if (++charCount == pagelen[1])  /* add check for line length */
		{
			putchar('\n');
			return;
		}
	}
	if (c == EOF) 				/* print prompt at EOF       */
	{
		printf("\033[7m more? - Next File\r \033[m");
	}
	else putchar( c );
}

/*  how_much_more -- ask user how much more to show
 *      args: none
 *      rets: number of additional lines to show: 0 => all done
 *      note: space => screenful, 'q' => quit, '\n' => one line
 */
int how_much_more(FILE *fp, int *pagelen)
{
        int     c;
	getPageSize(pagelen);  	                /* update pagelen          */
        while( (c = rawgetc(fp)) != EOF )       /* get user input          */
        {
                if ( c == 'q' )                 /* q -> N                  */
                        return 0;
                if ( c == ' ' )			/* ' ' => next page        */   
		{
			return pagelen[0] - 1;  /* leave 1 line for prompt */
		}                 
                if ( c == '\n' )                /* Enter key => 1 line     */
                        return 1;               
        }
        return 0;
}

/* Updates the current terminal size. Exits with -1 if get_term_size fails */
void getPageSize(int *pagelen) 
{
	if(get_term_size(pagelen) == -1)
	{
		fprintf(stderr, "Error reading terminal size");
		exit(-1);
	}
}