// cycle_accumulator.h
#pragma once
#include "decoded_messages.h"
#include "udp_radar_packet.h"
#include "../assignment/RadarTypes.h"
#include <vector>
#include <cstdint>

namespace RadarDecoder
{

class cCycleAccumulator
{
public:
    cCycleAccumulator();

    // Returns true when a complete cycle (all 5 sub-messages) is ready.
    // Call getSendBuffer() immediately after to get the serialised packet.
    bool addMessage(
        const RadarDecoded::tRDIMessage& msg,
        uint32_t                         nMessageID);

    // Returns serialised UDP payload ready to send.
    // Only valid after addMessage() returns true.
    const uint8_t* getSendBuffer() const;
    size_t         getSendBufferSize() const;

    // Current cycle stats for logging
    uint32_t getCurrentCycle()    const { return m_nCurrentCycle; }
    uint16_t getNearCount()       const { return m_nNearCount;    }
    uint16_t getFarCount()        const { return m_nFarCount;     }

private:
    void reset(uint32_t nNewCycle);
    void appendDetections(
        tUDPRadarPoint* pDest,
        uint16_t&       nCount,
        const RadarDecoded::tRDIMessage& msg);
    bool isCycleComplete() const;
    void serialise();

    static tUDPRadarPoint convertPoint(
        const RadarDecoded::tRDIDetection& det);

    // Cycle state
    uint32_t m_nCurrentCycle   = 0;
    uint32_t m_nTimestampUs    = 0;

    // Deduplication flags
    bool m_bNear0 = false;
    bool m_bNear1 = false;
    bool m_bNear2 = false;
    bool m_bFar0  = false;
    bool m_bFar1  = false;

    // Accumulated points (max counts)
    tUDPRadarPoint m_aNearPoints[MAX_NEAR_DETECTIONS];
    tUDPRadarPoint m_aFarPoints[MAX_FAR_DETECTIONS];
    uint16_t       m_nNearCount = 0;
    uint16_t       m_nFarCount  = 0;

    // Serialised send buffer
    uint8_t m_aSendBuffer[MAX_UDP_PAYLOAD];
    size_t  m_nSendBufferSize = 0;
};

} // namespace RadarDecoder