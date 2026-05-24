#include "packet_parser_filter.h"

ADTF_PLUGIN(NAME_CUSTOM_FILTER, cPacketParserFilter);

cPacketParserFilter::cPacketParserFilter()
{
    LOG_INFO("Initializing filter...");
    
    SetDescription("Radar Packed Decoder");

    adtf::ucom::object_ptr<adtf::streaming::IStreamType const> pStreamType = 
        adtf::ucom::make_object_ptr<stream_meta_type<tEthernetPacket>>();

    adtf::ucom::object_ptr<adtf::streaming::IStreamType const> pPlainStreamType =
        adtf::ucom::make_object_ptr<adtf::streaming::stream_type_plain<tUInt64>>();

    /*
    auto oDescAdtfHeader = adtf::mediadescription::structure<tAdtfNetworkHeader>("tAdtfNetworkHeader");
    oDescAdtfHeader.createElement("nTimestampUs", &tAdtfNetworkHeader::nTimestampUs);
    oDescAdtfHeader.createElement("nFrameLength", &tAdtfNetworkHeader::nFrameLength);
    oDescAdtfHeader.createElement("nOrigLength", &tAdtfNetworkHeader::nOrigLength);
    oDescAdtfHeader.createElement("nInterfaceId", &tAdtfNetworkHeader::nInterfaceId);
    oDescAdtfHeader.createElement("nPadding", &tAdtfNetworkHeader::nPadding);
    
    auto oDescMACArray = adtf::mediadescription::structure<RadarTypes::tMACArray>("tMACArray");
    oDescMACArray.createElement("nMac", &RadarTypes::tMACArray::nMAC);

    auto oDescEthernetHeader = adtf::mediadescription::structure<RadarTypes::tEthernetHeader>("tEthernetHeader");
    oDescEthernetHeader.createElement("nTest", &RadarTypes::tEthernetHeader::nTest);
    oDescEthernetHeader.createElement("aMacDestination", &RadarTypes::tEthernetHeader::aMacDestination, oDescMACArray);
    oDescEthernetHeader.createElement("aMacSource", &RadarTypes::tEthernetHeader::aMacSource, oDescMACArray);
    oDescEthernetHeader.createElement("nEtherType", &RadarTypes::tEthernetHeader::nEtherType);

    auto oDescIPHeader = adtf::mediadescription::structure<RadarTypes::tIPHeader>("tIPHeader");
    oDescIPHeader.createElement("nIPVers_Length", &RadarTypes::tIPHeader::nIPVers_Length);
    oDescIPHeader.createElement("nTypeOfService", &RadarTypes::tIPHeader::nTypeOfService);
    oDescIPHeader.createElement("nTotalLength", &RadarTypes::tIPHeader::nTotalLength);
    oDescIPHeader.createElement("nIdentification", &RadarTypes::tIPHeader::nIdentification);
    oDescIPHeader.createElement("nFragmentation", &RadarTypes::tIPHeader::nFragmentation);
    oDescIPHeader.createElement("nTTL", &RadarTypes::tIPHeader::nTTL);
    oDescIPHeader.createElement("nProtocol", &RadarTypes::tIPHeader::nProtocol);
    oDescIPHeader.createElement("nCRC", &RadarTypes::tIPHeader::nCRC);
    oDescIPHeader.createElement("nIPAddressSrc", &RadarTypes::tIPHeader::nIPAddressSrc);
    oDescIPHeader.createElement("nIPAddressDest", &RadarTypes::tIPHeader::nIPAddressDest);

    auto oDescUDPHeader = adtf::mediadescription::structure<RadarTypes::tUDPHeader>("tUDPHeader");
    oDescUDPHeader.createElement("nSrcPort", &RadarTypes::tUDPHeader::nSrcPort);
    oDescUDPHeader.createElement("nDestPort", &RadarTypes::tUDPHeader::nDestPort);
    oDescUDPHeader.createElement("nLength", &RadarTypes::tUDPHeader::nLength);
    oDescUDPHeader.createElement("nCRC", &RadarTypes::tUDPHeader::nCRC);
    */

    auto oDescSOMEIPHeader = adtf::mediadescription::structure<RadarTypes::tSOMEIPHeader>("tSOMEIPHeader");
    oDescSOMEIPHeader.createElement("nServiceID", &RadarTypes::tSOMEIPHeader::nServiceID);
    oDescSOMEIPHeader.createElement("nMethodID", &RadarTypes::tSOMEIPHeader::nMethodID);
    oDescSOMEIPHeader.createElement("nLength", &RadarTypes::tSOMEIPHeader::nLength);
    oDescSOMEIPHeader.createElement("nClientID", &RadarTypes::tSOMEIPHeader::nClientID);
    oDescSOMEIPHeader.createElement("nSessionID", &RadarTypes::tSOMEIPHeader::nSessionID);
    oDescSOMEIPHeader.createElement("nProtocolVersion", &RadarTypes::tSOMEIPHeader::nProtocolVersion);
    oDescSOMEIPHeader.createElement("nInterfaceVersion", &RadarTypes::tSOMEIPHeader::nInterfaceVersion);
    oDescSOMEIPHeader.createElement("nMsgType", &RadarTypes::tSOMEIPHeader::nMsgType);
    oDescSOMEIPHeader.createElement("nReturnCode", &RadarTypes::tSOMEIPHeader::nReturnCode);



    auto oDescEthStream = adtf::mediadescription::structure<tEthernetPacket>("tEthernetPacket");
    // oDescEthStream.createElement("sEthernetHeader", &tEthernetPacket::sEthernetHeader, oDescEthernetHeader);
    // oDescEthStream.createElement("sIPHeader", &tEthernetPacket::sIPHeader, oDescIPHeader);
    // oDescEthStream.createElement("sUDPHeader", &tEthernetPacket::sUDPHeader, oDescUDPHeader);
    oDescEthStream.createElement("sSOMEIPHeader", &tEthernetPacket::sSOMEIPHeader, oDescSOMEIPHeader);



    m_pReader = CreateInputPin("Raw Ethernet Stream");
    //m_pReader = CreateInputPin("Raw Ethernet Stream", oDescEthStream);
    //m_pWriter = CreateOutputPin<adtf::filter::pin_writer<uint32_t>>("SOME/IP Output", stream_type_plain<uint32_t>());
    m_pWriter = CreateOutputPin("Decoded Output", oDescEthStream);
    //m_pWriter = CreateOutputPin("Decoded Output", oDescAdtfHeader);
    //m_pWriter = CreateOutputPin("Decoded Output");
    //m_pWriter = CreateOutputPin("Decoder Output", pPlainStreamType);

    RegisterPropertyVariable("expected_service_id", m_nExpectedServiceId);
    RegisterPropertyVariable("expected_method_id",  m_nExpectedMethodId);

}


