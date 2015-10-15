/*
 * This example illustrates the use of DosFindFirst, DosFindNext, and
 * DosFindClose.  The program lists the contents of a directory.
 * the parameter format is: FILELIST [-h] names...
 *
 * modified for 2 columns and only one switch for hidden files
 * Victor Roos, TFDL/ECIT 5-8-1988
 *
 * Created by Microsoft Corp. 1987
 * Appears also in assembler version in Iabucci's book: OS/2 prgrammers guide
 */

#define INCL_ERRORS
#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR

#include <malloc.h>
#include <stdio.h>
#include <time.h>
#include <os2def.h>
#include <bse.h>


/* file attribute constants */

#define	ATTR_READONLY	0x001	/* read only file */
#define	ATTR_HIDDEN	0x002	/* hidden file */
#define	ATTR_SYSTEM	0x004	/* system file */
#define	ATTR_DIRECTORY	0x010	/* subdirectory entry */
#define	ATTR_ARCHIVE	0x020	/* archive bit */

/* type of list */

#define TOTALCOLMS	2	/* number of columns */

#define RESULTBUFLEN	sizeof(FILEFINDBUF)	 /* size of ResultBuf */

#define SEARCHALL	"*.*"	/* default - search for everything */
#define BACKSLASH	"\\"
#define	MAXPATHSIZE	128
#define	FILEPATHSIZE	MAXPATHSIZE + sizeof(SEARCHALL) + 1

unsigned long	    fsize;	/* global filesize variable */
unsigned	    nfiles;	/* number of files in directory */
unsigned	    ndirs;	/* number of directories in directory */

main(argc, argv)
	int	argc;
	NPSZ	argv[];
{
	HDIR		DirHandle = -1; /* use any available directory handle */
	USHORT		SearchCount;	/* number of files to search for */
	USHORT		Attribute = ATTR_DIRECTORY;	/* default attribute */
	USHORT		rc;				/* return code */
	USHORT		listType;			/* default output */
	CHAR		FilePath[FILEPATHSIZE];
	NPSZ		s;
	PFILEFINDBUF	ResultBuf;	/* pointer to returned data */
	unsigned long	    tsize;	/* total K bytes in directory */

	/* parse command line for switches */

	while ((--argc > 0) && (**++argv == '-')) {
	    for (s = argv[0]+1; *s != '\0'; s++)

		switch(*s) {

		case 'h': Attribute |= (ATTR_HIDDEN | ATTR_SYSTEM);
			  break;
	
		default: printf("usage: ddir [ -h ] name...\n");
			 DosExit(EXIT_PROCESS, 0);
		}
	}

	/* allocate buffer for file data returned from find calls */

	if ((ResultBuf =
		(PFILEFINDBUF) malloc(RESULTBUFLEN)) == NULL) {

	    printf("error, not enough memory\n");
	    DosExit(EXIT_PROCESS, 0);
	}

	do {

	    if (argc > 0) {
		if (strlen(*argv) > MAXPATHSIZE) {
			printf("error, path too large\n");
			DosExit(EXIT_PROCESS, 1);
		}
		else {
			/* if path ends with a \, append "*.*" */

			strcpy(FilePath, *argv);
			
			if (FilePath[strlen(*argv) - 1] == ':')
			    strcat(FilePath, BACKSLASH);
			if (FilePath[strlen(*argv) - 1] == '\\')
			    strcat(FilePath, SEARCHALL);
		}
	    }
	    else
		strcpy(FilePath, SEARCHALL);	/* search using "*.*" */

	    printf("\nDirectory of %s\n\n", FilePath);
	    
	    tsize = 0;
	    fsize = 0;
	    nfiles = 0;
	    ndirs = 0;
	    SearchCount = 1;			/* search for one at a time */

	    /* search for first occurance of file search pattern */

	    if (rc = DosFindFirst( (PSZ) FilePath,
				   &DirHandle,
				   Attribute,
			  	   ResultBuf,	/* ptr to returned data */
				   RESULTBUFLEN,
				   &SearchCount,
			 	   0L )) {

		if (rc != ERROR_NO_MORE_FILES) {
		    printf("\nFindFirst failed, error %d\n", rc);
		    DosExit(EXIT_PROCESS, 1);
		}

	    } else {

		printDirEntry( ResultBuf );	     /* print file data */
		tsize = tsize + fsize;		     /* count total K */

		do {

		    /* search for next occurance of search pattern */

		    if (rc = DosFindNext( DirHandle,
				          ResultBuf,
				          RESULTBUFLEN,
					  &SearchCount )) {

		        if (rc != ERROR_NO_MORE_FILES)  {
			    printf("DosFindNext failed, error: %d\n", rc);
			    DosExit(EXIT_PROCESS, 1);
		        }
		    }
		    else
			/* print file data */

			printDirEntry( ResultBuf );
			tsize = tsize + fsize;	/* count total K */

    	
	        } while (SearchCount > 0);	/* when 0, no more files */
	    }

	    DosFindClose( DirHandle );		/* free directory handle */
	    printf("\n\n");
	    printf("Total of %lu K bytes in %u files in this directory\n", tsize, nfiles);
	    if (ndirs > 0 )
	    printf("%u directories in this directory", ndirs);
	    printf("\n");
	    tsize = 0;				/* reset total Kb counter */
	    nfiles = 0; 			/* reset file counter */
	    ndirs = 0;				/* reset directory counter */

	    if (argc > 1)			/* if any more arguments */
		argv++; 			/* point to next one */

	} while (--argc > 0);			/* for all filenames */
}
	

