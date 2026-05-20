#include "packet_parser_filter.h"
ADTF_PLUGIN(NAME_CUSTOM_FILTER, cPacketParserFilter);

cPacketParserFilter::cPacketParserFilter()
{
    LOG_INFO("Initializing filter...");
    
    SetDescription("Radar Packed Decoder");

    m_pReader = CreateInputPin("Raw UDP Input");
    m_pWriter = CreateOutputPin<adtf::filter::pin_writer<uint32_t>>("SOME/IP Output", stream_type_plain<uint32_t>());

    RegisterPropertyVariable("expected_service_id", m_nExpectedServiceId);
    RegisterPropertyVariable("expected_method_id",  m_nExpectedMethodId);

}

tResult cPacketParserFilter::ProcessInput(adtf::streaming::ISampleReader* pReader ,
    const adtf::ucom::iobject_ptr<const adtf::streaming::ISample>& pSample)
{

    if(pSample.Get()) {

        LOG_INFO("Processing samples");
        m_pWriter->Write(pSample);

    } else {
        LOG_ERROR("ERROR!");
        RETURN_ERROR(ERR_INVALID_ADDRESS);
    }

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