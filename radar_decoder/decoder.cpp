#include "decoder.h"
#include <cstring>
#include <algorithm>

namespace RadarDecoder
{

// ── Conversion helpers ────────────────────────────────────────────────────

static RadarDecoded::tPTPTimestamp convertPTP(
    const RadarTypes::tPTPTimestamp& r)
{
    RadarDecoded::tPTPTimestamp o{};
    o.nSecHi   = r.nPTPSec_Hi;
    o.nSecLo   = r.nPTPSec_Lo;
    o.nNanoSec = r.nPTPNanoSec;
    o.eSync    = r.ePTPSync;
    return o;
}

static RadarDecoded::tRDIDetection convertRDI(
    const RadarTypes::tRDI& r)
{
    RadarDecoded::tRDIDetection o{};

    o.fRange   = static_cast<float>(r.fRange)
                 * static_cast<float>(RadarTypes::RES_F_RANGE);
    o.fVrelRad = static_cast<float>(r.fVrelRad)
                 * static_cast<float>(RadarTypes::RES_F_VRELRAD);
    o.fAzAng0  = static_cast<float>(r.aAzAng.fAzAng[0])
                 * static_cast<float>(RadarTypes::RES_F_AZIMUTH);
    o.fAzAng1  = static_cast<float>(r.aAzAng.fAzAng[1])
                 * static_cast<float>(RadarTypes::RES_F_AZIMUTH);
    o.fElAng   = static_cast<float>(r.fElAng)
                 * static_cast<float>(RadarTypes::RES_F_ELEVATION);
    o.fRCS0    = static_cast<float>(r.aRCS.fRCS[0])
                 * static_cast<float>(RadarTypes::RES_F_RCS);
    o.fRCS1    = static_cast<float>(r.aRCS.fRCS[1])
                 * static_cast<float>(RadarTypes::RES_F_RCS);

    o.fRangeVar   = static_cast<float>(r.fRangeVar)
                    * static_cast<float>(RadarTypes::RES_F_RANGEVAR);
    o.fVrelRadVar = static_cast<float>(r.fVrelRadVar)
                    * static_cast<float>(RadarTypes::RES_F_VRELRADVAR);
    o.fAzAngVar   = static_cast<float>(r.fAzAngVar)
                    * static_cast<float>(RadarTypes::RES_F_AZIMUHTVAR);
    o.fElAngVar   = static_cast<float>(r.fElAngVar)
                    * static_cast<float>(RadarTypes::RES_F_ELEVATIONVAR);

    o.fSNR  = static_cast<float>(r.fSNR)
              * static_cast<float>(RadarTypes::RES_F_SNR);
    o.nPdh0 = r.nPdh0;

    return o;
}

static RadarDecoded::tObjectDecoded convertObject(
    const RadarTypes::tObject& r)
{
    RadarDecoded::tObjectDecoded o{};

    o.uObjId = r.uObjId;

    o.fDistX    = static_cast<float>(r.fDistX)
                  * static_cast<float>(RadarTypes::RES_F_DISTX);
    o.fDistY    = static_cast<float>(r.fDistY)
                  * static_cast<float>(RadarTypes::RES_F_DISTY);
    o.fVabsX    = static_cast<float>(r.fVabsX)
                  * static_cast<float>(RadarTypes::RES_F_VABSX);
    o.fVabsY    = static_cast<float>(r.fVabsY)
                  * static_cast<float>(RadarTypes::RES_F_VABSY);
    o.fAabsX    = static_cast<float>(r.fAabsX)
                  * static_cast<float>(RadarTypes::RES_F_AABSX);
    o.fAabsY    = static_cast<float>(r.fAabsY)
                  * static_cast<float>(RadarTypes::RES_F_AABSY);

    o.fDistXStd = static_cast<float>(r.fDistXStd)
                  * static_cast<float>(RadarTypes::RES_F_DISTXSTD);
    o.fDistYStd = static_cast<float>(r.fDistYStd)
                  * static_cast<float>(RadarTypes::RES_F_DISTYSTD);
    o.fVabsXStd = static_cast<float>(r.fVabsXStd)
                  * static_cast<float>(RadarTypes::RES_F_VABSXSTD);
    o.fVabsYStd = static_cast<float>(r.fVabsYStd)
                  * static_cast<float>(RadarTypes::RES_F_VABSYSTD);

    o.fAabsXStd = static_cast<float>(r.fAabsXStd)
                  * static_cast<float>(RadarTypes::RES_F_AABSXSTD);
    o.fAabsYStd = static_cast<float>(r.fAabsYStd)
                  * static_cast<float>(RadarTypes::RES_F_AABSYSTD);

    for (int i = 0; i < 3; ++i)
    {
        o.fLDeltaX[i] = static_cast<float>(r.aLDeltaX.fLDeltaX[i])
                        * static_cast<float>(RadarTypes::RES_F_LDELTAX);
        o.fLDeltaY[i] = static_cast<float>(r.aLDeltaY.fLDeltaY[i])
                        * static_cast<float>(RadarTypes::RES_F_LDELTAY);
    }

    o.eShapeQualifier  = r.eShapeQualifier;
    o.fObjOrientation  = static_cast<float>(r.fObjOrientation)
                         * static_cast<float>(RadarTypes::RES_F_OBJORIENTATION);
    o.fRCS             = static_cast<float>(r.fRCS)
                         * static_cast<float>(RadarTypes::RES_F_RCS);
    o.uProbOfExistence = r.uProbOfExistence;
    o.uLifeCycles      = r.uLifeCycles;
    o.eDynamicProperty = r.eDynamicProperty;
    o.eObjState        = r.eObjState;

    return o;
}

// ── RDI message decoder (shared for Near/Far) ─────────────────────────────

template<typename TRawMsg>
static RadarDecoded::tRDIMessage decodeRDIMessage(
    const TRawMsg& raw,
    uint16_t       nArraySize)
{
    RadarDecoded::tRDIMessage o{};

    o.nSensorID       = raw.nSensorID;
    o.nMessageCounter = raw.nMessageCounter;
    o.sPTPTimestamp   = convertPTP(raw.sPTPTimestamp);
    o.nTimeStamp      = raw.nTimeStamp;
    o.nCycleCounter   = raw.nCycleCounter;
    o.eSignalStatus   = raw.eSignalStatus;
    o.fVAmbig         = static_cast<float>(raw.fVAmbig)
                        * static_cast<float>(RadarTypes::RES_F_VAMBIG);
    o.fMaxRange       = static_cast<float>(raw.fMaxRange)
                        * static_cast<float>(RadarTypes::RES_F_MAXRANGE);
    o.nNbOfDetections = raw.nNbOfDetections;
    o.nArraySize      = nArraySize;

    const uint16_t nCount = std::min(
        o.nNbOfDetections,
        static_cast<uint16_t>(o.aDetections.size()));

    for (uint16_t i = 0; i < nCount; ++i)
        o.aDetections[i] = convertRDI(raw.aRDI.sRDI[i]);

    return o;
}

// ── Object message decoder (shared for 0/1) ───────────────────────────────

template<typename TRawMsg>
static RadarDecoded::tObjectMessage decodeObjectMessage(
    const TRawMsg& raw,
    uint16_t       nArraySize)
{
    RadarDecoded::tObjectMessage o{};

    o.nSensorID       = raw.nSensorID;
    o.nMessageCounter = raw.nMessageCounter;
    o.sPTPTimestamp   = convertPTP(raw.sPTPTimestamp);
    o.nTimeStamp      = raw.nTimeStamp;
    o.nCycleCounter   = raw.nCycleCounter;
    o.eSignalStatus   = raw.eSignalStatus;
    o.fEgoVx          = static_cast<float>(raw.fEgoVx)
                        * static_cast<float>(RadarTypes::RES_F_EGOVX);
    o.fEgoYawRate     = static_cast<float>(raw.fEgoYawRate)
                        * static_cast<float>(RadarTypes::RES_F_EGOYAWRATE);
    o.nNbOfObjects    = raw.nNbOfObjects;
    o.nArraySize      = nArraySize;

    const uint16_t nCount = std::min(
        o.nNbOfObjects,
        static_cast<uint16_t>(o.aObjects.size()));

    for (uint16_t i = 0; i < nCount; ++i)
        o.aObjects[i] = convertObject(raw.aObj.sObj[i]);

    return o;
}

// ── Radar UDP decode ───────────────────────────────────────────────────────────

EValidationResult decodeRadar(
    const uint8_t*                pData,
    size_t                        nLen,
    RadarDecoded::DecodedMessage& oOutput,
    uint32_t&                     nMessageIDOut)
{
    EValidationResult eResult =
        PacketValidator::validateSOMEIPPacket(pData, nLen, nMessageIDOut);

    if (eResult != EValidationResult::OK)
    {
        return eResult;
    }

    const uint8_t* pPayload = pData + sizeof(RadarTypes::tSOMEIPHeader);

    switch (nMessageIDOut)
    {
        case RadarTypes::MESSAGEID_RDINEAR_0:
        {
            RadarTypes::tRDI_Near_Message_0 raw{};
            std::memcpy(&raw, pPayload, sizeof(raw));
            oOutput = decodeRDIMessage(raw,
                static_cast<uint16_t>(RadarTypes::locationsNear0_Msgsize));
            break;
        }
        case RadarTypes::MESSAGEID_RDINEAR_1:
        {
            RadarTypes::tRDI_Near_Message_1 raw{};
            std::memcpy(&raw, pPayload, sizeof(raw));
            oOutput = decodeRDIMessage(raw,
                static_cast<uint16_t>(RadarTypes::locationsNear1_Msgsize));
            break;
        }
        case RadarTypes::MESSAGEID_RDINEAR_2:
        {
            RadarTypes::tRDI_Near_Message_2 raw{};
            std::memcpy(&raw, pPayload, sizeof(raw));
            oOutput = decodeRDIMessage(raw,
                static_cast<uint16_t>(RadarTypes::locationsNear2_Msgsize));
            break;
        }
        case RadarTypes::MESSAGEID_RDIFAR_0:
        {
            RadarTypes::tRDI_Far_Message_0 raw{};
            std::memcpy(&raw, pPayload, sizeof(raw));
            oOutput = decodeRDIMessage(raw,
                static_cast<uint16_t>(RadarTypes::locationsFar0_Msgsize));
            break;
        }
        case RadarTypes::MESSAGEID_RDIFAR_1:
        {
            RadarTypes::tRDI_Far_Message_1 raw{};
            std::memcpy(&raw, pPayload, sizeof(raw));
            oOutput = decodeRDIMessage(raw,
                static_cast<uint16_t>(RadarTypes::locationsFar1_Msgsize));
            break;
        }
        case RadarTypes::MESSAGEID_OBJECTS_0:
        {
            RadarTypes::tObject_Message_0 raw{};
            std::memcpy(&raw, pPayload, sizeof(raw));
            oOutput = decodeObjectMessage(raw,
                static_cast<uint16_t>(RadarTypes::objects0_Msgsize));
            break;
        }
        case RadarTypes::MESSAGEID_OBJECTS_1:
        {
            RadarTypes::tObject_Message_1 raw{};
            std::memcpy(&raw, pPayload, sizeof(raw));
            oOutput = decodeObjectMessage(raw,
                static_cast<uint16_t>(RadarTypes::objects1_Msgsize));
            break;
        }
        case RadarTypes::MESSAGEID_SENSORSTATUS:
        {
            RadarTypes::tSensorStatus_Message raw{};
            std::memcpy(&raw, pPayload, sizeof(raw));

            RadarDecoded::tSensorStatusDecoded o{};
            o.nSensorID          = raw.nSensorID;
            o.fCurrentLongPos    = static_cast<float>(raw.fCurrentLongPos)
                                   * static_cast<float>(RadarTypes::RES_F_CURRENTLONGPOS);
            o.fCurrentLatPos     = static_cast<float>(raw.fCurrentLatPos)
                                   * static_cast<float>(RadarTypes::RES_F_CURRENTLATPOS);
            o.fCurrentVertPos    = static_cast<float>(raw.fCurrentVertPos)
                                   * static_cast<float>(RadarTypes::RES_F_CURRENTVERTPOS);
            o.fCurrentLongPosCoG = static_cast<float>(raw.fCurrentLongPosCoG)
                                   * static_cast<float>(RadarTypes::RES_F_CURRENTLONGPOSCOG);
            o.fCurrentYawAngle   = static_cast<float>(raw.fCurrentYawAngle)
                                   * static_cast<float>(RadarTypes::RES_F_CURRENTYAWANGLE);
            o.fCurrentDamping    = static_cast<float>(raw.fCurrentDamping)
                                   * static_cast<float>(RadarTypes::RES_F_CURRENTDAMPING);
            o.nDefective         = raw.nDefective;
            o.nExtDisturbed      = raw.nExtDistrubed;
            o.nComError          = raw.nComError;
            o.fAlnMisalignmentAzNear = static_cast<float>(raw.fAlnMisalignmentAzNear)
                                       * static_cast<float>(RadarTypes::RES_F_ALNMISALIGNMENTAZNEAR);
            o.fAlnMisalignmentAzFar  = static_cast<float>(raw.fAlnMisalignmentAzFar)
                                       * static_cast<float>(RadarTypes::RES_F_ALNMISALIGNMENTAZFAR);
            o.fAlnMisalignmentElev   = static_cast<float>(raw.fAlnMisalignmentElev)
                                       * static_cast<float>(RadarTypes::RES_F_ALNMISALIGNMENTELEV);
            o.nAlnStatus         = raw.nAlnStatus;
            oOutput = o;
            break;
        }
        case RadarTypes::MESSAGEID_VEHDYN:
        {
            RadarTypes::tVehicleDynamics_Message raw{};
            std::memcpy(&raw, pPayload, sizeof(raw));

            RadarDecoded::tVehicleDynamicsDecoded o{};
            o.eLongDir   = raw.eLongDir;
            o.fLongVel   = static_cast<float>(raw.fLongVel)
                           * static_cast<float>(RadarTypes::RES_F_LONGVEL);
            o.fYawRate   = static_cast<float>(raw.fYawRate)
                           * static_cast<float>(RadarTypes::RES_F_YAWRATE);
            o.fLongAccel = static_cast<float>(raw.fLongAccel)
                           * static_cast<float>(RadarTypes::RES_F_LONGACCEL);
            o.fLatAccel  = static_cast<float>(raw.fLatAccel)
                           * static_cast<float>(RadarTypes::RES_F_LATACCEL);
            oOutput = o;
            break;
        }
        default:
        {
            oOutput = std::monostate{};
            return EValidationResult::ERR_UNKNOWN_MESSAGE_ID;
        }
    }
    return EValidationResult::OK;
}


// ── Egomotion decode ───────────────────────────────────────────────────────────

bool decodeEgomotion(
    const uint8_t*                         pData,
    size_t                                 nLen,
    RadarDecoded::tVehicleDynamicsDecoded& oOutput)
{
    // Buffer too short
    if (nLen < sizeof(EgoMasterIntf_V3::tEgoMaster3DData))
        return false;

    EgoMasterIntf_V3::tEgoMaster3DData oEgo{};
    std::memcpy(&oEgo, pData, sizeof(EgoMasterIntf_V3::tEgoMaster3DData));

    // Check confidence — reject if data is unreliable
    if (oEgo.sVelocity.nConf     > EgoMasterIntf_V3::EM_CONF_BESTGUESS ||
        oEgo.sAngularRate.nConf  > EgoMasterIntf_V3::EM_CONF_BESTGUESS ||
        oEgo.sAcceleration.nConf > EgoMasterIntf_V3::EM_CONF_BESTGUESS)
        return false;

    // Map EgoMaster → tVehicleDynamicsDecoded
    const float fLongVel = oEgo.sVelocity.fValX;

    oOutput.eLongDir   = (fLongVel >= 0.0f)
        ? RadarTypes::tEgoLongDir::EGOLONGDIR_FORWARD
        : RadarTypes::tEgoLongDir::EGOLONGDIR_BACKWARD;

    oOutput.fLongVel   = std::abs(fLongVel);
    oOutput.fYawRate   = oEgo.sAngularRate.fValZ;
    oOutput.fLongAccel = oEgo.sAcceleration.fValX;
    oOutput.fLatAccel  = oEgo.sAcceleration.fValY;

    return true;
}

bool encodeVehicleDynamics(
    const RadarDecoded::tVehicleDynamicsDecoded& oEgo,
    RadarTypes::tVehicleDynamics_Message&         oMsg)
{
    // Payload header
    oMsg.sHeader.nCRC = 0x0000;
    oMsg.sHeader.nLen =
        static_cast<uint16_t>(
            sizeof(RadarTypes::tVehicleDynamics_Message)
            - sizeof(uint16_t));
    oMsg.sHeader.nSQC = 0;

    // Direction — uint8_t enum, no swap needed
    oMsg.eLongDir = oEgo.eLongDir;

    // fLongVel is uint16_t in the raw struct — no sign
    oMsg.fLongVel = 
        static_cast<uint16_t>(
            oEgo.fLongVel
            / static_cast<float>(RadarTypes::RES_F_LONGVEL));

    // fYawRate, fLongAccel, fLatAccel are int16_t — signed
    oMsg.fYawRate = 
        static_cast<int16_t>(
            oEgo.fYawRate
            / static_cast<float>(RadarTypes::RES_F_YAWRATE));

    oMsg.fLongAccel =
        static_cast<int16_t>(
            oEgo.fLongAccel
            / static_cast<float>(RadarTypes::RES_F_LONGACCEL));

    oMsg.fLatAccel = 
        static_cast<int16_t>(
            oEgo.fLatAccel
            / static_cast<float>(RadarTypes::RES_F_LATACCEL));

    return true;
}

} // namespace RadarDecoder