/*
 * This routine prints the data on the file found and returns the size of
 * the file in Kbytes in global variable fsize.
 */

printDirEntry(dirEntry)
	PFILEFINDBUF dirEntry;

{
	static int	colm = 0;
	static int	nlines = 0;
	char		attStr[5];
	FDATE		datum;
	FTIME		tijd;

	   /* build attribute string */
	   
	   fsize = 0;
	   strcpy(attStr, "----");
	   if (dirEntry->attrFile * ATTR_ARCHIVE)
		attStr[0] = 'a';
	   if (dirEntry->attrFile & ATTR_SYSTEM)
		attStr[1] = 's';
	   if (dirEntry->attrFile & ATTR_HIDDEN)
		attStr[2] = 'h';
	   if (dirEntry->attrFile & ATTR_READONLY)
		attStr[3] = 'r';

	   
	   printf("%-13s", dirEntry->achName);		/* print filename */
	   printf("%-5s", attStr);			/* print attribute */
	   if (dirEntry->attrFile & ATTR_DIRECTORY) {
		printf("<DIR>");			/* directory */
		ndirs = ndirs + 1;			/* count dirs */
		 }
	    else {
		fsize = dirEntry->cbFileAlloc/1024;	 /* allocated file size */
		/* NOTE the use of the allocated file size to get the size in K bytes */
		/* prevents the use of this program as a family API program. If this */
		/* is converted with BIND to be used with MS-Dos, a 0 value for the */
		/* file size is returned */
	       
		printf("%5lu", fsize);			 /* print file size */
		nfiles = nfiles + 1;			 /* count files */
		 }
	   datum = dirEntry->fdateLastWrite;
	   printf(" %2u-%2.2u-%u", datum.day, datum.month, datum.year+80);
	   tijd = dirEntry->ftimeLastWrite;
	   printf(" %2u:%2.2u", tijd.hours, tijd.minutes);
  
	   if (++colm == TOTALCOLMS) {
		printf("\n");
		colm = 0;
	    }
	    else
		printf(" \x0BA ");
	    if (++nlines == 44) {
		printf("Type any character to continue");
		getche();
		printf("\n");
		nlines = 0;
		}

}
