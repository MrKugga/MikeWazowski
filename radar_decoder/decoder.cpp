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
    o.nSecHi   = __builtin_bswap16(r.nPTPSec_Hi);
    o.nSecLo   = __builtin_bswap32(r.nPTPSec_Lo);
    o.nNanoSec = __builtin_bswap32(r.nPTPNanoSec);
    o.eSync    = r.ePTPSync;
    return o;
}

static RadarDecoded::tRDIDetection convertRDI(
    const RadarTypes::tRDI& r)
{
    RadarDecoded::tRDIDetection o{};

    o.fRange   = static_cast<float>(__builtin_bswap16(r.fRange))
                 * static_cast<float>(RadarTypes::RES_F_RANGE);
    o.fVrelRad = static_cast<float>(__builtin_bswap16(r.fVrelRad))
                 * static_cast<float>(RadarTypes::RES_F_VRELRAD);
    o.fAzAng0  = static_cast<float>(__builtin_bswap16(r.aAzAng.fAzAng[0]))
                 * static_cast<float>(RadarTypes::RES_F_AZIMUTH);
    o.fAzAng1  = static_cast<float>(__builtin_bswap16(r.aAzAng.fAzAng[1]))
                 * static_cast<float>(RadarTypes::RES_F_AZIMUTH);
    o.fElAng   = static_cast<float>(__builtin_bswap16(r.fElAng))
                 * static_cast<float>(RadarTypes::RES_F_ELEVATION);
    o.fRCS0    = static_cast<float>(__builtin_bswap16(r.aRCS.fRCS[0]))
                 * static_cast<float>(RadarTypes::RES_F_RCS);
    o.fRCS1    = static_cast<float>(__builtin_bswap16(r.aRCS.fRCS[1]))
                 * static_cast<float>(RadarTypes::RES_F_RCS);

    o.fRangeVar   = static_cast<float>(__builtin_bswap16(r.fRangeVar))
                    * static_cast<float>(RadarTypes::RES_F_RANGEVAR);
    o.fVrelRadVar = static_cast<float>(__builtin_bswap16(r.fVrelRadVar))
                    * static_cast<float>(RadarTypes::RES_F_VRELRADVAR);
    o.fAzAngVar   = static_cast<float>(__builtin_bswap16(r.fAzAngVar))
                    * static_cast<float>(RadarTypes::RES_F_AZIMUHTVAR);
    o.fElAngVar   = static_cast<float>(__builtin_bswap16(r.fElAngVar))
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

    o.fDistX    = static_cast<float>(__builtin_bswap16(r.fDistX))
                  * static_cast<float>(RadarTypes::RES_F_DISTX);
    o.fDistY    = static_cast<float>(__builtin_bswap16(r.fDistY))
                  * static_cast<float>(RadarTypes::RES_F_DISTY);
    o.fVabsX    = static_cast<float>(__builtin_bswap16(r.fVabsX))
                  * static_cast<float>(RadarTypes::RES_F_VABSX);
    o.fVabsY    = static_cast<float>(__builtin_bswap16(r.fVabsY))
                  * static_cast<float>(RadarTypes::RES_F_VABSY);
    o.fAabsX    = static_cast<float>(__builtin_bswap16(r.fAabsX))
                  * static_cast<float>(RadarTypes::RES_F_AABSX);
    o.fAabsY    = static_cast<float>(__builtin_bswap16(r.fAabsY))
                  * static_cast<float>(RadarTypes::RES_F_AABSY);

    o.fDistXStd = static_cast<float>(__builtin_bswap16(r.fDistXStd))
                  * static_cast<float>(RadarTypes::RES_F_DISTXSTD);
    o.fDistYStd = static_cast<float>(__builtin_bswap16(r.fDistYStd))
                  * static_cast<float>(RadarTypes::RES_F_DISTYSTD);
    o.fVabsXStd = static_cast<float>(__builtin_bswap16(r.fVabsXStd))
                  * static_cast<float>(RadarTypes::RES_F_VABSXSTD);
    o.fVabsYStd = static_cast<float>(__builtin_bswap16(r.fVabsYStd))
                  * static_cast<float>(RadarTypes::RES_F_VABSYSTD);

    o.fAabsXStd = static_cast<float>(r.fAabsXStd)
                  * static_cast<float>(RadarTypes::RES_F_AABSXSTD);
    o.fAabsYStd = static_cast<float>(r.fAabsYStd)
                  * static_cast<float>(RadarTypes::RES_F_AABSYSTD);

    for (int i = 0; i < 3; ++i)
    {
        o.fLDeltaX[i] = static_cast<float>(__builtin_bswap16(r.aLDeltaX.fLDeltaX[i]))
                        * static_cast<float>(RadarTypes::RES_F_LDELTAX);
        o.fLDeltaY[i] = static_cast<float>(__builtin_bswap16(r.aLDeltaY.fLDeltaY[i]))
                        * static_cast<float>(RadarTypes::RES_F_LDELTAY);
    }

    o.eShapeQualifier  = r.eShapeQualifier;
    o.fObjOrientation  = static_cast<float>(__builtin_bswap16(r.fObjOrientation))
                         * static_cast<float>(RadarTypes::RES_F_OBJORIENTATION);
    o.fRCS             = static_cast<float>(__builtin_bswap16(r.fRCS))
                         * static_cast<float>(RadarTypes::RES_F_RCS);
    o.uProbOfExistence = r.uProbOfExistence;
    o.uLifeCycles      = __builtin_bswap16(r.uLifeCycles);
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
    o.nTimeStamp      = __builtin_bswap32(raw.nTimeStamp);
    o.nCycleCounter   = __builtin_bswap32(raw.nCycleCounter);
    o.eSignalStatus   = raw.eSignalStatus;
    o.fVAmbig         = static_cast<float>(__builtin_bswap16(raw.fVAmbig))
                        * static_cast<float>(RadarTypes::RES_F_VAMBIG);
    o.fMaxRange       = static_cast<float>(__builtin_bswap16(raw.fMaxRange))
                        * static_cast<float>(RadarTypes::RES_F_MAXRANGE);
    o.nNbOfDetections = __builtin_bswap16(raw.nNbOfDetections);
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
    o.nTimeStamp      = __builtin_bswap32(raw.nTimeStamp);
    o.nCycleCounter   = __builtin_bswap32(raw.nCycleCounter);
    o.eSignalStatus   = raw.eSignalStatus;
    o.fEgoVx          = static_cast<float>(__builtin_bswap16(raw.fEgoVx))
                        * static_cast<float>(RadarTypes::RES_F_EGOVX);
    o.fEgoYawRate     = static_cast<float>(__builtin_bswap16(raw.fEgoYawRate))
                        * static_cast<float>(RadarTypes::RES_F_EGOYAWRATE);
    o.nNbOfObjects    = __builtin_bswap16(raw.nNbOfObjects);
    o.nArraySize      = nArraySize;

    const uint16_t nCount = std::min(
        o.nNbOfObjects,
        static_cast<uint16_t>(o.aObjects.size()));

    for (uint16_t i = 0; i < nCount; ++i)
        o.aObjects[i] = convertObject(raw.aObj.sObj[i]);

    return o;
}

