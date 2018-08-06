#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define MESSAGE	0x01
#define STATUS	0x02
#define COMMAND	0x03
#define GPS		0x04

typedef struct zergPacket
{
	char type :4;
	char version :4;
	int totalLength :24;
	short sourceId;
	short destinationId;
	int sequenceId;
}zergPacket;

typedef struct pcapFileHeader
{
	int fileTypeId;
	short int majorVersion;
	short int minorVersion;
	int gmtOffset;
	int accDelta;
	int maxLength;
	int linkLayerType;
}pcapFileHeader;

typedef struct pcapPacketHeader
{
	int unixEpoch;
	int microEpoch;
	int lengthOfData;
	int fullLength;
}pcapPacketHeader;

typedef struct ethernetHeader
{
	char destMac[6];
	char sourceMac[6];
	short int etherType;
}ethernetHeader;

typedef struct ipv4Header
{
	unsigned char ipHeaderLength: 4;
	unsigned char version :4;
	char dscp;
	short ipLength;
	short id;
	short flags;
	unsigned char ttl;
	unsigned char protocol;
	short checksum;
	int sourceIp;
	int destIp;
}ipv4Header;

typedef struct udpHeader
{
	short sourcePort;
	short destPort;
	short length;
	short checksum;
}udpHeader;

typedef struct payload
{
	int currHitPoints :24;
	unsigned char armor;
	int maxHitPoints :24;
	char type;
	float speed;
}payload;

typedef struct cPayload 
{
	short command;
	unsigned short param1;
	union param2
	{
		float fParam2;
		int iParam2;
	}param2;
}cPayload;

void readZerg(FILE *source, FILE *dest);
void checkEntry(char string[16], int input, zergPacket *packet);
void pickPacketType(FILE *source, FILE *dest, zergPacket *packet);
void writeMessage(FILE *source, FILE *dest);
void writeStatus(FILE *source, FILE *dest);
void writeCommand(FILE *source, FILE *dest);
void writePcapHeader(FILE *dest);
void writePcapPacket(FILE *dest, int zergLength);
void writeEtherHeader(FILE *dest);
void writeIpv4Header(FILE *dest, int zergLength);
void writeUdpHeader(FILE *dest, int zergLength);
void writeZergHeader(FILE *dest, zergPacket *packet);
int rotate3ByteInt(int swap);
int rotateBack(int swap);
int rotate4ByteInt(int swap);

static int zergPayloadSize = 0;

int main(int argc, char *argv[])
{
	if(argc == 3)
	{
		FILE *dest;
		FILE *source;
		dest = fopen(argv[2], "wb");
		source = fopen(argv[1], "r");
		writePcapHeader(dest);
		readZerg(source, dest);
		fclose(dest);
		fclose(source);
	}
	return 0;
}

void readZerg(FILE *source, FILE *dest)
{
	zergPacket *packet = calloc(sizeof(zergPacket), 1);
	int readHeader = 0;
	while(readHeader < 7)
	{
		char string[16];
		int input = 0;
		fscanf(source, "%s %d", string, &input);
		checkEntry(string, input, packet);
		readHeader++;
	}
	pickPacketType(source, dest, packet);
	free(packet);
}

void checkEntry(char string[16], int input, zergPacket *packet)
{
	if(strcmp("Version:", string) == 0)
	{
		packet->version = input;
	}
	else if(strcmp("Type:", string) == 0)
	{
		packet->type = input;
	}
	else if(strcmp("Size:", string) == 0)
	{
		packet->totalLength = input;
		zergPayloadSize = input - 13;
	}
	else if(strcmp("From:", string) == 0)
	{
		packet->sourceId = input; 
	}
	else if(strcmp("To:", string) == 0)
	{
		packet->destinationId = input;
	}
	else if(strcmp("Sequence:", string) == 0)
	{
		packet->sequenceId = input;
	}
}

void pickPacketType(FILE *source, FILE *dest, zergPacket *packet)
{
	writePcapPacket(dest, packet->totalLength);
	writeEtherHeader(dest);
	writeIpv4Header(dest, packet->totalLength);
	writeUdpHeader(dest, packet->totalLength);
	writeZergHeader(dest, packet);
	switch(packet->type)
	{
		case(0):
			{
				writeMessage(source, dest);
				break;
			}
		case(1):
			{
				writeStatus(source, dest);
				break;
			}
		case(2):
			{
				writeCommand(source, dest);
				break;
			}
		case(3):
			{
				printf("GPS\n");
				break;
			}
		default:
			{
				printf("Not Found\n");
			}
	}
}

void writeMessage(FILE *source, FILE *dest)
{
	char grab = 0;
	fseek(source, 1, SEEK_CUR);
	for(int i = 0; i <= zergPayloadSize; i++)
	{
		grab = fgetc(source);
		fputc(grab, dest);
	} 
}

