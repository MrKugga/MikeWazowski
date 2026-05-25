#pragma once
#include "decoded_messages.h"
#include "udp_radar_packet.h"
#include "validator.h"
#include "../assignment/RadarTypes.h"
#include <cstdint>


namespace RadarDecoder
{

// ── Near accumulator — collects RDINEAR_0, RDINEAR_1, RDINEAR_2 ───────────
class cNearAccumulator
{
public:
    cNearAccumulator();

    // Returns true when all 3 near sub-messages are received
    bool addMessage(
        const RadarDecoded::tRDIMessage& msg,
        uint32_t                         nMessageID);

    bool     isComplete()    const { return m_bNear0 && m_bNear1 && m_bNear2; }
    uint32_t getCycle()      const { return m_nCurrentCycle; }
    uint32_t getTimestamp()  const { return m_nTimestampUs; }
    uint16_t getCount()      const { return m_nCount; }

    const tUDPRadarPoint* getPoints() const { return m_aPoints; }

    void reset(uint32_t nNewCycle);

private:
    static tUDPRadarPoint convertPoint(const RadarDecoded::tRDIDetection& det);

    uint32_t       m_nCurrentCycle = 0;
    uint32_t       m_nTimestampUs  = 0;
    bool           m_bNear0 = false;
    bool           m_bNear1 = false;
    bool           m_bNear2 = false;
    tUDPRadarPoint m_aPoints[MAX_NEAR_DETECTIONS];
    uint16_t       m_nCount = 0;
};

// ── Far accumulator — collects RDIFAR_0, RDIFAR_1 ─────────────────────────
class cFarAccumulator
{
public:
    cFarAccumulator();

    // Returns true when both far sub-messages are received
    bool addMessage(
        const RadarDecoded::tRDIMessage& msg,
        uint32_t                         nMessageID);

    bool     isComplete()    const { return m_bFar0 && m_bFar1; }
    uint32_t getCycle()      const { return m_nCurrentCycle; }
    uint32_t getTimestamp()  const { return m_nTimestampUs; }
    uint16_t getCount()      const { return m_nCount; }

    const tUDPRadarPoint* getPoints() const { return m_aPoints; }

    void reset(uint32_t nNewCycle);

private:
    static tUDPRadarPoint convertPoint(const RadarDecoded::tRDIDetection& det);

    uint32_t       m_nCurrentCycle = 0;
    uint32_t       m_nTimestampUs  = 0;
    bool           m_bFar0 = false;
    bool           m_bFar1 = false;
    tUDPRadarPoint m_aPoints[MAX_FAR_DETECTIONS];
    uint16_t       m_nCount = 0;
};

// ── Combined serialiser — builds UDP packet from both accumulators ─────────
class cCycleAccumulator
{
public:
    cCycleAccumulator();

    // Returns true when a complete UDP packet is ready to send
    // (near complete OR far complete — send whichever just finished)
    bool addMessage(
        const RadarDecoded::tRDIMessage& msg,
        uint32_t                         nMessageID);

    const uint8_t* getSendBuffer()     const { return m_aSendBuffer;     }
    size_t         getSendBufferSize() const { return m_nSendBufferSize; }
    uint32_t       getCurrentCycle()   const { return m_nLastSentCycle;  }
    uint16_t       getNearCount()      const { return m_nLastNearCount;  }
    uint16_t       getFarCount()       const { return m_nLastFarCount;   }

private:
    void serialise(
        const tUDPRadarPoint* pNear, uint16_t nNearCount, uint32_t nNearTs,
        const tUDPRadarPoint* pFar,  uint16_t nFarCount,  uint32_t nFarTs);

    cNearAccumulator m_oNear;
    cFarAccumulator  m_oFar;

    uint8_t  m_aSendBuffer[MAX_UDP_PAYLOAD];
    size_t   m_nSendBufferSize = 0;
    uint32_t m_nLastSentCycle  = 0;
    uint16_t m_nLastNearCount  = 0;
    uint16_t m_nLastFarCount   = 0;
};

} // namespace RadarDecoder