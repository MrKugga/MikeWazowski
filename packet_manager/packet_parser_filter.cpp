#include "packet_parser_filter.h"

ADTF_PLUGIN(NAME_CUSTOM_FILTER, cPacketParserFilter);

struct tEthernetStream {
    static constexpr const tChar* const MetaTypeName = "radar/IP_Stream";
    //RadarTypes::tEthernetHeader sEthernetHeader;                 // 4 bytes
    //RadarTypes::tIPHeader sIPHeader;                        // 20 bytes     
    //RadarTypes::tUDPHeader sUDPHeader;
    RadarTypes::tSOMEIPHeader sSOMEIPHeader;
    // tSOMEIPPayloadHeader sSOMEIPPayloadHeader;
};

struct tAdtfNetworkHeader
{
    tUInt64 nTimestampUs;   // capture timestamp in microseconds
    tUInt32 nFrameLength;   // total captured frame size
    tUInt32 nOrigLength;    // original length (may differ if truncated)
    tUInt8  nInterfaceId;   // source network interface index
    tUInt8  nPadding[3];
};

cPacketParserFilter::cPacketParserFilter()
{
    LOG_INFO("Initializing filter...");
    
    SetDescription("Radar Packed Decoder");

    adtf::ucom::object_ptr<adtf::streaming::IStreamType const> pStreamType = 
        adtf::ucom::make_object_ptr<stream_meta_type<tEthernetStream>>();

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



    auto oDescEthStream = adtf::mediadescription::structure<tEthernetStream>("tEthernetStream");
    // oDescEthStream.createElement("sEthernetHeader", &tEthernetStream::sEthernetHeader, oDescEthernetHeader);
    // oDescEthStream.createElement("sIPHeader", &tEthernetStream::sIPHeader, oDescIPHeader);
    // oDescEthStream.createElement("sUDPHeader", &tEthernetStream::sUDPHeader, oDescUDPHeader);
    oDescEthStream.createElement("sSOMEIPHeader", &tEthernetStream::sSOMEIPHeader, oDescSOMEIPHeader);



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

        tUInt32 nTotalSize = static_cast<tUInt32>(pSampleBuffer->GetSize());
        //LOG_INFO("Size: 0x%04x", nTotalSize);

        const tEthernetStream* pVal = reinterpret_cast<const tEthernetStream*>(pSampleBuffer->GetPtr());
        const tEthernetStream oVal = *pVal;

        const uint32_t nMessageID = __builtin_bswap16(((uint32_t)oVal.sSOMEIPHeader.nServiceID << 16 | oVal.sSOMEIPHeader.nMethodID));

        LOG_INFO("MessageID: 0x%08x", nMessageID);
        LOG_INFO("Expected MessageID: 0x%08x", RadarTypes::MESSAGEID_OBJECTS_0);

        switch (nMessageID) {
            case RadarTypes::MESSAGEID_SENSORCONFIG:
                LOG_INFO("Sensor config!");
                break;

            case RadarTypes::MESSAGEID_VEHDYN:
                LOG_INFO("Vehicle Dynamics");
                break;

            case RadarTypes::MESSAGEID_SENSORSTATUS:
                LOG_INFO("SensorStatus");
                break;

            case RadarTypes::MESSAGEID_OBJECTS_0:
                LOG_INFO("Objects0");
                break;

            case RadarTypes::MESSAGEID_OBJECTS_1:
                LOG_INFO("Objects1");
                break;

            case RadarTypes::MESSAGEID_RDINEAR_0:
                LOG_INFO("RDINEAR0");
                break;

            case RadarTypes::MESSAGEID_RDINEAR_1:
                LOG_INFO("RDINEAR1");
                break;

            case RadarTypes::MESSAGEID_RDINEAR_2:
                LOG_INFO("RDINEAR2");
                break;

            case RadarTypes::MESSAGEID_RDIFAR_0:
                LOG_INFO("RDIFAR0");
                break;

            case RadarTypes::MESSAGEID_RDIFAR_1:
                LOG_INFO("RDIFAR1");
                break;
        } 

        //const tUInt64 nSwappedVal = __builtin_bswap64(nVal);
        //LOG_INFO("Data: 0x%08x", nOurServiceID);

        // const tEthernetStream* val = reinterpret_cast<const tEthernetStream*>(pSampleBuffer->GetPtr());
        // LOG_INFO("%hn", &val->sEthernetHeader.nEtherType);
        m_pWriter->Write(pSample);

    } else {
        LOG_ERROR("ERROR!");
        RETURN_ERROR(ERR_INVALID_ADDRESS);
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