void writeStatus(FILE *source, FILE *dest)
{
	char string[16];
	int health = 0;
	int maxHealth = 0;
	int type = 0;
	int armor = 0;
	payload *status = calloc(sizeof(payload), 1);

	for(int i = 0; i < 9; i++)
	{
		fscanf(source, "%s", string);
		if(strcmp("Type:", string) == 0)
		{
			fscanf(source, "%d", &type);
			status->type = type & 0xff;
		}
		else if(strcmp("Speed:", string) == 0)
		{
			union speed
			{
				float fSpeed;
				int iSpeed;
			};
			union speed s;
			fscanf(source, "%s", string);
			s.fSpeed = strtof(string, NULL);
			s.iSpeed = htonl(s.iSpeed);
			status->speed = s.fSpeed;
		}	
		if(strcmp("Health:", string) == 0)
		{
			fscanf(source, "%d/%d", &health, &maxHealth);
			status->currHitPoints |= health;
			status->maxHitPoints |= maxHealth;
			status->currHitPoints = rotate3ByteInt(status->currHitPoints);
			status->maxHitPoints = rotate3ByteInt(status->maxHitPoints);
		}
		else if(strcmp("Armor:", string) == 0)
		{
			fscanf(source, "%d", &armor);
			status->armor = armor & 0xff;
			fwrite(status, sizeof(int) * 3, 1, dest);
		}
		else if(strcmp("Name:", string) == 0)
		{
			char grab = 0;
			fseek(source, 1, SEEK_CUR);
			for(int i = 0; i <= zergPayloadSize - 12; i++)
			{
				grab = fgetc(source);
				fputc(grab, dest);
			}
			free(status);
			return;
		}
	}
}

void writeCommand(FILE *source, FILE *dest)
{
	char string[16];
	short int input = 0;
	char garbage[16];
	cPayload *command = calloc(sizeof(cPayload), 1);	
	fscanf(source, "%s %hd %s", string, &input, garbage);
	if(strcmp("Command:", string) == 0)
	{
		command->command = htons(input);
	}		
	switch(htons(command->command))
	{
		case(1):
			fscanf(source, "%s %hd", string, &input);
			command->param1 = htons(input);
			float fInput = 0.0;
			fscanf(source, "%s %f", string, &fInput);
			command->param2.fParam2 = fInput;
			command->param2.iParam2 = htonl(command->param2.iParam2);
			fwrite(command, sizeof(int) * 2, 1, dest); 
			break;
		case(5):

		case(7):
			break;
	}
}

void writePcapHeader(FILE *dest)
{
	pcapFileHeader *header = calloc(sizeof(pcapFileHeader), 1);
	header->fileTypeId = 0xa1b2c3d4;
	header->majorVersion = 0x2;
	header->minorVersion = 0x4;
	header->gmtOffset = 0;
	header->accDelta = 0;
	header->maxLength = 0x10000;
	header->linkLayerType = 0x1;
	fwrite(header, sizeof(int) * 6, 1, dest);
	free(header);
}

void writePcapPacket(FILE *dest, int zergLength)
{
	pcapPacketHeader *header = calloc(sizeof(pcapPacketHeader), 1);
	header->unixEpoch = 0;
	header->microEpoch = 0;
	header->lengthOfData = zergLength + 42;
	header->fullLength = 0;
	fwrite(header, sizeof(int) * 4, 1, dest);
	free(header);
}

void writeEtherHeader(FILE *dest)
{
	ethernetHeader *header = calloc(sizeof(ethernetHeader), 1);
	header->etherType = 0x8;
	fwrite(header, sizeof(char) * 14, 1, dest);
	free(header);
}

void writeIpv4Header(FILE *dest, int zergLength)
{
	ipv4Header *header = calloc(sizeof(ipv4Header), 1);
	header->version = 0x4;
	header->ipHeaderLength = 0x5;
	header->ipLength = htons(zergLength + 28);
	header->protocol = 0x11;
	fwrite(header, sizeof(int) * 5, 1, dest);
	free(header);
}	

void writeUdpHeader(FILE *dest, int zergLength)
{
	udpHeader *header = calloc(sizeof(udpHeader), 1);	
	header->destPort = htons(0xea7);
	header->length = htons(zergLength + 8);
	fwrite(header, sizeof(int) * 2, 1, dest);
	free(header);
}

void writeZergHeader(FILE *dest, zergPacket *packet)
{
	packet->totalLength = rotate3ByteInt(packet->totalLength);
	packet->sourceId = htons(packet->sourceId);
	packet->destinationId = htons(packet->destinationId);
	packet->sequenceId = rotate4ByteInt(packet->sequenceId);
	fwrite(packet, sizeof(int) * 3, 1, dest);
}

int rotate4ByteInt(int swap)
{
	swap = ((swap >> 24) + (swap & 0xff0000) + (swap & 0xff00) + 
			(swap & 0xff)) << 24;
	return swap;
}
int rotate3ByteInt(int swap)
{
	swap = ((swap >> 16) + (swap & 0xff00) + (swap & 0xff)) << 16;
	return swap;
}

int rotateBack(int swap)
{
	swap = ((swap << 16) + (swap & 0xff00) + (swap & 0xff)) >> 16;
	return swap;
}
