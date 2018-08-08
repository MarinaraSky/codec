#ifndef zergStructs_H_
#define zergStructs_H_

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

#endif
