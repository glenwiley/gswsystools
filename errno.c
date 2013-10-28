/* errno.c
	$Id$
	Glen Wiley <glen.wiley@gmail.com>

	little command line utility to translate errno values to strings
	or search for error numbers based on a sub-string

	works on linux or OSX
	
	TODO: use regex for searches
	TODO: detect diff between machine built on and machine running on and warn user
	TODO: error numbers from other headers/interfaces as well
*/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSTRING 10
#define MAXERRNO  255

char *getErrString(int err);
int  buildmsgtable(char *msgs[MAXERRNO+1]);
void printmsgs(char *msgs[MAXERRNO+1], char *str);

/*---------------------------------------- main */
int
main(int argc, char *argv[])
{
	int err;
	int i;
	char s[MAXSTRING+2];
	char *tok;
	char *errstr;
	char *msgs[MAXERRNO+1];

	if(argc > 1 && strcmp(argv[1], "-h") == 0)
	{
		printf("USAGE: %s [<errno> ...]\n", argv[0]);
		return 0;
	}

	/* if any errnos were specified then print each one, otherwise
	   drop into a loop and print them as they are supplied */

	if(argc > 1)
	{
		for(i=1; i<argc; i++)
		{
			/* if user supplied a number then get the message
			   otherwise, try to find a number for the string */

			if(isdigit(argv[i][0]))
			{
				err = atoi(argv[i]);
				printf("errno %d: %s\n", err, getErrString(err));
			}
			else
			{
				buildmsgtable(msgs);
				printmsgs(msgs, argv[i]);
			}
		}
	}
	else
	{
		buildmsgtable(msgs);
		do
		{
			printf("errno? ");
			fgets(s, MAXSTRING, stdin);
			if(isdigit(s[0]))
			{
				err = atoi(s);
				if(err > 0)
					printf("errno %d: %s\n", err, getErrString(err));
			}
			else
			{
				tok = strchr(s, '\r');
				if(tok != NULL)
					*tok = '\0';
				tok = strchr(s, '\n');
				if(tok != NULL)
					*tok = '\0';

				printmsgs(msgs, s);
			}
		} while(err > 0);
	}

	return 0;
} /* main */

/*---------------------------------------- getErrString
  returns the string associated with the error number via strerror
  or "unknown error"
*/
char *
getErrString(int err)
{
	char *str;

	str = strerror(err);
	if(str == NULL)
		str = "unknown error";

	return str;
} /* getErrString */

/*---------------------------------------- buildmsgtable
  build a table of error message strings for error numbers 0-255
  returns: 0 on success
*/
int
buildmsgtable(char *msgs[MAXERRNO+1])
{
	int i;
	int c;

	for(i=0; i<=MAXERRNO; i++)
	{
		msgs[i] = strdup(getErrString(i));
		c = 0;
		/* convert message to lower case to facilitate searches */
		while(msgs[i][c] != '\0')
		{
			msgs[i][c] = tolower(msgs[i][c]);
			c++;
		}
	}

	return 0;
} /* buildmsgtable */

/*---------------------------------------- printmsgs
  print messages from message table that contain 'str' 
*/
void
printmsgs(char *msgs[MAXERRNO+1], char *str)
{
	int i;
	int c;
	char *strlc;

	/* convert string to lower case to facilitate searches */
	strlc = strdup(str);
	c = 0;
	while(strlc[c] != '\0')
	{
		strlc[c] = tolower(strlc[c]);
		c++;
	}

	for(i=0; i<=MAXERRNO; i++)
	{
		if(strstr(msgs[i], strlc) != NULL)
			printf("%d:%s\n", i, getErrString(i));
	}

	free(strlc);

	return;
} /* printmsgs */

/* errno.c */
