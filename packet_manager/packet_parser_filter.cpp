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
        const tEthernetPacket* pCurrentPacket = reinterpret_cast<const tEthernetPacket*>(pSampleBuffer->GetPtr());
        
        // Check if message is complete
        RETURN_IF_FAILED(checkCompleteness(pCurrentPacket, nTotalSize));


        // Check if message is correct
        // ???
        
        // Check if message is corrupted
        // ???

        // Process message based on ServiceID+MethodID

        tDecodedMessage oDecodedMessage;
        RETURN_IF_FAILED(decodeMessage(pCurrentPacket, oDecodedMessage));

        
        LOG_INFO("Fuori dalla funzione; %d", oDecodedMessage->sRDI_Near0.nTimeStamp;)


        //const uint32_t nMessageID = (uint32_t)__builtin_bswap16(oEditablePacket.sSOMEIPHeader.nServiceID) << 16 | __builtin_bswap16(oEditablePacket.sSOMEIPHeader.nMethodID);

        //LOG_INFO("MessageID: 0x%08x", nMessageID);
        //LOG_INFO("Expected MessageID: 0x%08x", RadarTypes::MESSAGEID_OBJECTS_0);



        //const tUInt64 nSwappedVal = __builtin_bswap64(nVal);
        //LOG_INFO("Data: 0x%08x", nOurServiceID);

        // const tEthernetPacket* val = reinterpret_cast<const tEthernetPacket*>(pSampleBuffer->GetPtr());
        // LOG_INFO("%hn", &val->sEthernetHeader.nEtherType);
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

// Check if the message is complete by comparing size of semple with length of message in the header
tResult cPacketParserFilter::checkCompleteness(const tEthernetPacket* oMessage, const uint32_t nSize) {
    
    // SOME/IP Header --> Big endian conversion
    uint32_t nExpectedLength = __builtin_bswap32(oMessage->sSOMEIPHeader.nLength) + 8; // Bytes covered by Length + 4 bytes Length + 4 bytes MessageID
    
    if(nSize < nExpectedLength) {
        LOG_WARNING("SOME/IP Message is not complete. Dropping sample.");
        RETURN_ERROR(ERR_FAILED);
    }

    RETURN_NOERROR;
}

// Decode message based on ServiceID and MethodID
tResult cPacketParserFilter::decodeMessage(const tEthernetPacket* oMessage, tDecodedMessage& oDecodedOutput) {
    
    // SOME/IP Header --> big-endian conversion
    const uint32_t nMessageID = 
        (uint32_t)__builtin_bswap16(oMessage->sSOMEIPHeader.nServiceID) 
        << 16 | __builtin_bswap16(oMessage->sSOMEIPHeader.nMethodID);


    // Decode sample in corresponding struct depending on the MessageID
    switch(nMessageID)
    {
        case RadarTypes::MESSAGEID_SENSORCONFIG: {
            break;
        }
        
        case RadarTypes::MESSAGEID_VEHDYN: {
            break;
        }

        case RadarTypes::MESSAGEID_SENSORSTATUS: {
            break;
        }

        case RadarTypes::MESSAGEID_OBJECTS_0: {
            const tObject0_Packet* oDecodedMessage = reinterpret_cast<const tObject0_Packet*>(oMessage);
            const int16_t nObjX = oDecodedMessage->sObject0_msg.aObj.sObj[0].fDistX;
            const int16_t nObjY = oDecodedMessage->sObject0_msg.aObj.sObj[0].fDistY;
            LOG_INFO("Object position: (%f, %f)", (double)nObjX*RadarTypes::RES_F_DISTX, (double)nObjY*RadarTypes::RES_F_DISTY);
            break;
        }

        case RadarTypes::MESSAGEID_OBJECTS_1: {
            const tObject1_Packet* oDecodedMessage = reinterpret_cast<const tObject1_Packet*>(oMessage);
            break;
        }

        case RadarTypes::MESSAGEID_RDINEAR_0: {
            const tRDI_Near0_Packet* oDecodedMessage = reinterpret_cast<const tRDI_Near0_Packet*>(oMessage);
            const uint32_t nTimeStamp = oDecodedMessage->sRDI_Near0.nTimeStamp;
            LOG_INFO("Timestamp: %d", nTimeStamp);
            break;
        }

        case RadarTypes::MESSAGEID_RDINEAR_1: {
            const tRDI_Near1_Packet* oDecodedMessage = reinterpret_cast<const tRDI_Near1_Packet*>(oMessage);
            break;
        }

        case RadarTypes::MESSAGEID_RDINEAR_2: {
            const tRDI_Near2_Packet* oDecodedMessage = reinterpret_cast<const tRDI_Near2_Packet*>(oMessage);
            break;
        }

        case RadarTypes::MESSAGEID_RDIFAR_0: {
            const tRDI_Far0_Packet* oDecodedMessage = reinterpret_cast<const tRDI_Far0_Packet*>(oMessage);
            break;
        }

        case RadarTypes::MESSAGEID_RDIFAR_1: {
        const tRDI_Far1_Packet* oDecodedMessage = reinterpret_cast<const tRDI_Far1_Packet*>(oMessage);
            break;
        }

        default:
            RETURN_ERROR(ERR_NOT_FOUND);
            break;

    }

    RETURN_NOERROR;
}