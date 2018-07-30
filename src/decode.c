#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parseCapture(FILE *psychicCapture);

typedef struct pscyhicPacket
{
	unsigned char *fileHeader;
	unsigned char *packetHeader;
	unsigned char *ethernetFrame;
	unsigned char *ipv4Header;
	unsigned char *udpHeader;
	unsigned char *zergHeader;
}psychicPacket;

int main(int argc, char *argv[])
{
	if(argc > 1)
	{
		FILE *psychicCapture;
		psychicCapture = fopen(argv[1], "rb");
		if(psychicCapture == NULL)
		{
			printf("Cannot open %s\n", argv[1]);
			exit(1);
		}
		parseCapture(psychicCapture);
		

	}
	return 0;
}

void parseCapture(FILE *psychicCapture)
{
	int buff[2];
	int i = 0;
	psychicPacket input;
	input.fileHeader = calloc(sizeof(char) * 24, 1);
	input.packetHeader = calloc(sizeof(char) * 16, 1);
	input.ethernetFrame = calloc(sizeof(char) * 16, 1);
	input.ipv4Header = calloc(sizeof(char) * 20, 1);
	input.udpHeader = calloc(sizeof(char) * 8, 1);
	unsigned int zergHeaderLength = 0;

	while(!feof(psychicCapture))
	{
		fread(buff, 1, 1, psychicCapture);
		if(i < 24)
		{
			input.fileHeader[i] = *buff;	
		}
		else if(i < 40)
		{
			input.packetHeader[i - 24] = *buff;	
		}
		else if(i < 56)
		{
			input.ethernetFrame[i - 40] = *buff;
		}
		else if(i < 74)
		{
			input.ipv4Header[i - 56] = *buff;
		}
		else if(i < 82)
		{
			input.udpHeader[i - 74] = *buff;
			if(i - 74 == 5)
			{
				for(int j = 4; j < 6; j++)
				{
					zergHeaderLength += input.udpHeader[j];
				}
				zergHeaderLength -= 8;
				input.zergHeader = malloc(sizeof(char) * zergHeaderLength);	
			}
		}
		else
		{
			input.zergHeader[i - 82] = *buff;
		}
		i++;
	}
	printf("file:\n");
	for(unsigned int i = 0; i < 24; i++)
	{
		printf("%x\n", input.fileHeader[i]);
	}
	printf("packet:\n");
	for(unsigned int i = 0; i < 16; i++)
	{
		printf("%x\n", input.packetHeader[i]);
	}
	printf("Ethernet: \n");
	for(unsigned int i = 0; i < 16; i++)
	{
		printf("%x\n", input.ethernetFrame[i]);
	}
	printf("IPV4: \n");
	for(unsigned int i = 0; i < 20; i++)
	{
		printf("%x\n", input.ipv4Header[i]);
	}
	printf("\n");
	printf("UDP:\n");
	for(unsigned int i = 0; i < 8; i++)
	{
		printf("%x\n", input.udpHeader[i]);
	}
	printf("Zerg:\n");
	for(unsigned int i = 0; i < zergHeaderLength; i++)
	{
		printf("%i\n", input.zergHeader[i]);
	}
	char type;
	type = input.zergHeader[0] &= 15;
	switch(type)
	{
		case(0):
			printf("Message\n");
			break;
		case(1):
			printf("Status\n");
			break;
		case(2):
			printf("command\n");
			break;
		case(4):
			printf("gps\n");
			break;
	}
	free(input.fileHeader);
}
