#include "radar_filter.h"

ADTF_PLUGIN(NAME_CUSTOM_FILTER, cPacketParserFilter);

// We dont know the Egomotion packet so we print it and analyze it! --> to be removed? Keep as debug.
static void dumpEgomotionPacket(const uint8_t* pData, size_t nLen)
{
    LOG_INFO("=== Egomotion packet: %zu bytes ===", nLen);
    LOG_INFO("  sizeof(tEgoMaster3DData) = %zu",
        sizeof(EgoMasterIntf_V3::tEgoMaster3DData));
    LOG_INFO("  size match: %s",
        nLen >= sizeof(EgoMasterIntf_V3::tEgoMaster3DData)
            ? "YES" : "NO — too short");

    // Dump first 32 bytes for visual inspection
    for (size_t i = 0; i < std::min(nLen, size_t(32)); ++i)
        LOG_INFO("  [%03zu] 0x%02X  %3u", i, pData[i], pData[i]);

    // If size matches — dump key fields
    if (nLen >= sizeof(EgoMasterIntf_V3::tEgoMaster3DData))
    {
        EgoMasterIntf_V3::tEgoMaster3DData oEgo{};
        std::memcpy(&oEgo, pData,
            sizeof(EgoMasterIntf_V3::tEgoMaster3DData));

        LOG_INFO("  nTime:              %lld us",
            static_cast<long long>(oEgo.nTime));
        LOG_INFO("  sVelocity:          X=%.4f  Y=%.4f  Z=%.4f  conf=%d",
            oEgo.sVelocity.fValX,
            oEgo.sVelocity.fValY,
            oEgo.sVelocity.fValZ,
            oEgo.sVelocity.nConf);
        LOG_INFO("  sAngularRate:       X=%.4f  Y=%.4f  Z=%.4f  conf=%d",
            oEgo.sAngularRate.fValX,
            oEgo.sAngularRate.fValY,
            oEgo.sAngularRate.fValZ,
            oEgo.sAngularRate.nConf);
        LOG_INFO("  sAcceleration:      X=%.4f  Y=%.4f  Z=%.4f  conf=%d",
            oEgo.sAcceleration.fValX,
            oEgo.sAcceleration.fValY,
            oEgo.sAcceleration.fValZ,
            oEgo.sAcceleration.nConf);
        LOG_INFO("  sPoseUSK0_2D.fValYaw: %.4f rad",
            oEgo.sPoseUSK0_2D.fValYaw);
    }
}

cPacketParserFilter::cPacketParserFilter()
{
    LOG_INFO("Initializing filter...");
    
    SetDescription("Radar Packed Decoder");

    m_pRadarReader = CreateInputPin("raw_someip");
    m_pEgoReader= CreateInputPin("egomotion_input");

    
    // Outputs — one for udp out + one per decoded message category
    m_pUDPWriter = CreateOutputPin("udp_out",
    adtf::streaming::stream_type
        <adtf::streaming::stream_meta_type_anonymous>());
    m_pRDIWriter = CreateOutputPin("rdi_detections",
        adtf::streaming::stream_type
            <adtf::streaming::stream_meta_type_anonymous>());
    m_pObjectWriter = CreateOutputPin("objects",
        adtf::streaming::stream_type
            <adtf::streaming::stream_meta_type_anonymous>());
    m_pStatusWriter = CreateOutputPin("sensor_status",
        adtf::streaming::stream_type
            <adtf::streaming::stream_meta_type_anonymous>());
    m_pVehDynWriter = CreateOutputPin("vehicle_dynamics",
        adtf::streaming::stream_type
            <adtf::streaming::stream_meta_type_anonymous>());

    SetDescription("Parses raw SOME/IP packets and decodes radar messages.");
}


