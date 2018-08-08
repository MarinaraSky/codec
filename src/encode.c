#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <byteswap.h>
#include "zergProtos.h"
#include "zergStructs.h"

#define MESSAGE	0x01
#define STATUS	0x02
#define COMMAND	0x03
#define GPS		0x04

int main(int argc, char *argv[])
{
	if(argc == 3)
	{
		FILE *dest;
		FILE *source;
		dest = fopen(argv[2], "wb");
		if(dest == NULL)
		{
			printf("Could not open %s. Exiting.\n", argv[2]);
			exit(1);
		}
		source = fopen(argv[1], "r");
		if(source == NULL)
		{
			printf("Could not open %s. Exiting.\n", argv[1]);
			exit(1);
		}
		writePcapHeader(dest);
		do
		{
			readZerg(source, dest);
		}while(!feof(source));
		fclose(dest);
		fclose(source);
	}
	else
	{
		printf("Usage: ./encode [Source] [Destination]\n");
	}
	return 0;
}

