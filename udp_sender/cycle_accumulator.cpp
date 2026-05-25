#include "cycle_accumulator.h"
#include <adtffiltersdk/adtf_filtersdk.h>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace RadarDecoder
{

// ── Shared point conversion ───────────────────────────────────────────────

static tUDPRadarPoint convertPoint(const RadarDecoded::tRDIDetection& det)
{
    tUDPRadarPoint p{};
    const float fCosEl = std::cos(det.fElAng);
    p.fX        = det.fRange * fCosEl * std::cos(det.fAzAng0);
    p.fY        = det.fRange * fCosEl * std::sin(det.fAzAng0);
    p.fZ        = det.fRange * std::sin(det.fElAng);
    p.fVelocity = det.fVrelRad;
    p.fRCS      = det.fRCS0;
    p.fSNR      = det.fSNR;
    p.nPdh0     = det.nPdh0;
    return p;
}

static void appendDetections(
    tUDPRadarPoint* pDest,
    uint16_t&       nCount,
    uint16_t        nMaxCount,
    const RadarDecoded::tRDIMessage& msg)
{
    const uint16_t nValid = std::min(
        msg.nNbOfDetections,
        static_cast<uint16_t>(msg.nArraySize));

    for (uint16_t i = 0; i < nValid && nCount < nMaxCount; ++i)
    {
        const RadarDecoded::tRDIDetection& det = msg.aDetections[i];
        if (det.nPdh0 != 0)   continue;
        if (det.fSNR  < 5.0f) continue;
        pDest[nCount++] = convertPoint(det);
    }
}

// ── cNearAccumulator ──────────────────────────────────────────────────────

cNearAccumulator::cNearAccumulator()
{
    std::memset(m_aPoints, 0, sizeof(m_aPoints));
}

void cNearAccumulator::reset(uint32_t nNewCycle)
{
    // LOG_INFO("NearAccumulator: reset %u → %u  (was %d%d%d  count=%u)",
    //     m_nCurrentCycle, nNewCycle,
    //     m_bNear0, m_bNear1, m_bNear2, m_nCount);
    m_nCurrentCycle = nNewCycle;
    m_nTimestampUs  = 0;
    m_bNear0 = false;
    m_bNear1 = false;
    m_bNear2 = false;
    m_nCount = 0;
}

bool cNearAccumulator::addMessage(
    const RadarDecoded::tRDIMessage& msg,
    uint32_t                         nMessageID)
{
    if (msg.nCycleCounter != m_nCurrentCycle)
        reset(msg.nCycleCounter);

    if (msg.eSignalStatus != RadarTypes::tSignalStatus::SIGNALSTATUS_OK)
        return false;

    if (m_nTimestampUs == 0)
        m_nTimestampUs = msg.nTimeStamp;

    switch (nMessageID)
    {
        case RadarTypes::MESSAGEID_RDINEAR_0:
            if (m_bNear0) return false;
            m_bNear0 = true;
            appendDetections(m_aPoints, m_nCount,
                MAX_NEAR_DETECTIONS, msg);
            break;
        case RadarTypes::MESSAGEID_RDINEAR_1:
            if (m_bNear1) return false;
            m_bNear1 = true;
            appendDetections(m_aPoints, m_nCount,
                MAX_NEAR_DETECTIONS, msg);
            break;
        case RadarTypes::MESSAGEID_RDINEAR_2:
            if (m_bNear2) return false;
            m_bNear2 = true;
            appendDetections(m_aPoints, m_nCount,
                MAX_NEAR_DETECTIONS, msg);
            break;
        default:
            return false;
    }

    // LOG_INFO("NearAccumulator: %s accepted — state=%d%d%d count=%u",
    //     PacketValidator::messageIDToString(nMessageID),
    //     m_bNear0, m_bNear1, m_bNear2, m_nCount);

    return isComplete();
}

// ── cFarAccumulator ───────────────────────────────────────────────────────

cFarAccumulator::cFarAccumulator()
{
    std::memset(m_aPoints, 0, sizeof(m_aPoints));
}

void cFarAccumulator::reset(uint32_t nNewCycle)
{
    // LOG_INFO("FarAccumulator: reset %u → %u  (was %d%d  count=%u)",
    //     m_nCurrentCycle, nNewCycle,
    //     m_bFar0, m_bFar1, m_nCount);
    m_nCurrentCycle = nNewCycle;
    m_nTimestampUs  = 0;
    m_bFar0  = false;
    m_bFar1  = false;
    m_nCount = 0;
}

bool cFarAccumulator::addMessage(
    const RadarDecoded::tRDIMessage& msg,
    uint32_t                         nMessageID)
{
    if (msg.nCycleCounter != m_nCurrentCycle)
        reset(msg.nCycleCounter);

    if (msg.eSignalStatus != RadarTypes::tSignalStatus::SIGNALSTATUS_OK)
        return false;

    if (m_nTimestampUs == 0)
        m_nTimestampUs = msg.nTimeStamp;

    switch (nMessageID)
    {
        case RadarTypes::MESSAGEID_RDIFAR_0:
            if (m_bFar0) return false;
            m_bFar0 = true;
            appendDetections(m_aPoints, m_nCount,
                MAX_FAR_DETECTIONS, msg);
            break;
        case RadarTypes::MESSAGEID_RDIFAR_1:
            if (m_bFar1) return false;
            m_bFar1 = true;
            appendDetections(m_aPoints, m_nCount,
                MAX_FAR_DETECTIONS, msg);
            break;
        default:
            return false;
    }

    // LOG_INFO("FarAccumulator: %s accepted — state=%d%d count=%u",
    //     PacketValidator::messageIDToString(nMessageID),
    //     m_bFar0, m_bFar1, m_nCount);

    return isComplete();
}

// ── cCycleAccumulator ─────────────────────────────────────────────────────

cCycleAccumulator::cCycleAccumulator()
{
    std::memset(m_aSendBuffer, 0, sizeof(m_aSendBuffer));
}

bool cCycleAccumulator::addMessage(
    const RadarDecoded::tRDIMessage& msg,
    uint32_t                         nMessageID)
{
    // Route to correct sub-accumulator
    const bool bIsNear =
        nMessageID == RadarTypes::MESSAGEID_RDINEAR_0 ||
        nMessageID == RadarTypes::MESSAGEID_RDINEAR_1 ||
        nMessageID == RadarTypes::MESSAGEID_RDINEAR_2;

    const bool bIsFar =
        nMessageID == RadarTypes::MESSAGEID_RDIFAR_0 ||
        nMessageID == RadarTypes::MESSAGEID_RDIFAR_1;

    if (bIsNear)
    {
        if (!m_oNear.addMessage(msg, nMessageID))
            return false;

        // Near complete — check if far is also ready
        // If far has data from a nearby cycle, send combined
        // Otherwise send near-only packet
        const bool bFarReady = m_oFar.isComplete();

        serialise(
            m_oNear.getPoints(), m_oNear.getCount(), m_oNear.getTimestamp(),
            bFarReady ? m_oFar.getPoints() : nullptr,
            bFarReady ? m_oFar.getCount()  : 0,
            bFarReady ? m_oFar.getTimestamp() : 0);

        m_nLastSentCycle  = m_oNear.getCycle();
        m_nLastNearCount  = m_oNear.getCount();
        m_nLastFarCount   = bFarReady ? m_oFar.getCount() : 0;

        // Reset near — it has been consumed
        m_oNear.reset(0);
        if (bFarReady)
            m_oFar.reset(0);

        return true;
    }

    if (bIsFar)
    {
        if (!m_oFar.addMessage(msg, nMessageID))
            return false;

        // Far complete — check if near is also ready
        const bool bNearReady = m_oNear.isComplete();

        serialise(
            bNearReady ? m_oNear.getPoints() : nullptr,
            bNearReady ? m_oNear.getCount()  : 0,
            bNearReady ? m_oNear.getTimestamp() : 0,
            m_oFar.getPoints(), m_oFar.getCount(), m_oFar.getTimestamp());

        m_nLastSentCycle  = m_oFar.getCycle();
        m_nLastNearCount  = bNearReady ? m_oNear.getCount() : 0;
        m_nLastFarCount   = m_oFar.getCount();

        m_oFar.reset(0);
        if (bNearReady)
            m_oNear.reset(0);

        return true;
    }

    return false;
}

void cCycleAccumulator::serialise(
    const tUDPRadarPoint* pNear, uint16_t nNearCount, uint32_t nNearTs,
    const tUDPRadarPoint* pFar,  uint16_t nFarCount,  uint32_t nFarTs)
{
    tUDPRadarHeader oHeader{};
    oHeader.nMagic       = RADAR_UDP_MAGIC;
    oHeader.nCycle       = m_nLastSentCycle;
    oHeader.nTimestampUs = (nNearTs > 0) ? nNearTs : nFarTs;
    oHeader.nNearCount   = nNearCount;
    oHeader.nFarCount    = nFarCount;

    uint8_t* pCursor = m_aSendBuffer;
    std::memcpy(pCursor, &oHeader, sizeof(tUDPRadarHeader));
    pCursor += sizeof(tUDPRadarHeader);

    if (pNear && nNearCount > 0)
    {
        const size_t nBytes = nNearCount * sizeof(tUDPRadarPoint);
        std::memcpy(pCursor, pNear, nBytes);
        pCursor += nBytes;
    }

    if (pFar && nFarCount > 0)
    {
        const size_t nBytes = nFarCount * sizeof(tUDPRadarPoint);
        std::memcpy(pCursor, pFar, nBytes);
        pCursor += nBytes;
    }

    m_nSendBufferSize = static_cast<size_t>(pCursor - m_aSendBuffer);

    // LOG_INFO("Serialised: near=%u far=%u bufSize=%zu",
    //     nNearCount, nFarCount, m_nSendBufferSize);
}

} // namespace RadarDecoder