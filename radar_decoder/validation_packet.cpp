#include "validation_packet.h"
#include <cstring>
#include <algorithm>

namespace PacketValidator
{

size_t expectedTotalSize(uint32_t nMessageID)
{
    switch (nMessageID)
    {
        case RadarTypes::MESSAGEID_RDINEAR_0:
            return sizeof(RadarTypes::tSOMEIPHeader)
                 + sizeof(RadarTypes::tRDI_Near_Message_0);
        case RadarTypes::MESSAGEID_RDINEAR_1:
            return sizeof(RadarTypes::tSOMEIPHeader)
                 + sizeof(RadarTypes::tRDI_Near_Message_1);
        case RadarTypes::MESSAGEID_RDINEAR_2:
            return sizeof(RadarTypes::tSOMEIPHeader)
                 + sizeof(RadarTypes::tRDI_Near_Message_2);
        case RadarTypes::MESSAGEID_RDIFAR_0:
            return sizeof(RadarTypes::tSOMEIPHeader)
                 + sizeof(RadarTypes::tRDI_Far_Message_0);
        case RadarTypes::MESSAGEID_RDIFAR_1:
            return sizeof(RadarTypes::tSOMEIPHeader)
                 + sizeof(RadarTypes::tRDI_Far_Message_1);
        case RadarTypes::MESSAGEID_OBJECTS_0:
            return sizeof(RadarTypes::tSOMEIPHeader)
                 + sizeof(RadarTypes::tObject_Message_0);
        case RadarTypes::MESSAGEID_OBJECTS_1:
            return sizeof(RadarTypes::tSOMEIPHeader)
                 + sizeof(RadarTypes::tObject_Message_1);
        case RadarTypes::MESSAGEID_SENSORSTATUS:
            return sizeof(RadarTypes::tSOMEIPHeader)
                 + sizeof(RadarTypes::tSensorStatus_Message);
        case RadarTypes::MESSAGEID_VEHDYN:
            return sizeof(RadarTypes::tSOMEIPHeader)
                 + sizeof(RadarTypes::tVehicleDynamics_Message);
        case RadarTypes::MESSAGEID_SENSORCONFIG:
            return sizeof(RadarTypes::tSOMEIPHeader)
                 + sizeof(RadarTypes::tSensorConfig_Message);
        default:
            return 0;
    }
}

EValidationResult validateSOMEIPPacket(
    const uint8_t* pData,
    size_t         nLen,
    uint32_t&      nMessageIDOut)
{
    // 1 Buffer large enough for SOME/IP header?
    if (nLen < sizeof(RadarTypes::tSOMEIPHeader))
        return EValidationResult::ERR_BUFFER_TOO_SHORT;

    // 2 Parse service ID and method ID (big-endian)
    uint16_t nServiceID = 0;
    uint16_t nMethodID  = 0;
    std::memcpy(&nServiceID, pData + 0, sizeof(uint16_t));
    std::memcpy(&nMethodID,  pData + 2, sizeof(uint16_t));
    nServiceID = __builtin_bswap16(nServiceID);
    nMethodID  = __builtin_bswap16(nMethodID);

    // 3 Known message ID?
    nMessageIDOut = (uint32_t)nServiceID << 16 | nMethodID;

    const size_t nExpectedTotal = expectedTotalSize(nMessageIDOut);
    if (nExpectedTotal == 0)
        return EValidationResult::ERR_UNKNOWN_MESSAGE_ID;

    // 4 Buffer large enough for full packet?
    if (nLen < nExpectedTotal)
        return EValidationResult::ERR_PAYLOAD_TOO_SHORT;

     // CRC check intentionally skipped — field not reliably implemented by sensor

    return EValidationResult::OK;
}

} // namespace PacketValidator