// sizes.c
// Glen Wiley <glen.wiley@gmail.com>
//
// print the size of various types

#include <stdlib.h>
#include <stdio.h>

//---------------------------------------- main
int
main(int argc, char *argv[])
{
	printf("char          : %lu\n", sizeof(char));
	printf("short int     : %lu\n", sizeof(short int));
	printf("int           : %lu\n", sizeof(int));
	printf("float         : %lu\n", sizeof(float));
	printf("long          : %lu\n", sizeof(long));
	printf("long long     : %lu\n", sizeof(long long));
	printf("double        : %lu\n", sizeof(double));

	return 0;
} // main

// end sizes.c
