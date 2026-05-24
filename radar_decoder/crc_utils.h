#pragma once
#include <cstdint>
#include <cstddef>

namespace CRCUtils
{

// ── Individual algorithms ─────────────────────────────────────────────────

uint16_t crc16_ccitt_ffff  (const uint8_t* pData, size_t nLen);
uint16_t crc16_ccitt_0000  (const uint8_t* pData, size_t nLen);
uint16_t crc16_ibm         (const uint8_t* pData, size_t nLen);
uint16_t crc16_autosar     (const uint8_t* pData, size_t nLen);
uint16_t crc16_wordwise    (const uint8_t* pData, size_t nLen);
uint16_t crc32_low16       (const uint8_t* pData, size_t nLen);
uint16_t crc32_high16      (const uint8_t* pData, size_t nLen);
uint16_t fletcher16        (const uint8_t* pData, size_t nLen);
uint16_t additive16        (const uint8_t* pData, size_t nLen);
uint16_t xor16             (const uint8_t* pData, size_t nLen);
uint16_t crc8_e2e          (const uint8_t* pData, size_t nLen);
uint16_t internet_checksum (const uint8_t* pData, size_t nLen);

// ── Verification ──────────────────────────────────────────────────────────

// Once the correct algorithm is found, use this in production.
// Pass the confirmed algorithm via function pointer.
using AlgoFn = uint16_t (*)(const uint8_t*, size_t);

bool verifyCRC(
    const uint8_t* pData,
    size_t         nLen,
    uint16_t       nExpectedCRC,
    AlgoFn         fnAlgorithm);

// Internet checksum verify form — includes nCRC field, result must be 0x0000
bool verifyInternetChecksum(const uint8_t* pData, size_t nLen);

// ── Debug ─────────────────────────────────────────────────────────────────

// Runs all algorithms over all candidate ranges and logs results.
// Only call this during debug — do not leave in production code.
void debugAllCRCCombinations(
    const uint8_t* pPacket,
    size_t         nTotalLen,
    size_t         nSOMEIPHeaderSize,
    size_t         nPayloadHeaderSize);

// Logs nCRC + nSQC for a single packet — call on first N packets to
// check if CRC field is populated and nSQC increments as expected.
void checkCRCAcrossPackets(
    const uint8_t* pPacket,
    size_t         nTotalLen,
    size_t         nSOMEIPHeaderSize,
    uint32_t       nPacketIndex);

// Logs detailed byte inspection of payload header + first/last bytes
// of the primary CRC candidate range.
void compareKnownCRC(
    const uint8_t* pPacket,
    size_t         nTotalLen,
    size_t         nSOMEIPHeaderSize,
    size_t         nPayloadHeaderSize);

} // namespace CRCUtils