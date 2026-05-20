#include "packet_parser_filter.h"
ADTF_PLUGIN(NAME_CUSTOM_FILTER, cPacketParserFilter);

cPacketParserFilter::cPacketParserFilter()
{
    LOG_INFO("Initializing filter...");
    
    SetDescription("Radar Packed Decoder");

    m_pReader = CreateInputPin("Raw UDP Input");
    m_pWriter = CreateOutputPin("SOME/IP Output");

    RegisterPropertyVariable("expected_service_id", m_nExpectedServiceId);
    RegisterPropertyVariable("expected_method_id",  m_nExpectedMethodId);

}

tResult cPacketParserFilter::ProcessInput(adtf::streaming::ISampleReader* pReader ,
    const adtf::ucom::iobject_ptr<const adtf::streaming::ISample>& pSample)
{

    LOG_INFO("Processing data...");
    
    adtf::ucom::object_ptr<const adtf::streaming::ISample> pReadSample;
    
    while(IS_OK(m_pReader->GetNextSample(pReadSample))) {
        LOG_INFO("Okay");
        RETURN_IF_FAILED(ProcessSample(pReadSample));
    }

    RETURN_NOERROR;
}

tResult cPacketParserFilter::ProcessSample(adtf::ucom::object_ptr<const adtf::streaming::ISample>& pSample) {
    LOG_INFO("I'm processing something");
    RETURN_NOERROR;
}