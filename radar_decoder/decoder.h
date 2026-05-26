#pragma once
#include "decoded_messages.h"
#include "../assignment/RadarTypes.h"
#include "../assignment/EgoMaster3DOutput.h"
#include "validator.h" 
#include "validation_result.h"

namespace RadarDecoder
{

// pData points to the start of tSOMEIPHeader.
// Returns ERR_UNKNOWN_MESSAGE_ID if message ID is not in our table.
EValidationResult decodeRadar(
    const uint8_t*                  pData,
    size_t                          nLen,
    RadarDecoded::DecodedMessage&   oOutput,
    uint32_t&                       nMessageIDOut);


// Decodes raw tEgoMaster3DData bytes into tVehicleDynamicsDecoded
bool decodeEgomotion(
    const uint8_t*                         pData,
    size_t                                 nLen,
    RadarDecoded::tVehicleDynamicsDecoded& oOutput);

bool encodeVehicleDynamics(
    const RadarDecoded::tVehicleDynamicsDecoded&  oEgo,
    RadarTypes::tVehicleDynamics_Message&         oMsg,
    RadarTypes::tSOMEIPHeader&                    oSOMEIPHeader);

} // namespace RadarDecoder




