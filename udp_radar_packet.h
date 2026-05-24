#pragma once
#include <cstdint>

// Magic number to validate packet integrity: "RDR\0"
static constexpr uint32_t RADAR_UDP_MAGIC = 0x41756469;

// One detection point on the wire
#pragma pack(push, 1)
struct tUDPRadarPoint
{
    float   fX;         // [m]  forward
    float   fY;         // [m]  left
    float   fZ;         // [m]  up
    float   fVelocity;  // [m/s]
    float   fRCS;       // [dBm^2]
    float   fSNR;       // [dB]
    uint8_t nPdh0;      // false detection bitmask
};

// Packet header
struct tUDPRadarHeader
{
    uint32_t nMagic;        // = RADAR_UDP_MAGIC
    uint32_t nCycle;        // nCycleCounter from radar
    uint32_t nTimestampUs;  // nTimeStamp from radar [us]
    uint16_t nNearCount;    // number of near detections following
    uint16_t nFarCount;     // number of far detections following
};
#pragma pack(pop)

// Max detections per cycle
static constexpr uint16_t MAX_NEAR_DETECTIONS = 108;  // 38+38+32
static constexpr uint16_t MAX_FAR_DETECTIONS  =  76;  // 38+38

// Max UDP payload size
static constexpr size_t MAX_UDP_PAYLOAD =
    sizeof(tUDPRadarHeader) +
    (MAX_NEAR_DETECTIONS + MAX_FAR_DETECTIONS) * sizeof(tUDPRadarPoint);
// = 16 + 184 * 25 = 4616 bytes — well within UDP limit