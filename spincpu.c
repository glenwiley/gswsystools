/* spincpu.c
   $HeadURL: https://cmsvn.vrsn.com/ats/ad/arch/systools/spincpu.c $
   Glen Wiley <gwiley@verisign.com>

   make N cpus very busy - useful for testing power consumption and performance
   issues with a loaded host

   works on Solaris and Linux
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int g_verbose = 0;

#define BUFSIZE 1024 * 1024
#define VERSION "1.0"

void usage(void);

/*---------------------------------------- main */
int
main(int argc, char *argv[])
{
	int opt;
	int nthr = 1;
	int i;
	int j;
	int pid;
	double buf[BUFSIZE];

	while((opt=getopt(argc, argv, "hn:v")) != EOF)
	{
		switch(opt)
		{
			case 'n':
				nthr = atoi(optarg);
				break;	
			case 'v':
				g_verbose = 1;
				break;
			default:
				usage();
				exit(1);
				break;
		}
	}

	if(g_verbose)
		printf("spincpu v1.0 - Glen Wiley <gwiley@verisign.com>\n");

	printf("%d threads\n", nthr);

	for(i=0; i<nthr; i++)
	{
		pid = fork();
		if(pid == 0)
		{
			while (( 1 ))
			{
				for(j=0; j<BUFSIZE; j++)
					buf[j] = j / 100;
			}
		}
	}

	return 0;
} /* main */

/*---------------------------------------- usage */
void
usage(void)
{
   printf(
	   "spincpu v.%s - Glen Wiley <gwiley@verisign.com>\n"
		"\n"
		"USAGE: spincpu [-n <nthreads>] [-v]\n"
		"-n <nthreads>  number of threads to start, default is 1\n"
		"-v             be verbose\n"
		"\n"
		, VERSION
	 );

	return;
} /* usage */
