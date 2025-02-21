#include	<stdio.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<utmp.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include    <string.h>
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

int utmp_open(char *filename)
{
    stat(filename, &sb);
    fd = open( filename, O_RDONLY );
    numElements = sb.st_blksize/UTSIZE;  // 4096/384 = 10 entries
    pageSize = UTSIZE * numElements;      // 10 * 384 = 3840b
    if (fd != -1) {       // allocate memory for numElements
        utBuf = calloc(sb.st_blksize/UTSIZE, UTSIZE);
    }
    return fd;
}

int utmp_len()
{
    return sb.st_size / UTSIZE;
}

void utmp_close()
{
    free(utBuf);
    utmp_stats(stats);
    close(fd);
}

struct utmp* utmp_getrec(int index)
{
    stats[0] += 1;
    if (index/numElements != currentPage) {
        filepos = lseek(fd, pageSize*(index/numElements), SEEK_SET);
        elementsRead = read(fd, utBuf, pageSize);
        currentPage = index/numElements;
        stats[1] += 1;
    }
    return &(utBuf[index % numElements]);
}

void utmp_stats(int a[2])
{
    a[0] = stats[0];
    a[1] = stats[1];
    //fprintf(stderr, "%d records read, %d buffer misses\n", stats[0], stats[1]);
}
 