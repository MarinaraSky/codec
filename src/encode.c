#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <byteswap.h>

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
	unsigned int sequenceId;
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
	unsigned short checksum;
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

typedef struct gpsPayload
{
	union longitude
	{
		double dLong;
		long long iLong;
	}longitude;
	union latitude
	{
		double dLat;
		long long iLat;
	}latitude;
	union altitude
	{
		float fAltitude;
		int iAltitude;
	}altitude;
	union bearing
	{
		float fBearing;
		int iBearing;
	}bearing;
	union speed
	{
		float fSpeed;
		int iSpeed;
	}speed;
	union accuracy
	{
		float fAccuracy;
		int iAccuracy;
	}accuracy;
}gpsPayload;

/* Read Zerg Header and send to parsing function */
void readZerg(FILE *source, FILE *dest);
/* Collects information about packet */
void checkEntry(char string[16], unsigned int input, zergPacket *packet);
/* Chooses the type of packet */
void pickPacketType(FILE *source, FILE *dest, zergPacket *packet);
/* Writes the packet to a file if its a message */
void writeMessage(FILE *source, FILE *dest);
/* Writes the packet to a file if its a status */
void writeStatus(FILE *source, FILE *dest);
/* Writes the packet to a file if its a command */
void writeCommand(FILE *source, FILE *dest);
/* Writes the packet to a file if its a GPS */
void writeGPS(FILE *source, FILE *dest);
/* Writes the Pcap Header to file */
void writePcapHeader(FILE *dest);
/* Writes the Pcap Packet to file */
void writePcapPacket(FILE *dest, int zergLength);
/* Writes the Ether Header to file */
void writeEtherHeader(FILE *dest);
/* Writes the Ipv4 Header to file */
void writeIpv4Header(FILE *dest, int zergLength);
/* Writes the UDP Header to file */
void writeUdpHeader(FILE *dest, int zergLength);
/* Writes the Zerg Header to file */
void writeZergHeader(FILE *dest, zergPacket *packet);
/* Rotates a 3 byte integer endianess */
int rotate3ByteInt(int swap);
/* Reverses the rotation */
int rotateBack(int swap);
/* Used to exit if anything goes wrong with reading the file */
void fileCorruption(void);
/* Used to check if there are invalid inputs in Zerg Header */
int validateHeader(zergPacket *packet);

static int zergPayloadSize = 0;
static int fscanNum = 0;

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
			fscanNum = 0;
			zergPayloadSize = 0;
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

void readZerg(FILE *source, FILE *dest)
{
	zergPacket *packet = calloc(sizeof(zergPacket), 1);
	if(packet == NULL)
	{
		printf("Unable to allocate memory. Quitting.\n");	
		exit(1);
	}
	int readHeader = 0;
	char string[16] = "";
	unsigned int input = 0;
	while(readHeader < 7)
	{
		fscanf(source, "%s %u", string, &input);
		checkEntry(string, input, packet);
		readHeader++;
	}
	if(validateHeader(packet))
	{
		pickPacketType(source, dest, packet);
		free(packet);
	}
}

void checkEntry(char string[16], unsigned int input, zergPacket *packet)
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

