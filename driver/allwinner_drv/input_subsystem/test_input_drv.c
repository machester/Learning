#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

#define usage() 													\
		do{															\
				printf("usage:\n");									\
				printf("\t <%s> <%s>\n", "./file", "filename");		\
				return 0;											\
		}while(0)
int main(int argv, char **argc)
{
	if(argv != 2) {
		usage();
	}
	printf("open PATH =  /dev/%s\n", argc[1]);

	return 0;
}
