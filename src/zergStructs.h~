#ifndef zergStructs_H_
#define zergStructs_H_

typedef struct zergPacket
{
    char type :4;
    char version :4;
    unsigned int totalLength :24;
    unsigned short sourceId;
    unsigned short destinationId;
    unsigned int sequenceId;
}zergPacket;

typedef struct pcapFileHeader
{
    unsigned int fileTypeId;
    unsigned short majorVersion;
    unsigned short minorVersion;
    unsigned int gmtOffset;
    unsigned int accDelta;
    unsigned int maxLength;
    unsigned int linkLayerType;
}pcapFileHeader;

typedef struct pcapPacketHeader
{
  	unsigned int unixEpoch;
  	unsigned int microEpoch;
  	unsigned int lengthOfData;
 	unsigned int fullLength;
}pcapPacketHeader;

typedef struct ethernetHeader
{
  	unsigned char destMac[6];
  	unsigned char sourceMac[6];
  	unsigned short int etherType;
}ethernetHeader;

typedef struct ipv4Header
{
    unsigned char ipHeaderLength :4;
    unsigned char version :4;
    unsigned char dscp;
    unsigned short ipLength;
    unsigned short id;
    unsigned short flags;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short checksum;
    unsigned int sourceIp;
    unsigned int destIp;
}ipv4Header;

typedef struct ipv6Header
{
	unsigned char version :4;
	unsigned char trafficClass;
	unsigned int flowLabel :20;
	unsigned short payloadLength;
	unsigned char nextHeader;
	unsigned char hopLimit;
	unsigned char source[16];
	unsigned char destination[16];
}ipv6Header;

typedef struct udpHeader
{
    unsigned short sourcePort;
    unsigned short destPort;
    unsigned short length;
    unsigned short checksum;
}udpHeader;

typedef struct payload
{
    unsigned int currHitPoints :24;
    unsigned char armor;
    unsigned int maxHitPoints :24;
    unsigned char type;
	union sSpeed
	{
    	float fSpeed;
		unsigned int iSpeed;
	}sSpeed;
}payload;

typedef struct cPayload
{
    unsigned short command;
    unsigned short param1;
    union param2
    {
            float fParam2;
            int iParam2;
			unsigned int uiParam2;
    }param2;
}cPayload;

typedef struct gpsPayload
{
    union longitude
    {
            double dLong;
            unsigned long long iLong;
    }longitude;
    union latitude
    {
            double dLat;
            unsigned long long iLat;
    }latitude;
    union altitude
    {
            float fAltitude;
            unsigned int iAltitude;
    }altitude;
    union bearing
    {
            float fBearing;
            unsigned int iBearing;
    }bearing;
    union speed
    {
            float fSpeed;
            unsigned int iSpeed;
    }speed;
    union accuracy
    {
            float fAccuracy;
            unsigned int iAccuracy;
    }accuracy;
}gpsPayload;

#endif
