#pragma once
#include "decoded_messages.h"
#include "../assignment/RadarTypes.h"
#include "validator.h" 
#include "validation_result.h"

namespace RadarDecoder
{

// Main decode entry point.
// pData points to the start of tSOMEIPHeader.
// Returns ERR_UNKNOWN_MESSAGE_ID if message ID is not in our table.
EValidationResult decode(
    const uint8_t*                  pData,
    size_t                          nLen,
    RadarDecoded::DecodedMessage&   oOutput,
    uint32_t&                       nMessageIDOut);
} // namespace RadarDecoder