tResult cPacketParserFilter::ProcessInput(adtf::streaming::ISampleReader* pReader ,
    const adtf::ucom::iobject_ptr<const adtf::streaming::ISample>& pSample)
{   

    // if(m_pReader == pReader) {
        
    //     adtf::ucom::object_ptr<const adtf::streaming::ISample> pReadSample;
    //     while(IS_OK(m_pReader->GetNextSample(pReadSample))) {
    //         LOG_INFO("Inside here");
    //         RETURN_IF_FAILED(ProcessSample(pReadSample));
    //     }
        
    //     // if(pSample.Get()) {
    //     //     m_pWriter->Write(pSample);
    //     // }
        
    // }

    if(pSample.Get()) {

        //LOG_INFO("Processing samples");

        adtf::ucom::object_ptr_shared_locked<const adtf::streaming::ISampleBuffer> pSampleBuffer;
        RETURN_IF_FAILED(pSample->Lock(pSampleBuffer));

        const uint32_t nTotalSize = static_cast<const uint32_t>(pSampleBuffer->GetSize());
        //const tEthernetPacket* pCurrentPacket = reinterpret_cast<const tEthernetPacket*>(pSampleBuffer->GetPtr());
        
        const uint8_t* pCurrentPacket = static_cast<const uint8_t*>(pSampleBuffer->GetPtr());
        
        tDecodedMessage oDecodedMessage;
        EValidationResult eResult;
        uint32_t nMessageID = 0;

        static uint32_t nPacketCount = 0;
        static bool     bDebugDone   = false;

        // Parse message ID from every packet
        uint16_t nServiceID = 0, nMethodID = 0;
        std::memcpy(&nServiceID, pCurrentPacket + 0, 2); nServiceID = __builtin_bswap16(nServiceID);
        std::memcpy(&nMethodID,  pCurrentPacket + 2, 2); nMethodID  = __builtin_bswap16(nMethodID);
        const uint32_t nMsgID = (uint32_t)nServiceID << 16 | nMethodID;


        /* ------------------- CRC DEBUG -------------------------- */

       // Log first 20 packets of any type
        if (nPacketCount < 20){
            CRCUtils::checkCRCAcrossPackets(
                pCurrentPacket, nTotalSize,
                sizeof(RadarTypes::tSOMEIPHeader),
                nPacketCount);
        }
        nPacketCount++;

        // Debug only on RDINEAR_0
        if (nMsgID == RadarTypes::MESSAGEID_RDINEAR_0 && !bDebugDone)
        {
            bDebugDone = true;
            LOG_INFO("First RDINEAR_0 packet — size=%zu  expected=%zu",
                nTotalSize,
                sizeof(RadarTypes::tSOMEIPHeader) +
                sizeof(RadarTypes::tRDI_Near_Message_0));

            CRCUtils::compareKnownCRC(
                pCurrentPacket, nTotalSize,
                sizeof(RadarTypes::tSOMEIPHeader),
                sizeof(RadarTypes::tSOMEIPPayloadHeader));

            CRCUtils::debugAllCRCCombinations(
                pCurrentPacket, nTotalSize,
                sizeof(RadarTypes::tSOMEIPHeader),
                sizeof(RadarTypes::tSOMEIPPayloadHeader));
        }

        /* ------------------- CRC DEBUG END -------------------------- */

        eResult = ValidatePacket(pCurrentPacket, nTotalSize, nMessageID);
        if (eResult != EValidationResult::OK) {
            LOG_WARNING("Decode failed at ts=%lld: %s",
            static_cast<long long>(pSample->GetTime()),
            toString(eResult));
        RETURN_NOERROR;
        }
        m_pWriter->Write(pSample);

    } else {
        LOG_ERROR("Error obtaining the sample!");
        RETURN_ERROR(ERR_FAILED);
    }

    RETURN_NOERROR;
}