// ── Main decode ───────────────────────────────────────────────────────────

EValidationResult decode(
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
            o.fCurrentLongPos    = static_cast<float>(__builtin_bswap16(raw.fCurrentLongPos))
                                   * static_cast<float>(RadarTypes::RES_F_CURRENTLONGPOS);
            o.fCurrentLatPos     = static_cast<float>(__builtin_bswap16(raw.fCurrentLatPos))
                                   * static_cast<float>(RadarTypes::RES_F_CURRENTLATPOS);
            o.fCurrentVertPos    = static_cast<float>(__builtin_bswap16(raw.fCurrentVertPos))
                                   * static_cast<float>(RadarTypes::RES_F_CURRENTVERTPOS);
            o.fCurrentLongPosCoG = static_cast<float>(__builtin_bswap16(raw.fCurrentLongPosCoG))
                                   * static_cast<float>(RadarTypes::RES_F_CURRENTLONGPOSCOG);
            o.fCurrentYawAngle   = static_cast<float>(__builtin_bswap16(raw.fCurrentYawAngle))
                                   * static_cast<float>(RadarTypes::RES_F_CURRENTYAWANGLE);
            o.fCurrentDamping    = static_cast<float>(__builtin_bswap16(raw.fCurrentDamping))
                                   * static_cast<float>(RadarTypes::RES_F_CURRENTDAMPING);
            o.nDefective         = raw.nDefective;
            o.nExtDisturbed      = raw.nExtDistrubed;
            o.nComError          = raw.nComError;
            o.fAlnMisalignmentAzNear = static_cast<float>(__builtin_bswap16(raw.fAlnMisalignmentAzNear))
                                       * static_cast<float>(RadarTypes::RES_F_ALNMISALIGNMENTAZNEAR);
            o.fAlnMisalignmentAzFar  = static_cast<float>(__builtin_bswap16(raw.fAlnMisalignmentAzFar))
                                       * static_cast<float>(RadarTypes::RES_F_ALNMISALIGNMENTAZFAR);
            o.fAlnMisalignmentElev   = static_cast<float>(__builtin_bswap16(raw.fAlnMisalignmentElev))
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
            o.fLongVel   = static_cast<float>(__builtin_bswap16(raw.fLongVel))
                           * static_cast<float>(RadarTypes::RES_F_LONGVEL);
            o.fYawRate   = static_cast<float>(__builtin_bswap16(raw.fYawRate))
                           * static_cast<float>(RadarTypes::RES_F_YAWRATE);
            o.fLongAccel = static_cast<float>(__builtin_bswap16(raw.fLongAccel))
                           * static_cast<float>(RadarTypes::RES_F_LONGACCEL);
            o.fLatAccel  = static_cast<float>(__builtin_bswap16(raw.fLatAccel))
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

} // namespace RadarDecoder