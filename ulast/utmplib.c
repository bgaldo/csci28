#include	<stdio.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<utmp.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include    <sys/stat.h>
#include    "utmplib.h"
/**********************************************************
 *    utmplib.c                                           *
 *    Contains functions to buffer reads from utmp file   * 
 **********************************************************/
#define UTSIZE (sizeof(struct utmp))

static struct stat sb; 
static int    fd, pageSize, numElements, currentPage = -1, elementsRead;
static off_t  filepos;
static struct utmp *utBuf;
static int    stats[2] = {0,0};  // records read, buffer misses

/*******************************************************************
 * utmp_open()        - Opens a file passed as a parameter         *
 *                    - Calls stat to find filesystem block size   *
 *                    - Determines optimal pagesize and allocates  *
 *                      memory for an array of utemp entries       *
 *                    - Returns file descriptor, or -1 if error    *
 *******************************************************************/
int utmp_open(char *filename)
{
    stat(filename, &sb);                 // Get filesystem stats
    fd = open( filename, O_RDONLY );    
    numElements = sb.st_blksize/UTSIZE;   // 4096/384 = 10 entries
    pageSize = UTSIZE * numElements;      // 10 * 384 = 3840b
    if (fd != -1) {       // allocate memory for numElements
        utBuf = calloc(sb.st_blksize/UTSIZE, UTSIZE);
    }
    return fd;
}

/*******************************************************************
*  utmp_len()         - Returns number of utmp entries in file     *
********************************************************************/
int utmp_len()
{
    return sb.st_size / UTSIZE;
}

/********************************************************************
 * utmp_close()       - Closes utmp file, collects stats, and frees *
 *                      allocated memory                            *
 ********************************************************************/
void utmp_close()
{
    free(utBuf);
    utmp_stats(stats);
    close(fd);
}

/*********************************************************************
 * utmp_getrec()       - Returns a pointer to the utmp entry that    *
 *                       corresponds to the index passed as a param  * 
 *********************************************************************/
struct utmp* utmp_getrec(int index)
{
    stats[0] += 1;                            // Add one entry request
    if (index/numElements != currentPage) {   // See if entry is in buffer
        filepos = lseek(fd, pageSize*(index/numElements), SEEK_SET);
        elementsRead = read(fd, utBuf, pageSize); // Reload buffer
        currentPage = index/numElements;          // update currentPage
        stats[1] += 1;                            // add 1 buffer miss
    }
    return &(utBuf[index % numElements]);         // Return ptr to entry 
}

/********************************************************************
 * utmp_stats()     - Provides number of entry requests and buffer  *
 *                    misses as a two-element int array             *
 ********************************************************************/
void utmp_stats(int a[2])
{
    a[0] = stats[0];                              // Entry requests
    a[1] = stats[1];                              // Buffer misses
}
 