tResult cPacketParserFilter::ProcessSample(adtf::ucom::object_ptr<const adtf::streaming::ISample>& pInSample) {
    LOG_INFO("Processing sample");
    // tFloat64 val = adtf::streaming::sample_data<tFloat64>(pInSample);
    // adtf::streaming::output_sample_data<tFloat64> oData(adtf::streaming::get_sample_time(pInSample));
    // *m_pWriter << oData.Release();
    // m_pWriter->ManualTrigger();
    m_pWriter->Write(pInSample);
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

tResult cPacketParserFilter::byteSwap(tEthernetPacket* oMessage) {
    
    //uint32_t nMessageID = (uint32_t)__builtin_bswap16(oEditablePacket.sSOMEIPHeader.nServiceID) << 16 | __builtin_bswap16(oEditablePacket.sSOMEIPHeader.nMethodID);
    //uint32_t nOurLength = __builtin_bswap32(oEditablePacket.sSOMEIPHeader.nLength);

    RETURN_NOERROR;
}


EValidationResult cPacketParserFilter::ValidatePacket(const uint8_t* oMessage, const uint32_t nLen, uint32_t& nMessageID)
{
    /* [1] Completeness check
    1) SOME/IP Header --> Big endian conversion
    2) Bytes covered by Length + 4 bytes Length + 4 bytes MessageID (+8)
    */
    if (nLen < sizeof(RadarTypes::tSOMEIPHeader))
    {
        return EValidationResult::ERR_BUFFER_TOO_SHORT;
    }

    RadarTypes::tSOMEIPHeader oSOMEIP{};
    std::memcpy(&oSOMEIP, oMessage, sizeof(RadarTypes::tSOMEIPHeader));
    nMessageID =  (uint32_t)__builtin_bswap16(oSOMEIP.nServiceID) << 16 | __builtin_bswap16(oSOMEIP.nMethodID);

    const size_t nExpectedTotal = expectedTotalSize(nMessageID);
    if (nExpectedTotal == 0)
    {
        return EValidationResult::ERR_UNKNOWN_MESSAGE_ID;
    }

    // SOME/IP nLength field sanity check
    // nLength covers everything after the first 8 bytes (after ServiceID+MethodID+Length)
    const size_t nExpectedSOMEIPLength =
        nExpectedTotal - offsetof(RadarTypes::tSOMEIPHeader, nClientID);
    uint32_t someIPHeaderLen = __builtin_bswap32(oSOMEIP.nLength); // byte-swap required!
    if (someIPHeaderLen != nExpectedSOMEIPLength)
    {
        return EValidationResult::ERR_SOMEIP_LENGTH;
    }

    if (nLen < nExpectedTotal)
    {
        return EValidationResult::ERR_PAYLOAD_TOO_SHORT;
    }
    /* 
    [3] Payload header check --> check if len is the same as in the someip header
    */
    const uint8_t* pPayload = oMessage + sizeof(RadarTypes::tSOMEIPHeader);
    RadarTypes::tSOMEIPPayloadHeader oPayloadHeader{};
    std::memcpy(&oPayloadHeader, pPayload, sizeof(RadarTypes::tSOMEIPPayloadHeader));

    uint32_t payloadHeaderLen = (uint32_t(__builtin_bswap16(oPayloadHeader.nLen))); // byteswap-required
    // if the two length are not the same the packet could be badly formatted
    if ( payloadHeaderLen != someIPHeaderLen)
    {   
        return EValidationResult::ERR_PAYLOAD_LENGTH;
    }

   
    /* 
    [4] Checksum
    */

    /*
    const uint8_t* pPayloadData = pPayload + sizeof(RadarTypes::tSOMEIPPayloadHeader);
    size_t payloadDataLen = nLen - sizeof(RadarTypes::tSOMEIPHeader) - sizeof(RadarTypes::tSOMEIPPayloadHeader);
    const uint16_t nComputedCRC = computeCRC16(pPayloadData, payloadDataLen);
    if (nComputedCRC != __builtin_bswap16(oPayloadHeader.nCRC))
    {
        LOG_WARNING("CRC Expected: %d", __builtin_bswap16(oPayloadHeader.nCRC));
        return EValidationResult::ERR_CRC;
    }
    */

    return EValidationResult::OK;
}