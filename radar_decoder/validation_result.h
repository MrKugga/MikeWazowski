#pragma once
#include <cstdint>

enum class EValidationResult : uint8_t
{
    OK = 0,
    ERR_BUFFER_TOO_SHORT,       // buffer smaller than SOME/IP header
    ERR_UNKNOWN_MESSAGE_ID,     // service+method pair not recognised
    ERR_SOMEIP_LENGTH,          // SOME/IP nLength field inconsistent with buffer
    ERR_PAYLOAD_TOO_SHORT,      // buffer too small for the expected payload struct
    ERR_PAYLOAD_LENGTH,         // tSOMEIPPayloadHeader.nLen inconsistent
    ERR_CRC,                    // CRC mismatch in tSOMEIPPayloadHeader
    ERR_SEQUENCE_COUNTER,       // nSQC not incrementing (optional check)
    ERR_FIELD_OUT_OF_RANGE      // a field value is physically impossible
};

inline const char* toString(EValidationResult eResult)
{
    switch (eResult)
    {
        case EValidationResult::OK:                     return "OK";
        case EValidationResult::ERR_BUFFER_TOO_SHORT:   return "buffer too short";
        case EValidationResult::ERR_UNKNOWN_MESSAGE_ID: return "unknown message ID";
        case EValidationResult::ERR_SOMEIP_LENGTH:      return "SOME/IP length mismatch";
        case EValidationResult::ERR_PAYLOAD_TOO_SHORT:  return "payload too short";
        case EValidationResult::ERR_PAYLOAD_LENGTH:     return "payload header length mismatch";
        case EValidationResult::ERR_CRC:                return "CRC failed";
        case EValidationResult::ERR_SEQUENCE_COUNTER:   return "sequence counter gap";
        case EValidationResult::ERR_FIELD_OUT_OF_RANGE: return "field out of range";
        default:                                        return "unknown";
    }
}