// cycle_accumulator.cpp
#include "cycle_accumulator.h"
#include <cstring>
#include <cmath>
#include <algorithm>

namespace RadarDecoder
{

cCycleAccumulator::cCycleAccumulator()
{
    std::memset(m_aNearPoints,  0, sizeof(m_aNearPoints));
    std::memset(m_aFarPoints,   0, sizeof(m_aFarPoints));
    std::memset(m_aSendBuffer,  0, sizeof(m_aSendBuffer));
}

void cCycleAccumulator::reset(uint32_t nNewCycle)
{
    m_nCurrentCycle = nNewCycle;
    m_nTimestampUs  = 0;
    m_bNear0 = false;
    m_bNear1 = false;
    m_bNear2 = false;
    m_bFar0  = false;
    m_bFar1  = false;
    m_nNearCount    = 0;
    m_nFarCount     = 0;
    m_nSendBufferSize = 0;
}

tUDPRadarPoint cCycleAccumulator::convertPoint(
    const RadarDecoded::tRDIDetection& det)
{
    tUDPRadarPoint p{};

    // Spherical to Cartesian
    // Radar convention: X=forward, Y=left, Z=up
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

void cCycleAccumulator::appendDetections(
    tUDPRadarPoint* pDest,
    uint16_t&       nCount,
    const RadarDecoded::tRDIMessage& msg)
{
    // Store timestamp from first sub-message of cycle
    if (m_nTimestampUs == 0)
        m_nTimestampUs = msg.nTimeStamp;

    const uint16_t nValid = std::min(
        msg.nNbOfDetections,
        static_cast<uint16_t>(msg.nArraySize));

    for (uint16_t i = 0; i < nValid; ++i)
    {
        const RadarDecoded::tRDIDetection& det = msg.aDetections[i];

        // Filter: skip invalid signal status detections
        // (checked at message level in addMessage)

        // Filter: skip detections with any false detection flag set
        if (det.nPdh0 != 0) continue;

        // Filter: skip low SNR detections (threshold: 10 dB)
        if (det.fSNR < 10.0f) continue;

        pDest[nCount++] = convertPoint(det);
    }
}

bool cCycleAccumulator::addMessage(
    const RadarDecoded::tRDIMessage& msg,
    uint32_t                         nMessageID)
{
    // New cycle detected — reset and start fresh
    if (msg.nCycleCounter != m_nCurrentCycle)
        reset(msg.nCycleCounter);

    // Skip entire message if signal status is not OK
    if (msg.eSignalStatus != RadarTypes::tSignalStatus::SIGNALSTATUS_OK)
        return false;

    // Deduplication + routing
    switch (nMessageID)
    {
        case RadarTypes::MESSAGEID_RDINEAR_0:
            if (m_bNear0) return false;
            m_bNear0 = true;
            appendDetections(m_aNearPoints, m_nNearCount, msg);
            break;

        case RadarTypes::MESSAGEID_RDINEAR_1:
            if (m_bNear1) return false;
            m_bNear1 = true;
            appendDetections(m_aNearPoints, m_nNearCount, msg);
            break;

        case RadarTypes::MESSAGEID_RDINEAR_2:
            if (m_bNear2) return false;
            m_bNear2 = true;
            appendDetections(m_aNearPoints, m_nNearCount, msg);
            break;

        case RadarTypes::MESSAGEID_RDIFAR_0:
            if (m_bFar0) return false;
            m_bFar0 = true;
            appendDetections(m_aFarPoints, m_nFarCount, msg);
            break;

        case RadarTypes::MESSAGEID_RDIFAR_1:
            if (m_bFar1) return false;
            m_bFar1 = true;
            appendDetections(m_aFarPoints, m_nFarCount, msg);
            break;

        default:
            return false;
    }

    if (!isCycleComplete())
        return false;

    // All 5 sub-messages received — serialise and signal ready
    serialise();
    return true;
}

bool cCycleAccumulator::isCycleComplete() const
{
    return m_bNear0 && m_bNear1 && m_bNear2
        && m_bFar0  && m_bFar1;
}

void cCycleAccumulator::serialise()
{
    // Build header
    tUDPRadarHeader oHeader{};
    oHeader.nMagic       = RADAR_UDP_MAGIC;
    oHeader.nCycle       = m_nCurrentCycle;
    oHeader.nTimestampUs = m_nTimestampUs;
    oHeader.nNearCount   = m_nNearCount;
    oHeader.nFarCount    = m_nFarCount;

    // Write header
    uint8_t* pCursor = m_aSendBuffer;
    std::memcpy(pCursor, &oHeader, sizeof(tUDPRadarHeader));
    pCursor += sizeof(tUDPRadarHeader);

    // Write near points
    const size_t nNearBytes = m_nNearCount * sizeof(tUDPRadarPoint);
    std::memcpy(pCursor, m_aNearPoints, nNearBytes);
    pCursor += nNearBytes;

    // Write far points
    const size_t nFarBytes = m_nFarCount * sizeof(tUDPRadarPoint);
    std::memcpy(pCursor, m_aFarPoints, nFarBytes);
    pCursor += nFarBytes;

    m_nSendBufferSize = static_cast<size_t>(pCursor - m_aSendBuffer);
}

const uint8_t* cCycleAccumulator::getSendBuffer() const
{
    return m_aSendBuffer;
}

size_t cCycleAccumulator::getSendBufferSize() const
{
    return m_nSendBufferSize;
}

} // namespace RadarDecoder