tResult cPacketParserFilter::ProcessInput(
    adtf::streaming::ISampleReader* pReader,
    const adtf::ucom::iobject_ptr
        <const adtf::streaming::ISample>& pSample)
{   
    
    //Create sample buffer
    adtf::ucom::object_ptr_shared_locked<const adtf::streaming::ISampleBuffer> pSampleBuffer;
    RETURN_IF_FAILED(pSample->Lock(pSampleBuffer));

    const uint8_t* pData = static_cast<const uint8_t*>(pSampleBuffer->GetPtr());
    const size_t   nLen  = pSampleBuffer->GetSize();
    const adtf::base::tNanoSeconds tmSample = adtf::streaming::get_sample_time(pSample);

    // ── Egomotion trigger the call --> process Egomotion data ───────────────────
    if (pReader == m_pEgoReader)
    {
        // Egomotion stream — decode and forward to radar
        return processEgomotion(pData, nLen, tmSample);
    }
    
    // ── Radar reader trigger the call --> process Radar data ───────────────────
    else if(pReader == m_pRadarReader) 
    {
        // ── Debug block — remove once validated ----removed for now :-) ─────────────────────────────
        /*
        if (m_nPacketCount < 20)
        {
            CRCUtils::checkCRCAcrossPackets(
                pData, nLen,
                sizeof(RadarTypes::tSOMEIPHeader),
                m_nPacketCount);
        }

        // Run full CRC debug on first RDINEAR_0 packet only
        if (!m_bDebugDone)
        {
            uint16_t nServiceID = 0, nMethodID = 0;
            std::memcpy(&nServiceID, pData + 0, sizeof(uint16_t));
            std::memcpy(&nMethodID,  pData + 2, sizeof(uint16_t));
            nServiceID = __builtin_bswap16(nServiceID);
            nMethodID  = __builtin_bswap16(nMethodID);

            uint32_t nMessageIDOut = (uint32_t)nServiceID << 16 | nMethodID;

            if (nMessageIDOut == RadarTypes::MESSAGEID_RDINEAR_0)
            {
                m_bDebugDone = true;
                // LOG_INFO("First RDINEAR_0: size=%zu expected=%zu",
                //     nLen,
                //     sizeof(RadarTypes::tSOMEIPHeader) +
                //     sizeof(RadarTypes::tRDI_Near_Message_0));

                CRCUtils::compareKnownCRC(
                    pData, nLen,
                    sizeof(RadarTypes::tSOMEIPHeader),
                    sizeof(RadarTypes::tSOMEIPPayloadHeader));

                CRCUtils::debugAllCRCCombinations(
                    pData, nLen,
                    sizeof(RadarTypes::tSOMEIPHeader),
                    sizeof(RadarTypes::tSOMEIPPayloadHeader));
            }
        }
        m_nPacketCount++;
        */
        // ── End debug block ───────────────────────────────────────────────────

        // ── Decode ────────────────────────────────────────────────────────────
        RadarDecoded::DecodedMessage oOutput;
        uint32_t nMessageID = 0;
        const EValidationResult eResult = RadarDecoder::decodeRadar(pData, nLen, oOutput, nMessageID);
        if (eResult != EValidationResult::OK)
        {
            // ERR_UNKNOWN_MESSAGE_ID is expected for SENSORCONFIG (Tx only)
            // — only warn on unexpected errors
            if (eResult != EValidationResult::ERR_UNKNOWN_MESSAGE_ID)
            {
                LOG_WARNING("[pkt %u] decode failed: %s  messageID=0x%08X (%s)",
                    m_nPacketCount,
                    toString(eResult),
                    nMessageID,
                    PacketValidator::messageIDToString(nMessageID));
            }
            RETURN_NOERROR;
        }

        // ── Dispatch to output pins ───────────────────────────────────────────

        const tResult oVisitResult = std::visit(RadarDecoded::overloaded{

            [&](const RadarDecoded::tRDIMessage& msg) -> tResult
            {
                return writeRDI(msg, nMessageID, tmSample);
            },

            [&](const RadarDecoded::tObjectMessage& msg) -> tResult {
                RETURN_NOERROR;
                //return writeObject(msg, tmSample);
            },

            [&](const RadarDecoded::tSensorStatusDecoded& msg) -> tResult {
                RETURN_NOERROR;
                //return writeStatus(msg, tmSample);
            },

            [&](const RadarDecoded::tVehicleDynamicsDecoded& msg) -> tResult {
                RETURN_NOERROR;
                //return writeVehDyn(msg, tmSample);
            },

            [](std::monostate) -> tResult {
                return ERR_NOERROR;
            }

        }, oOutput);

        RETURN_IF_FAILED(oVisitResult);

        RETURN_NOERROR;
    }
}

tResult cPacketParserFilter::processEgomotion(
    const uint8_t*           pData,
    size_t                   nLen,
    adtf::base::tNanoSeconds tmSample)
{
    if (!m_bEgoDebugDone)
    {
        m_bEgoDebugDone = true;
        dumpEgomotionPacket(pData, nLen);
    }

    RadarDecoded::tVehicleDynamicsDecoded oVehDyn{};

    if (!RadarDecoder::decodeEgomotion(pData, nLen, oVehDyn))
    {
        // Log reason here — where LOG_WARNING is available
        if (nLen < sizeof(EgoMasterIntf_V3::tEgoMaster3DData))
        {
            LOG_WARNING("processEgomotion: buffer too short "
                        "(%zu bytes, need %zu)",
                nLen,
                sizeof(EgoMasterIntf_V3::tEgoMaster3DData));
        }
        else
        {
            LOG_WARNING("processEgomotion: confidence check failed "
                        "— data not valid or best guess");
        }
        RETURN_NOERROR;
    }

    LOG_INFO("Egomotion: dir=%u  vel=%.3f m/s  yaw=%.4f rad/s  "
             "longAcc=%.3f m/s²  latAcc=%.3f m/s²",
        static_cast<uint8_t>(oVehDyn.eLongDir),
        oVehDyn.fLongVel,
        oVehDyn.fYawRate,
        oVehDyn.fLongAccel,
        oVehDyn.fLatAccel);

    // Re-encode and forward to radar
    RadarTypes::tVehicleDynamics_Message oRawMsg{};
    RadarDecoder::encodeVehicleDynamics(oVehDyn, oRawMsg);

    adtf::ucom::object_ptr<adtf::streaming::ISample> pOutSample;
    RETURN_IF_FAILED(adtf::streaming::alloc_sample(pOutSample, tmSample));

    {
        adtf::ucom::object_ptr_locked<adtf::streaming::ISampleBuffer> pOutBuffer;
        RETURN_IF_FAILED(pOutSample->WriteLock(
            pOutBuffer,
            sizeof(RadarTypes::tVehicleDynamics_Message)));
        std::memcpy(pOutBuffer->GetPtr(), &oRawMsg,
            sizeof(RadarTypes::tVehicleDynamics_Message));
    }

    RETURN_IF_FAILED(m_pVehDynWriter->Write(pOutSample));

    RETURN_NOERROR;
}

