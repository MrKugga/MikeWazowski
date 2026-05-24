#include "crc_utils.h"
#include <cstring>
#include <vector>
#include <algorithm>
#include <functional>

// LOG_INFO is provided by ADTF — include the filtersdk header
#include <adtffiltersdk/adtf_filtersdk.h>

namespace CRCUtils
{

// ── Algorithms ────────────────────────────────────────────────────────────

uint16_t crc16_ccitt_ffff(const uint8_t* p, size_t n)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < n; ++i) {
        crc ^= static_cast<uint16_t>(p[i]) << 8;
        for (int j = 0; j < 8; ++j)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

uint16_t crc16_ccitt_0000(const uint8_t* p, size_t n)
{
    uint16_t crc = 0x0000;
    for (size_t i = 0; i < n; ++i) {
        crc ^= static_cast<uint16_t>(p[i]) << 8;
        for (int j = 0; j < 8; ++j)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

uint16_t crc16_ibm(const uint8_t* p, size_t n)
{
    uint16_t crc = 0x0000;
    for (size_t i = 0; i < n; ++i) {
        crc ^= static_cast<uint16_t>(p[i]);
        for (int j = 0; j < 8; ++j)
            crc = (crc & 0x0001) ? (crc >> 1) ^ 0x8005 : crc >> 1;
    }
    return crc;
}

uint16_t crc16_autosar(const uint8_t* p, size_t n)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < n; ++i) {
        crc ^= static_cast<uint16_t>(p[i]);
        for (int j = 0; j < 8; ++j)
            crc = (crc & 0x0001) ? (crc >> 1) ^ 0x8408 : crc >> 1;
    }
    return crc;
}

uint16_t crc16_wordwise(const uint8_t* p, size_t n)
{
    uint16_t crc = 0xFFFF;
    size_t i = 0;
    for (; i + 1 < n; i += 2) {
        uint16_t w = static_cast<uint16_t>((p[i] << 8) | p[i + 1]);
        crc ^= w;
        for (int j = 0; j < 16; ++j)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    if (i < n) {
        uint16_t w = static_cast<uint16_t>(p[i] << 8);
        crc ^= w;
        for (int j = 0; j < 16; ++j)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

uint16_t crc32_low16(const uint8_t* p, size_t n)
{
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < n; ++i) {
        crc ^= p[i];
        for (int j = 0; j < 8; ++j)
            crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320 : crc >> 1;
    }
    return static_cast<uint16_t>((crc ^ 0xFFFFFFFF) & 0xFFFF);
}

uint16_t crc32_high16(const uint8_t* p, size_t n)
{
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < n; ++i) {
        crc ^= p[i];
        for (int j = 0; j < 8; ++j)
            crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320 : crc >> 1;
    }
    return static_cast<uint16_t>(((crc ^ 0xFFFFFFFF) >> 16) & 0xFFFF);
}

uint16_t fletcher16(const uint8_t* p, size_t n)
{
    uint16_t s1 = 0, s2 = 0;
    for (size_t i = 0; i < n; ++i) {
        s1 = (s1 + p[i]) % 255;
        s2 = (s2 + s1)   % 255;
    }
    return (s2 << 8) | s1;
}

uint16_t additive16(const uint8_t* p, size_t n)
{
    uint16_t sum = 0;
    for (size_t i = 0; i < n; ++i) sum += p[i];
    return sum;
}

uint16_t xor16(const uint8_t* p, size_t n)
{
    uint16_t x = 0;
    for (size_t i = 0; i < n; ++i) x ^= p[i];
    return x;
}

uint16_t crc8_e2e(const uint8_t* p, size_t n)
{
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < n; ++i) {
        crc ^= p[i];
        for (int j = 0; j < 8; ++j)
            crc = (crc & 0x80) ? (crc << 1) ^ 0x1D : crc << 1;
    }
    return static_cast<uint16_t>(crc);
}

uint16_t internet_checksum(const uint8_t* p, size_t n)
{
    uint32_t sum = 0;
    size_t i = 0;
    for (; i + 1 < n; i += 2)
        sum += static_cast<uint16_t>((p[i] << 8) | p[i + 1]);
    if (i < n)
        sum += static_cast<uint16_t>(p[i] << 8);
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    return static_cast<uint16_t>(~sum);
}

// ── Verification ──────────────────────────────────────────────────────────

bool verifyCRC(
    const uint8_t* pData,
    size_t         nLen,
    uint16_t       nExpectedCRC,
    AlgoFn         fnAlgorithm)
{
    return fnAlgorithm(pData, nLen) == nExpectedCRC;
}

bool verifyInternetChecksum(const uint8_t* pData, size_t nLen)
{
    return internet_checksum(pData, nLen) == 0x0000;
}

// ── Internal helpers ──────────────────────────────────────────────────────

namespace
{

// Read big-endian uint16 at offset
static uint16_t readBE16(const uint8_t* p, size_t nOffset)
{
    uint16_t v = 0;
    std::memcpy(&v, p + nOffset, sizeof(uint16_t));
    return static_cast<uint16_t>((v << 8) | (v >> 8));
}

// Read big-endian uint32 at offset
static uint32_t readBE32(const uint8_t* p, size_t nOffset)
{
    uint32_t v = 0;
    std::memcpy(&v, p + nOffset, sizeof(uint32_t));
    return ((v & 0x000000FFu) << 24) |
           ((v & 0x0000FF00u) <<  8) |
           ((v & 0x00FF0000u) >>  8) |
           ((v & 0xFF000000u) >> 24);
}

// Clamp length so pStart + nLen never exceeds pPacket + nTotalLen
static size_t clamp(
    const uint8_t* pPacket,
    size_t         nTotalLen,
    const uint8_t* pStart,
    size_t         nLen)
{
    if (pStart >= pPacket + nTotalLen) return 0;
    const size_t nMax = static_cast<size_t>((pPacket + nTotalLen) - pStart);
    return std::min(nLen, nMax);
}

struct tAlgoEntry
{
    const char*                                    pName;
    std::function<uint16_t(const uint8_t*, size_t)> fn;
};

struct tRangeEntry
{
    const char*    pName;
    const uint8_t* pStart;
    size_t         nLen;
};

static const tAlgoEntry* buildAlgoTable()
{
    static const tAlgoEntry aAlgos[] =
    {
        { "CRC16/CCITT  init=0xFFFF",   crc16_ccitt_ffff  },
        { "CRC16/CCITT  init=0x0000",   crc16_ccitt_0000  },
        { "CRC16/IBM    init=0x0000",   crc16_ibm         },
        { "CRC16/AUTOSAR init=0xFFFF",  crc16_autosar     },
        { "CRC16 word-wise",            crc16_wordwise    },
        { "Fletcher-16",                fletcher16        },
        { "CRC32 low  16 bits",         crc32_low16       },
        { "CRC32 high 16 bits",         crc32_high16      },
        { "Additive16",                 additive16        },
        { "XOR16",                      xor16             },
        { "CRC8/E2E (low byte)",        crc8_e2e          },
        { "Internet checksum",          internet_checksum },
    };
    return aAlgos;
}

static constexpr size_t ALGO_COUNT = 12;

} // anonymous namespace

// ── Debug functions ───────────────────────────────────────────────────────

void checkCRCAcrossPackets(
    const uint8_t* pPacket,
    size_t         nTotalLen,
    size_t         nSOMEIPHeaderSize,
    uint32_t       nPacketIndex)
{
    if (nTotalLen < nSOMEIPHeaderSize + 5)
    {
        LOG_WARNING("[CRCUtils] checkCRCAcrossPackets: packet too short (%zu bytes)",
            nTotalLen);
        return;
    }

    const uint8_t* pPayload = pPacket + nSOMEIPHeaderSize;

    uint16_t nServiceID = readBE16(pPacket, 0);
    uint16_t nMethodID  = readBE16(pPacket, 2);

    uint16_t nRawCRC = 0;
    std::memcpy(&nRawCRC, pPayload, sizeof(uint16_t));

    const uint8_t nSQC = pPayload[4];

    LOG_INFO("[CRCUtils pkt %04u]  ServiceID=%u  MethodID=%u  "
             "nCRC raw=0x%04X  swapped=0x%04X  nSQC=%u  totalLen=%zu",
        nPacketIndex,
        nServiceID,
        nMethodID,
        nRawCRC,
        static_cast<uint16_t>((nRawCRC << 8) | (nRawCRC >> 8)),
        nSQC,
        nTotalLen);
}

void compareKnownCRC(
    const uint8_t* pPacket,
    size_t         nTotalLen,
    size_t         nSOMEIPHeaderSize,
    size_t         nPayloadHeaderSize)
{
    const uint8_t* pPayload  = pPacket + nSOMEIPHeaderSize;
    const uint8_t* pAfterCRC = pPayload + sizeof(uint16_t);

    uint16_t nRawCRC     = 0;
    uint16_t nRawLen     = 0;
    std::memcpy(&nRawCRC, pPayload + 0, 2);
    std::memcpy(&nRawLen, pPayload + 2, 2);

    const uint16_t nHostCRC = static_cast<uint16_t>((nRawCRC << 8) | (nRawCRC >> 8));
    const uint16_t nHostLen = static_cast<uint16_t>((nRawLen << 8) | (nRawLen >> 8));
    const uint8_t  nSQC     = pPayload[4];

    // Compute safe length from offset 18
    const size_t nMaxFromAfterCRC = nTotalLen > nSOMEIPHeaderSize + sizeof(uint16_t)
        ? nTotalLen - nSOMEIPHeaderSize - sizeof(uint16_t)
        : 0;
    const size_t nSafeLen = std::min(static_cast<size_t>(nHostLen), nMaxFromAfterCRC);

    LOG_INFO("── compareKnownCRC ──────────────────────────────────────");
    LOG_INFO("  Total packet size:           %zu bytes", nTotalLen);
    LOG_INFO("  SOME/IP header size:         %zu bytes", nSOMEIPHeaderSize);
    LOG_INFO("  Payload header size:         %zu bytes", nPayloadHeaderSize);
    LOG_INFO("  nCRC raw BE=0x%04X  host=0x%04X  (%u)", nRawCRC, nHostCRC, nHostCRC);
    LOG_INFO("  nLen raw BE=0x%04X  host=%u",            nRawLen, nHostLen);
    LOG_INFO("  nLen clamped to buffer:      %zu bytes", nSafeLen);
    LOG_INFO("  nSQC:                        %u",        nSQC);

    if (nHostCRC == 0x0000)
        LOG_INFO("  *** WARNING: nCRC=0x0000 — sender may not implement CRC ***");
    else if (nHostCRC == 0xFFFF)
        LOG_INFO("  *** WARNING: nCRC=0xFFFF — possible uninitialized field ***");
    else
        LOG_INFO("  nCRC is non-trivial — CRC is populated by sender");

    // Raw payload header bytes
    LOG_INFO("  Payload header raw bytes:");
    for (size_t i = 0; i < nPayloadHeaderSize; ++i)
        LOG_INFO("    [packet offset %02zu] = 0x%02X  (%3u)",
            nSOMEIPHeaderSize + i,
            pPayload[i],
            pPayload[i]);

    // First 16 bytes of primary candidate range
    const size_t nDumpFront = std::min(size_t(16), nSafeLen);
    LOG_INFO("  First %zu bytes of primary CRC range (offset %zu onward):",
        nDumpFront, nSOMEIPHeaderSize + sizeof(uint16_t));
    for (size_t i = 0; i < nDumpFront; ++i)
        LOG_INFO("    [packet offset %02zu] = 0x%02X  (%3u)",
            nSOMEIPHeaderSize + sizeof(uint16_t) + i,
            pAfterCRC[i],
            pAfterCRC[i]);

    // Last 4 bytes of primary candidate range
    if (nSafeLen >= 4)
    {
        LOG_INFO("  Last 4 bytes of primary CRC range:");
        for (size_t i = nSafeLen - 4; i < nSafeLen; ++i)
            LOG_INFO("    [packet offset %02zu] = 0x%02X  (%3u)",
                nSOMEIPHeaderSize + sizeof(uint16_t) + i,
                pAfterCRC[i],
                pAfterCRC[i]);
    }
}

void debugAllCRCCombinations(
    const uint8_t* pPacket,
    size_t         nTotalLen,
    size_t         nSOMEIPHeaderSize,
    size_t         nPayloadHeaderSize)
{
    const uint8_t* pPayload      = pPacket + nSOMEIPHeaderSize;
    const uint8_t* pFromClientID = pPacket + 8;               // SOME/IP: after first 8 bytes
    const uint8_t* pFromNLen     = pPayload + sizeof(uint16_t); // after nCRC
    const uint8_t* pFromNSQC    = pPayload + sizeof(uint16_t) + sizeof(uint16_t); // after nCRC+nLen
    const uint8_t* pFromAfterHdr = pPayload + nPayloadHeaderSize;

    // Read fields
    uint16_t nRawCRC = 0, nRawLen = 0;
    std::memcpy(&nRawCRC, pPayload + 0, 2);
    std::memcpy(&nRawLen, pPayload + 2, 2);

    const uint16_t nExpected    = static_cast<uint16_t>((nRawCRC << 8) | (nRawCRC >> 8));
    const uint16_t nPayloadLen  = static_cast<uint16_t>((nRawLen << 8) | (nRawLen >> 8));
    const uint32_t nSOMEIPLen   = readBE32(pPacket, 4);

    // Safe lengths — clamp all to buffer
    auto safeLen = [&](const uint8_t* pStart, size_t nLen) -> size_t {
        return clamp(pPacket, nTotalLen, pStart, nLen);
    };

    const size_t nLenFromClientID  = safeLen(pFromClientID, static_cast<size_t>(nSOMEIPLen));
    const size_t nLenFromPayload   = safeLen(pPayload, nTotalLen - nSOMEIPHeaderSize);
    const size_t nLenFromNLen      = safeLen(pFromNLen,
        std::min(static_cast<size_t>(nPayloadLen),
                 nTotalLen - nSOMEIPHeaderSize - sizeof(uint16_t)));
    const size_t nLenFromNSQC      = safeLen(pFromNSQC,
        nTotalLen - nSOMEIPHeaderSize - sizeof(uint16_t) - sizeof(uint16_t));
    const size_t nLenFromAfterHdr  = safeLen(pFromAfterHdr,
        nTotalLen - nSOMEIPHeaderSize - nPayloadHeaderSize);
    const size_t nLenFull          = nTotalLen;

    LOG_INFO("════════════════════════════════════════════════════════");
    LOG_INFO("  CRC DEBUG SESSION");
    LOG_INFO("  Total packet:    %zu bytes", nTotalLen);
    LOG_INFO("  SOME/IP nLength: %u (from ClientID to end)", nSOMEIPLen);
    LOG_INFO("  Payload nLen:    %u (clamped: %zu)", nPayloadLen, nLenFromNLen);
    LOG_INFO("  Expected CRC:    0x%04X (host order)", nExpected);
    LOG_INFO("════════════════════════════════════════════════════════");

    // ── Hex dump ─────────────────────────────────────────────────────────
    LOG_INFO("── Hex dump: first 32 bytes ─────────────────────────────");
    for (size_t i = 0; i < std::min(size_t(32), nTotalLen); ++i)
        LOG_INFO("  [%04zu]  0x%02X  %3u  '%c'",
            i, pPacket[i], pPacket[i],
            (pPacket[i] >= 32 && pPacket[i] < 127) ? (char)pPacket[i] : '.');

    LOG_INFO("── Hex dump: last 8 bytes ───────────────────────────────");
    const size_t nTailStart = nTotalLen > 8 ? nTotalLen - 8 : 0;
    for (size_t i = nTailStart; i < nTotalLen; ++i)
        LOG_INFO("  [%04zu]  0x%02X  %3u", i, pPacket[i], pPacket[i]);

    // ── SOME/IP header parse ──────────────────────────────────────────────
    LOG_INFO("── SOME/IP header ───────────────────────────────────────");
    LOG_INFO("  ServiceID:         0x%04X  (%u)", readBE16(pPacket,  0), readBE16(pPacket,  0));
    LOG_INFO("  MethodID:          0x%04X  (%u)", readBE16(pPacket,  2), readBE16(pPacket,  2));
    LOG_INFO("  Length:            0x%08X (%u)", nSOMEIPLen,             nSOMEIPLen);
    LOG_INFO("  ClientID:          0x%04X  (%u)", readBE16(pPacket,  8), readBE16(pPacket,  8));
    LOG_INFO("  SessionID:         0x%04X  (%u)", readBE16(pPacket, 10), readBE16(pPacket, 10));
    LOG_INFO("  ProtocolVersion:   0x%02X   (%u)", pPacket[12],           pPacket[12]);
    LOG_INFO("  InterfaceVersion:  0x%02X   (%u)", pPacket[13],           pPacket[13]);
    LOG_INFO("  MsgType:           0x%02X   (%u)", pPacket[14],           pPacket[14]);
    LOG_INFO("  ReturnCode:        0x%02X   (%u)", pPacket[15],           pPacket[15]);

    // ── Payload header parse ──────────────────────────────────────────────
    LOG_INFO("── Payload header (offset %zu) ──────────────────────────",
        nSOMEIPHeaderSize);
    for (size_t i = 0; i < nPayloadHeaderSize; ++i)
        LOG_INFO("  [%04zu]  0x%02X  %3u",
            nSOMEIPHeaderSize + i, pPayload[i], pPayload[i]);

    // ── Candidate ranges ──────────────────────────────────────────────────
    const tRangeEntry aRanges[] =
    {
        { "PRIMARY: nLen bytes after nCRC",
            pFromNLen,     nLenFromNLen     },
        { "nSOMEIPLen bytes from ClientID",
            pFromClientID, nLenFromClientID },
        { "after nCRC+nLen (from nSQC)",
            pFromNSQC,     nLenFromNSQC     },
        { "after full payload header",
            pFromAfterHdr, nLenFromAfterHdr },
        { "full payload incl. nCRC",
            pPayload,      nLenFromPayload  },
        { "full packet",
            pPacket,       nLenFull         },
    };
    constexpr size_t RANGE_COUNT = sizeof(aRanges) / sizeof(aRanges[0]);

    LOG_INFO("── Candidate ranges ─────────────────────────────────────");
    for (size_t r = 0; r < RANGE_COUNT; ++r)
        LOG_INFO("  [%s]  offset=%zu  len=%zu",
            aRanges[r].pName,
            static_cast<size_t>(aRanges[r].pStart - pPacket),
            aRanges[r].nLen);

    // ── Zeroed-field helper ───────────────────────────────────────────────
    auto makeZeroed = [&](
        const std::function<uint16_t(const uint8_t*, size_t)>& fn,
        const uint8_t* pStart,
        size_t         nLen) -> uint16_t
    {
        std::vector<uint8_t> buf(pPacket, pPacket + nTotalLen);
        buf[nSOMEIPHeaderSize + 0] = 0x00;
        buf[nSOMEIPHeaderSize + 1] = 0x00;
        const size_t nOffset = static_cast<size_t>(pStart - pPacket);
        return fn(buf.data() + nOffset, nLen);
    };

    // ── Standard sweep ────────────────────────────────────────────────────
    const tAlgoEntry* aAlgos = buildAlgoTable();

    LOG_INFO("── Standard sweep  (expected=0x%04X) ────────────────────",
        nExpected);

    bool bFoundAny = false;
    for (size_t r = 0; r < RANGE_COUNT; ++r)
    {
        for (size_t a = 0; a < ALGO_COUNT; ++a)
        {
            const uint16_t nNormal = aAlgos[a].fn(
                aRanges[r].pStart, aRanges[r].nLen);
            if (nNormal == nExpected)
            {
                LOG_INFO("  *** MATCH ***  range=[%-40s]  algo=[%-25s]  =0x%04X",
                    aRanges[r].pName, aAlgos[a].pName, nNormal);
                bFoundAny = true;
            }

            const uint16_t nZeroed = makeZeroed(
                aAlgos[a].fn, aRanges[r].pStart, aRanges[r].nLen);
            if (nZeroed == nExpected)
            {
                LOG_INFO("  *** MATCH (zeroed nCRC) ***  range=[%-40s]  algo=[%-25s]  =0x%04X",
                    aRanges[r].pName, aAlgos[a].pName, nZeroed);
                bFoundAny = true;
            }
        }
    }

     // ── Sliding window ────────────────────────────────────────────────────
    LOG_INFO("── Sliding window (±16 bytes around PRIMARY) ────────────");

    const int nSlide = 16;
    for (int nSD = -nSlide; nSD <= nSlide; ++nSD)
    {
        const uint8_t* pTry = pFromNLen + nSD;
        if (pTry < pPacket || pTry >= pPacket + nTotalLen) continue;

        for (int nLD = -nSlide; nLD <= nSlide; ++nLD)
        {
            const int nTrySigned = static_cast<int>(nLenFromNLen) + nLD;
            if (nTrySigned <= 0) continue;
            const size_t nTryLen = static_cast<size_t>(nTrySigned);
            if (pTry + nTryLen > pPacket + nTotalLen) continue;

            for (size_t a = 0; a < ALGO_COUNT; ++a)
            {
                const uint16_t nResult = aAlgos[a].fn(pTry, nTryLen);
                if (nResult == nExpected)
                {
                    LOG_INFO("  *** SLIDING MATCH ***  "
                        "offset=%zu (delta=%+d)  len=%zu (delta=%+d)  "
                        "algo=[%s]  =0x%04X",
                        static_cast<size_t>(pTry - pPacket), nSD,
                        nTryLen, nLD,
                        aAlgos[a].pName, nResult);
                    bFoundAny = true;
                }
            }
        }
    }

    // ── Internet checksum verification ────────────────────────────────────
    LOG_INFO("── Internet checksum (result must be 0x0000 if valid) ───");
    for (size_t r = 0; r < RANGE_COUNT; ++r)
    {
        const uint16_t nResult = internet_checksum(
            aRanges[r].pStart, aRanges[r].nLen);
        LOG_INFO("  [%-40s] = 0x%04X  %s",
            aRanges[r].pName, nResult,
            (nResult == 0x0000) ? "*** VALID ***" : "");
        if (nResult == 0x0000) bFoundAny = true;
    }

    // ── Full results table if no match ────────────────────────────────────
    if (!bFoundAny)
    {
        LOG_INFO("── No match — full results table ────────────────────────");
        for (size_t r = 0; r < RANGE_COUNT; ++r)
        {
            LOG_INFO("  range=[%s]  offset=%zu  len=%zu",
                aRanges[r].pName,
                static_cast<size_t>(aRanges[r].pStart - pPacket),
                aRanges[r].nLen);
            for (size_t a = 0; a < ALGO_COUNT; ++a)
            {
                const uint16_t nN = aAlgos[a].fn(
                    aRanges[r].pStart, aRanges[r].nLen);
                const uint16_t nZ = makeZeroed(
                    aAlgos[a].fn, aRanges[r].pStart, aRanges[r].nLen);
                LOG_INFO("    [%-25s]  normal=0x%04X  zeroed=0x%04X",
                    aAlgos[a].pName, nN, nZ);
            }
        }
    }

    LOG_INFO("════════════════════════════════════════════════════════");
    LOG_INFO("  END CRC DEBUG SESSION");
    LOG_INFO("════════════════════════════════════════════════════════");
}

} // namespace CRCUtils