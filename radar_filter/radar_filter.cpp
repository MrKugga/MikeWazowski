#include "radar_filter.h"

ADTF_PLUGIN(NAME_CUSTOM_FILTER, cPacketParserFilter);

cPacketParserFilter::cPacketParserFilter()
{
    LOG_INFO("Initializing filter...");
    
    SetDescription("Radar Packed Decoder");

    m_pReader = CreateInputPin("raw_someip");
    m_pVehDynReader= CreateInputPin("egomotion_input");

    
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
    if(pReader == m_pReader) 
    {
        // Lock sample buffer
        adtf::ucom::object_ptr_shared_locked
        <const adtf::streaming::ISampleBuffer> pSampleBuffer;
        RETURN_IF_FAILED(pSample->Lock(pSampleBuffer));

        const uint8_t* pData = static_cast<const uint8_t*>(pSampleBuffer->GetPtr());
        const size_t   nLen  = pSampleBuffer->GetSize();

        // ── Debug block — remove once validated ──────────────────────────────
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
        // ── End debug block ───────────────────────────────────────────────────

        // ── Decode ────────────────────────────────────────────────────────────
        RadarDecoded::DecodedMessage oOutput;
        uint32_t nMessageID = 0;
        const EValidationResult eResult =
            RadarDecoder::decode(pData, nLen, oOutput, nMessageID);


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
        const adtf::base::tNanoSeconds tmSample = adtf::streaming::get_sample_time(pSample);

        const tResult oVisitResult = std::visit(RadarDecoded::overloaded{

            [&](const RadarDecoded::tRDIMessage& msg) -> tResult
            {
                // LOG_INFO("[pkt %u] %s → RDI sensor=%u det=%u cycle=%u",
                //     m_nPacketCount,
                //     PacketValidator::messageIDToString(nMessageID),
                //     msg.nSensorID,
                //     msg.nNbOfDetections,
                //     msg.nCycleCounter);

                // Feed into cycle accumulator
                const bool bCycleComplete =
                    m_oAccumulator.addMessage(msg, nMessageID);

                if (bCycleComplete)
                {
                    const size_t nBufSize = m_oAccumulator.getSendBufferSize();

                    // Guard — must never be zero
                    if (nBufSize == 0)
                    {
                        LOG_ERROR("Cycle complete but buffer size is 0 — skipping");
                        RETURN_NOERROR;  // ← bug 1 fix: always return
                    }

                    LOG_INFO("Cycle %u complete — near=%u far=%u  buf=%zu bytes",
                        m_oAccumulator.getCurrentCycle(),
                        m_oAccumulator.getNearCount(),
                        m_oAccumulator.getFarCount(),
                        nBufSize);

                    const uint8_t* pBuf = m_oAccumulator.getSendBuffer();

                    adtf::ucom::object_ptr<adtf::streaming::ISample> pOutSample;
                    RETURN_IF_FAILED(adtf::streaming::alloc_sample(pOutSample, tmSample));

                    {
                        adtf::ucom::object_ptr_locked<adtf::streaming::ISampleBuffer> pOutBuffer;
                        RETURN_IF_FAILED(pOutSample->WriteLock(pOutBuffer, nBufSize));
                        std::memcpy(pOutBuffer->GetPtr(), pBuf, nBufSize);
                    }

                    RETURN_IF_FAILED(m_pUDPWriter->Write(pOutSample));
                }

                RETURN_NOERROR;
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
    else if (pReader == m_pVehDynReader)
    {
        
        RETURN_NOERROR;
    }

}

// ── Write helpers ─────────────────────────────────────────────────────────

tResult cPacketParserFilter::writeRDI(
    const RadarDecoded::tRDIMessage& msg,
    adtf::base::tNanoSeconds         tmSample)
{

    adtf::streaming::output_sample_data<RadarDecoded::tRDIMessage>
        oOut(tmSample, msg);
    RETURN_IF_FAILED(m_pRDIWriter->Write(oOut.Release()));

    RETURN_NOERROR;
}

tResult cPacketParserFilter::writeObject(
    const RadarDecoded::tObjectMessage& msg,
    adtf::base::tNanoSeconds            tmSample)
{
    // LOG_INFO("OBJ: sensor=%u  objects=%u/%u  cycle=%u  ts=%u  "
    //          "egoVx=%.2f m/s  egoYaw=%.4f rad/s  status=%u",
    //     msg.nSensorID,
    //     msg.nNbOfObjects,
    //     msg.nArraySize,
    //     msg.nCycleCounter,
    //     msg.nTimeStamp,
    //     msg.fEgoVx,
    //     msg.fEgoYawRate,
    //     static_cast<uint8_t>(msg.eSignalStatus));

    adtf::streaming::output_sample_data<RadarDecoded::tObjectMessage>
        oOut(tmSample, msg);
    RETURN_IF_FAILED(m_pObjectWriter->Write(oOut.Release()));

    RETURN_NOERROR;
}

tResult cPacketParserFilter::writeStatus(
    const RadarDecoded::tSensorStatusDecoded& msg,
    adtf::base::tNanoSeconds                  tmSample)
{
    LOG_INFO("STATUS: sensor=%u  longPos=%.3f m  latPos=%.3f m  "
             "yaw=%.4f rad  aln=%u",
        msg.nSensorID,
        msg.fCurrentLongPos,
        msg.fCurrentLatPos,
        msg.fCurrentYawAngle,
        msg.nAlnStatus);

    adtf::streaming::output_sample_data<RadarDecoded::tSensorStatusDecoded>
        oOut(tmSample, msg);
    RETURN_IF_FAILED(m_pStatusWriter->Write(oOut.Release()));

    RETURN_NOERROR;
}

tResult cPacketParserFilter::writeVehDyn(
    const RadarDecoded::tVehicleDynamicsDecoded& msg,
    adtf::base::tNanoSeconds                     tmSample)
{
    // LOG_INFO("VEHDYN: vel=%.2f m/s  yawrate=%.4f rad/s  "
    //          "longAccel=%.3f m/s^2  latAccel=%.3f m/s^2  dir=%u",
    //     msg.fLongVel,
    //     msg.fYawRate,
    //     msg.fLongAccel,
    //     msg.fLatAccel,
    //     static_cast<uint8_t>(msg.eLongDir));

    adtf::streaming::output_sample_data<RadarDecoded::tVehicleDynamicsDecoded>
        oOut(tmSample, msg);
    RETURN_IF_FAILED(m_pVehDynWriter->Write(oOut.Release()));

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