// ── Write helpers ─────────────────────────────────────────────────────────
tResult cPacketParserFilter::writeRDI(
    const RadarDecoded::tRDIMessage& msg,
    uint32_t                         nMessageID,
    adtf::base::tNanoSeconds         tmSample)
{
    // ── Forward raw decoded struct to ADTF stream pin ─────────────────────
    // Downstream ADTF filters can connect to this pin
    adtf::streaming::output_sample_data<RadarDecoded::tRDIMessage>
        oOut(tmSample, msg);
    RETURN_IF_FAILED(m_pRDIWriter->Write(oOut.Release()));

    // ── Feed into cycle accumulator ───────────────────────────────────────
    // Accumulates RDINEAR_0/1/2 and RDIFAR_0/1 separately
    // Returns true when a complete near OR far group is ready
    const bool bCycleComplete =
        m_oAccumulator.addMessage(msg, nMessageID);

    if (!bCycleComplete)
        RETURN_NOERROR;

    // ── Cycle complete — send via UDP ─────────────────────────────────────
    const size_t nBufSize = m_oAccumulator.getSendBufferSize();

    if (nBufSize == 0)
    {
        LOG_ERROR("writeRDI: cycle complete but buffer size is 0 — skipping");
        RETURN_NOERROR;
    }

    LOG_INFO("writeRDI: cycle %u complete — near=%u far=%u  buf=%zu bytes",
        m_oAccumulator.getCurrentCycle(),
        m_oAccumulator.getNearCount(),
        m_oAccumulator.getFarCount(),
        nBufSize);

    const uint8_t* pBuf = m_oAccumulator.getSendBuffer();

    // Allocate sample
    adtf::ucom::object_ptr<adtf::streaming::ISample> pOutSample;
    RETURN_IF_FAILED(adtf::streaming::alloc_sample(pOutSample, tmSample));

    // Lock, copy, unlock -> la graffe vanno tenute, perSmette di fare fare unlock del buffer dopo '}'
    {
        adtf::ucom::object_ptr_locked<adtf::streaming::ISampleBuffer> pOutBuffer;
        RETURN_IF_FAILED(pOutSample->WriteLock(pOutBuffer, nBufSize));
        std::memcpy(pOutBuffer->GetPtr(), pBuf, nBufSize);
    }

    // Write to UDP output pin (UDPSinkToNonADTFApplication=
    RETURN_IF_FAILED(m_pUDPWriter->Write(pOutSample));

    RETURN_NOERROR;
}

tResult cPacketParserFilter::writeObject(
    const RadarDecoded::tObjectMessage& msg,
    adtf::base::tNanoSeconds            tmSample)
{
    adtf::streaming::output_sample_data<RadarDecoded::tObjectMessage>
        oOut(tmSample, msg);
    RETURN_IF_FAILED(m_pObjectWriter->Write(oOut.Release()));

    RETURN_NOERROR;
}

tResult cPacketParserFilter::writeStatus(
    const RadarDecoded::tSensorStatusDecoded& msg,
    adtf::base::tNanoSeconds                  tmSample)
{
    adtf::streaming::output_sample_data<RadarDecoded::tSensorStatusDecoded>
        oOut(tmSample, msg);
    RETURN_IF_FAILED(m_pStatusWriter->Write(oOut.Release()));

    RETURN_NOERROR;
}



tResult cPacketParserFilter::Init(tInitStage eStage) {

    LOG_INFO("Initialising UDP Decoder");
    RETURN_NOERROR;
}


tResult cPacketParserFilter::Shutdown(tInitStage eStage) {

    LOG_INFO("Shutting down UDP Decoder");
    RETURN_NOERROR;
}
