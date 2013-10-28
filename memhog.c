/* memhog.c
   Glen Wiley <gwiley@verisign.com>

	This is intended to allocate ram to help test how a machine behaves when 
	it gets worked.

	TODO: allow units to be specified, K/M/G
	TODO: permit stack
	TODO: shared memory
	TODO: lock ram
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*---------------------------------------- usage */
void
usage(void)
{
	printf(
	  "USAGE: memhog [-wuv] -s <kbytes>\n"
	  "\n"
	  "  -s <kbytes>  allocate <kbytes> on the heap\n"
	  "  -w           allocate then wait - do not exit (use CTRL-C to exit)\n"
	  "  -u           use the memory (via memcpy)\n"
	  "  -v           be verbose\n"
     "\n"
	  "program will exit with 0 if the memory was successfully allocated\n"
	  "\n"
	  );
	return;
} /* usage */

/*---------------------------------------- main */
int
main(int argc, char *argv[])
{
	int opt;
	int retval  = 0;
	int sz      = 0;
	int noexit  = 0;
	int usemem  = 0;
	int verbose = 0;
	void *buf   = NULL;

	if(argc == 1)
	{
		usage();
		exit(1);
	}

	/*-------------------- command line args */
	while((opt = getopt(argc, argv, "?ws:uv")) != EOF)
	{
		switch(opt)
		{
			case 's':
				sz = atoi(optarg);
				if(sz == 0)
					printf("error, specify size in kbytes for -s\n\n");
				break;

			case 'u':
				usemem = 1;
				break;

			case 'v':
				verbose = 1;
				break;

			case 'w':
				noexit = 1;
				break;

			case 'h':
			case '?':
			default:
				usage();
				exit(1);
		}
	}

	if(sz == 0)
	{
		usage();
		exit(1);
	}

	if(verbose)
		printf("allocating memory...\n");

	buf = malloc(sz * 1024);
	if(buf == NULL)
	{
		printf("error, malloc failed, %d, %s\n", errno, strerror(errno));
		exit(1);
	}

	if(usemem)
	{
		if(verbose)
			printf("filling memory with 'C' (for Chris)...\n");
		memset(buf, 'C', sz * 1024);
	}

	if(noexit)
	{
		if(verbose)
			printf("waiting forever...\n");

		while (1) sleep(10);
	}

	if(verbose)
		printf("exiting with joy...\n");

	return retval;
} /* main */

/* memhog.c */
