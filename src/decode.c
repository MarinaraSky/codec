#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parseCapture(FILE *psychicCapture);
float hexToFloat(unsigned char *charArray);
double longHexToDouble(unsigned char *charArray);

typedef struct pscyhicPacket
{
	unsigned char *fileHeader;
	unsigned char *packetHeader;
	unsigned char *ethernetFrame;
	unsigned char *ipv4Header;
	unsigned char *udpHeader;
	unsigned char *zergHeader;
	unsigned char *payload;
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
				input.payload = malloc(sizeof(char) * zergHeaderLength - 12);
			}
		}
		else
		{
			if(i < 94)
			{
				input.zergHeader[i - 82] = *buff;
			}
			else
			{
				input.payload[i - 94] = *buff;
			}
		}
		i++;
	}
	char messageType = input.zergHeader[0] & 15;
	char version = input.zergHeader[0] & 16;
	int senderId = 0;
	int receiverId = 0;
	unsigned long int sequenceNum = 0;
	if(version == 16)
	{
		printf("Version: 1\n");
	}
	else
	{
		printf("Version: Unknown\n");
	}
	for(int i = 4; i < 6; i++)
	{
		senderId = senderId << 8;
		senderId |= input.zergHeader[i]; 
	
	}
	for(int i = 6; i < 8; i++)
	{
		receiverId = receiverId << 8;
		receiverId |= input.zergHeader[i];
	}
	for(int i = 8; i < 12; i++)
	{
		sequenceNum =  sequenceNum << 8;
		sequenceNum |= input.zergHeader[i];
	}
	printf("Sequence Num: %ld\n", sequenceNum);
	printf("Sender ID: %d\n", senderId);
	printf("Receiver ID: %d\n", receiverId);
	printf("Message type: ");
	switch(messageType)
	{
		case(0):
			printf("Message\n");
			input.payload[strlen((char *)input.payload) - 1] = 0;
			printf("%s\n", input.payload);
			break;
		case(1):
			printf("Status\n");
			const char *zergTypes[] = {"Overmind", "Larva", "Cerebrate", 
			"Overlord", "Queen", "Drone", "Zergling", "Lurker", "Brooding",
			"Hydralisk", "Guardian", "Scourge", "Ultralisk",
			"Mutalisk", "Defiler", "Devourer"};
			int health = 0;
			int maxHealth = 0;
			int armor = input.payload[3];
			unsigned int type = input.payload[7];
			unsigned char speedArray[4];
			for(int i = 0; i < 3; i++)
			{
				health += input.payload[i];
			}
			for(int i = 4; i < 7; i++)
			{
				maxHealth += input.payload[i];
			}
			int j = 0;
			for(int i = 8; i < 12; i++)
			{
				speedArray[j] = input.payload[i];
				j++;
			}
			j = 0;
			char *name = malloc(sizeof(char) * (zergHeaderLength - 24));
			for(unsigned int i = 12; i < zergHeaderLength - 12; i++)
			{
				name[j] = input.payload[i];
				j++;
			}
			float unitSpeed = hexToFloat(speedArray);
			printf("Name: %s\n", name);
			printf("Type: %s\n", zergTypes[type]);
			printf("Speed: %f m\\s\n", unitSpeed);
			printf("Health: %d/%d\n", health, maxHealth);
			printf("Armor: %d\n", armor);
			break;
		case(2):
			printf("command\n");
			const char *commandList[] = {"GET_STATUS", "GOTO", "GET_GPS",
				"RESERVED", "RETURN", "SET_GROUP", "STOP", "REPEAT"};
			int command = 0;;
			j = 0;
			for(int i = 0; i < 2; i++)
			{
				command += input.payload[i];
			}
			printf("%s\n", commandList[command]);
			if(command % 2 == 1)
			{
				unsigned char *param1 = malloc(sizeof(char) * 2);
				unsigned char *param2 = malloc(sizeof(char) * 4);
				j = 0;
				for(int i = 2; i < 4; i++)
				{
					param1[j] = input.payload[i];
					j++;
				}
				j = 0;
				for(int i = 4; i < 8; i++)
				{
					param2[j] = input.payload[i];
					j++;
				}	
				switch(command)
				{
					case(1):
						{
							int distance = 0;
							float newBearing = hexToFloat(param2);

							for(int i = 0; i < 2; i++);
							{
								distance = distance << 8;
								distance |= param1[i];
							}
							printf("Move %d meters in %f\n", distance, newBearing);
							break;
						}
					case(5):
						{
							int addOrRemove = 0;
							int groupId = 0;
							for(int i = 0; i < 2; i++)
							{
								addOrRemove = addOrRemove << 8;
								addOrRemove |= param1[i];
							}
							for(int i = 0; i < 4; i++)
							{
								groupId = groupId << 8;
								groupId |= param2[i];
							}
							if(addOrRemove == 1)
							{
								printf("Add ");
							}
							else
							{
								printf("Remove ");
							}
							printf("unit from group %d\n", groupId);
							break;
						}
					case(7):
						{
							int repeatSequenceId = 0;
							for(int i = 0; i < 4; i++)
							{
								repeatSequenceId = repeatSequenceId << 8;
								repeatSequenceId |= param2[i];
							}
							printf("Request resending of packet with ID: %d\n", repeatSequenceId);
							break;
						}
				}
			}
			break;
		case(3):
			{
				printf("GPS Message\n");
				unsigned char longitude[8];
				unsigned char latitude[8];
				unsigned char altitude[4];
				unsigned char bearing[4];
				unsigned char travelSpeed[4];
				unsigned char accuracy[4];
				j = 0;
				for(int i = 0; i < 8; i ++)
				{
					longitude[j] = input.payload[i];	
					j++;
				}
				j = 0;
				for(int i = 8; i < 16; i++)
				{
					latitude[j] = input.payload[i];
					j++;
				}
				j = 0;
				for(int i = 16; i < 20; i++)
				{
					altitude[j] = input.payload[i];
					j++;
				}
				j = 0;
				for(int i = 20; i < 24; i++)
				{
					bearing[j] = input.payload[i];
					j++;
				}
				j = 0;
				for(int i = 24; i < 28; i++)
				{
					travelSpeed[j] = input.payload[i];
					j++;
				}
				j = 0;
				for(int i = 28; i < 32; i++)
				{
					accuracy[j] = input.payload[i];
					j++;
				}
	
				double fLongitude = longHexToDouble(longitude);
				double fLatitude = longHexToDouble(latitude);
				float fAltitude = hexToFloat(altitude);
				float fBearing = hexToFloat(bearing);
				float fSpeed = hexToFloat(travelSpeed);
				float fAccuracy = hexToFloat(accuracy);
				char fancyLongitude[10];
				char fancyLatitude[10];
				sprintf(fancyLongitude, "%f", fLongitude);
				sprintf(fancyLatitude, "%f", fLatitude);
	
				printf("Long: %s\n", fancyLongitude);
				printf("Lat: %s\n", fancyLatitude);
				printf("Alt: %f fathoms\n", fAltitude);
				printf("Bearing: %f\u00B0\n", fBearing);
				printf("Speed: %f m\\s\n", fSpeed);
				printf("Acc: %fm\n", fAccuracy);
	
				break;
			}
	}
	free(input.fileHeader);
}

float hexToFloat(unsigned char *charArray)
{
	unsigned long bitFloat = 0;
	for(int i = 0; i < 4; i++)
	{
		bitFloat = bitFloat << 8;
		bitFloat |= charArray[i];	
	}
	float myFloat = 0.0;
	memcpy(&myFloat, &bitFloat, sizeof(float));
	return myFloat;
}

double longHexToDouble(unsigned char *charArray)
{
	unsigned long long bitFloat = 0;
	for(int i = 0;i < 8; i ++)
	{
		bitFloat = bitFloat << 8;
		bitFloat |= charArray[i];
	}
	double myFloat = 0.0;
	memcpy(&myFloat, &bitFloat, sizeof(double));
	return myFloat;
}