void pickPacketType(FILE *source, FILE *dest, zergPacket *packet) {
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
				writeGPS(source, dest);
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
		if(grab == EOF)
		{
			fileCorruption();
		}
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
	if(status == NULL)
	{
		printf("Cannot allocate memory\n");
		exit(1);
	}

	for(int i = 0; i < 9; i++)
	{
		fscanNum = fscanf(source, "%s", string);
		if(fscanNum != 1)
		{
			fileCorruption();
		}
		if(strcmp("Type:", string) == 0)
		{
			fscanNum = fscanf(source, "%d", &type);
			if(fscanNum != 1)
			{
				fileCorruption();
			}
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
			fscanNum = fscanf(source, "%s", string);
			if(fscanNum != 1)
			{
				fileCorruption();
			}
			s.fSpeed = strtof(string, NULL);
			s.iSpeed = htonl(s.iSpeed);
			status->speed = s.fSpeed;
		}	
		if(strcmp("Health:", string) == 0)
		{
			fscanNum = fscanf(source, "%d/%d", &health, &maxHealth);
			if(fscanNum != 2)
			{
				fileCorruption();
			}
			status->currHitPoints |= health;
			status->maxHitPoints |= maxHealth;
			status->currHitPoints = rotate3ByteInt(status->currHitPoints);
			status->maxHitPoints = rotate3ByteInt(status->maxHitPoints);
		}
		else if(strcmp("Armor:", string) == 0)
		{
			fscanNum = fscanf(source, "%d", &armor);
			if(fscanNum != 1)
			{
				fileCorruption();
			}
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
				if(grab == EOF)
				{
					printf("here\n");
					fileCorruption();
				}
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
	char garbage[16];
	short int input = 0;
	int intInput = 0;
	cPayload *command = calloc(sizeof(cPayload), 1);	
	if(command == NULL)
	{
		printf("Cannot allocate memory\n");
		exit(1);
	}
	fscanNum = fscanf(source, "%s %hd %s", string, &input, garbage);
	if(fscanNum != 3)
	{
		fileCorruption();
	}
	if(strcmp("Command:", string) == 0)
	{
		command->command = htons(input);
	}		
	switch(htons(command->command))
	{
		case(1):
			fscanNum = fscanf(source, "%s %hd", string, &input);
			if(fscanNum != 2)
			{
				fileCorruption();
			}
			command->param1 = htons(input);
			float fInput = 0.0;
			fscanNum = fscanf(source, "%s %f", string, &fInput);
			if(fscanNum != 2)
			{
				fileCorruption();
			}
			command->param2.fParam2 = fInput;
			command->param2.iParam2 = htonl(command->param2.iParam2);
			fwrite(command, sizeof(int) * 2, 1, dest); 
			break;
		case(5):
			fscanNum = fscanf(source, "%s %hd", string, &input);
			if(fscanNum != 2)
			{
				fileCorruption();
			}
			command->param1 = htons(input & 0x1);
			fscanNum = fscanf(source, "%s %d", string, &intInput);
			if(fscanNum != 2)
			{
				fileCorruption();
			}
			command->param2.iParam2 = htonl(intInput);
			fwrite(command, sizeof(int) * 2, 1, dest);
			break;
		case(7):
			fscanNum = fscanf(source, "%s %d", string, &intInput);
			if(fscanNum != 2)
			{
				fileCorruption();
			}
			command->param2.iParam2 = htonl(intInput);
			fwrite(command, sizeof(int) * 2, 1, dest);
			break;
		default:
			fwrite(command, sizeof(char) * 6, 1, dest);
			break;
	}
	free(command);
}

void writeGPS(FILE *source, FILE *dest)
{
	char string[16];
	double dInput = 0.0;
	float fInput = 0.0;
	char garbage[16];
	gpsPayload *gpsCoords = calloc(sizeof(gpsPayload), 1);
	if(gpsCoords == NULL)
	{
		printf("Cannot allocate memory\n");
		exit(1);
	}

	for(int i = 0; i < 6; i++)
	{
		fscanNum = fscanf(source, "%s", string);
		if(fscanNum != 1)
		{
			fileCorruption();
		}
		if(strcmp("Long:", string) == 0)
		{
			fscanNum = fscanf(source, "%lf", &dInput);
			if(fscanNum != 1)
			{
				fileCorruption();
			}
			gpsCoords->longitude.dLong = dInput;
			gpsCoords->longitude.iLong = 
				bswap_64(gpsCoords->longitude.iLong);
		}
		else if(strcmp("Lat:", string) == 0)
		{
			fscanNum = fscanf(source, "%lf", &dInput);
			if(fscanNum != 1)
			{
				fileCorruption();
			}
			gpsCoords->latitude.dLat = dInput;
			gpsCoords->latitude.iLat = bswap_64(gpsCoords->latitude.iLat);
		}
		else if(strcmp("Alt:", string) == 0)
		{
			fscanf(source, "%f %s", &fInput, garbage);
			if(fscanNum != 2)
			{
				fileCorruption();
			}
			gpsCoords->altitude.fAltitude = fInput;
			gpsCoords->altitude.iAltitude = 
				htonl(gpsCoords->altitude.iAltitude);
		}
		else if(strcmp("Bearing:", string) == 0)
		{
			fscanNum = fscanf(source, "%f %s", &fInput, garbage);
			if(fscanNum != 2)
			{
				fileCorruption();
			}
			gpsCoords->bearing.fBearing = fInput;
			gpsCoords->bearing.iBearing = 
				htonl(gpsCoords->bearing.iBearing);
		}
		else if(strcmp("Speed:", string) == 0)
		{
			fscanNum = fscanf(source, "%f %s", &fInput, garbage);
			if(fscanNum != 2)
			{
				fileCorruption();
			}
			gpsCoords->speed.fSpeed = fInput;
			gpsCoords->speed.iSpeed = htonl(gpsCoords->speed.iSpeed);
		}	
		else if(strcmp("Acc:", string) == 0)
		{
			fscanNum = fscanf(source, "%f %s", &fInput, garbage);
			if(fscanNum != 2)
			{
				fileCorruption();
			}
			gpsCoords->accuracy.fAccuracy = fInput;
			gpsCoords->accuracy.iAccuracy = 
				htonl(gpsCoords->accuracy.iAccuracy);
		}	
		else
		{
			printf("Something's not right here. Exiting.");
			exit(1);
		}
	}
	fwrite(gpsCoords, sizeof(int) * 8, 1, dest);
	free(gpsCoords);
}

void writePcapHeader(FILE *dest)
{
	pcapFileHeader *header = calloc(sizeof(pcapFileHeader), 1);
	if(header == NULL)
	{
		printf("Cannot allocate memory\n");
		exit(1);
	}

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
	if(header == NULL)
	{
		printf("Cannot allocate memory\n");
		exit(1);
	}

	header->unixEpoch = 0x11111111;
	header->microEpoch = 0x11111111;
	header->lengthOfData = zergLength + 42;
	header->fullLength = 0x11111111;
	fwrite(header, sizeof(int) * 4, 1, dest);
	free(header);
}

void writeEtherHeader(FILE *dest)
{
	ethernetHeader *header = calloc(sizeof(ethernetHeader), 1);
	if(header == NULL)
	{
		printf("Cannot allocate memory\n");
		exit(1);
	}

	header->etherType = 0x8;
	fwrite(header, sizeof(char) * 14, 1, dest);
	free(header);
}

void writeIpv4Header(FILE *dest, int zergLength)
{
	ipv4Header *header = calloc(sizeof(ipv4Header), 1);
	if(header == NULL)
	{
		printf("Cannot allocate memory\n");
		exit(1);
	}

	header->version = 0x4;
	header->ipHeaderLength = 0x5;
	header->ipLength = htons(zergLength + 28);
	header->protocol = 0x11;
	header->destIp = 0x22222222;
	fwrite(header, sizeof(int) * 5, 1, dest);
	free(header);
}	

void writeUdpHeader(FILE *dest, int zergLength)
{
	udpHeader *header = calloc(sizeof(udpHeader), 1);	
	if(header == NULL)
	{
		printf("Cannot allocate memory\n");
		exit(1);
	}

	header->destPort = htons(0xea7);
	header->length = htons(zergLength + 8);
	header->checksum = 0xefbe;
	fwrite(header, sizeof(int) * 2, 1, dest);
	free(header);
}

void writeZergHeader(FILE *dest, zergPacket *packet)
{
	packet->totalLength = rotate3ByteInt(packet->totalLength);
	packet->sourceId = htons(packet->sourceId);
	packet->destinationId = htons(packet->destinationId);
	packet->sequenceId = htonl(packet->sequenceId);
	fwrite(packet, sizeof(int) * 3, 1, dest);
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

void fileCorruption(void)
{
	printf("Unexpected data found in source document. Exiting.\n");
	exit(2);
}

int validateHeader(zergPacket *packet)
{
	if( packet->version == 0 && packet->totalLength == 0 && 
		packet-> sourceId == 0 && packet->destinationId == 0 && 
		packet->sequenceId == 0)
	{
		free(packet);
		return 0;
	}
	else if( packet->version == 0 || packet->totalLength == 0 || 
		packet-> sourceId == 0 || packet->destinationId == 0 || 
		packet->sequenceId == 0)
	{
		fileCorruption();
	}
	return 1;
}
