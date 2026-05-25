#pragma once
#include <cstdint>
#include <cstddef>
#include "../assignment/RadarTypes.h"
#include "validation_result.h"



namespace PacketValidator
{
// Returns a human-readable name for a known message ID
inline const char* messageIDToString(uint32_t nMessageID)
{
    switch (nMessageID)
    {
        case RadarTypes::MESSAGEID_RDINEAR_0:    return "RDINEAR_0";
        case RadarTypes::MESSAGEID_RDINEAR_1:    return "RDINEAR_1";
        case RadarTypes::MESSAGEID_RDINEAR_2:    return "RDINEAR_2";
        case RadarTypes::MESSAGEID_RDIFAR_0:     return "RDIFAR_0";
        case RadarTypes::MESSAGEID_RDIFAR_1:     return "RDIFAR_1";
        case RadarTypes::MESSAGEID_OBJECTS_0:    return "OBJECTS_0";
        case RadarTypes::MESSAGEID_OBJECTS_1:    return "OBJECTS_1";
        case RadarTypes::MESSAGEID_SENSORSTATUS: return "SENSORSTATUS";
        case RadarTypes::MESSAGEID_VEHDYN:       return "VEHDYN";
        case RadarTypes::MESSAGEID_SENSORCONFIG: return "SENSORCONFIG";
        default:                                 return "UNKNOWN";
    }
}

// Expected total packet size (tSOMEIPHeader + payload struct) per message ID.
// Returns 0 if the message ID is unknown.
size_t expectedTotalSize(uint32_t nMessageID);

// Main validation entry point.
// pData points to the start of tSOMEIPHeader.
// On success, nMessageIDOut is populated with the 32-bit message ID.
EValidationResult validateSOMEIPPacket(
    const uint8_t* pData,
    size_t         nLen,
    uint32_t&      nMessageIDOut);

} // namespace PacketValidator