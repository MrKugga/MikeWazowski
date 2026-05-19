#include "packet_parser_filter.h"
ADTF_PLUGIN("UDP SOMEIP Parser Plugin", cPacketParserFilter);

// Ethernet II header: 14 bytes
// IPv4 header:        20 bytes (no options assumed)
// UDP header:          8 bytes
// Total offset:       42 bytes
static constexpr size_t ETH_HEADER_SIZE  = 14;
static constexpr size_t IPV4_HEADER_SIZE = 20;
static constexpr size_t UDP_HEADER_SIZE  =  8;
static constexpr size_t TOTAL_HDR_OFFSET =
    ETH_HEADER_SIZE + IPV4_HEADER_SIZE + UDP_HEADER_SIZE;

cPacketParserFilter::cPacketParserFilter()
{
    m_pRawReader1    = CreateInputPin("raw_udp_frame1", false);
    m_pRawReader2    = CreateInputPin("raw_udp_frame2", false);
    m_pPayloadWriter = CreateOutputPin("radar_payload");
    SetDescription("Strips Ethernet IP UDP headers and forwards payload.");
    LOG_INFO("Constructing the constructor");
}

tResult cPacketParserFilter::ProcessInput(adtf::streaming::ISampleReader* pReader ,
    const adtf::ucom::iobject_ptr<const adtf::streaming::ISample>& pSample)
{
    if(pSample.Get()) {

        LOG_INFO("Getting asmple");
        adtf::ucom::object_ptr<const ISample> oLastOtherSample;

        RETURN_IF_FAILED(m_pRawReader1->GetLastSample(oLastOtherSample));
        uint32_t nSampleValue = adtf::streaming::sample_data<uint32_t>(pSample) + adtf::streaming::sample_data<uint32_t>(oLastOtherSample);

        m_pPayloadWriter->Write(pSample);
        RETURN_NOERROR;
    } else {LOG_INFO("Not getting asmple"); RETURN_NOERROR;}


    // // Lock sample data for reading
    // adtf::ucom::iobject_ptr<const ISampleBuffer>& oSampleBuffer;
    // RETURN_IF_FAILED(pSample->Lock(&oSampleBuffer));

    // const uint8_t* pRaw = static_cast<const uint8_t*>(oSampleBuffer->GetPtr());
    // const size_t   nLen = oSampleBuffer->getSize();

    // if (nLen <= TOTAL_HDR_OFFSET)
    // {
    //     LOG_WARNING("UDP Parser: packet too short (%zu bytes)", nLen);
    //     RETURN_NOERROR;
    // }

    // // Optional: validate EtherType = 0x0800 (IPv4)
    // uint16_t nEtherType = (uint16_t)((pRaw[12] << 8) | pRaw[13]);
    // if (nEtherType != 0x0800)
    // {
    //     RETURN_NOERROR; // skip non-IPv4
    // }

    // // Optional: validate IP protocol = 0x11 (UDP)
    // if (pRaw[ETH_HEADER_SIZE + 9] != 0x11)
    // {
    //     RETURN_NOERROR;
    // }

    // const uint8_t* pPayload = pRaw + TOTAL_HDR_OFFSET;
    // const size_t   nPayloadLen = nLen - TOTAL_HDR_OFFSET;

    // // Write payload as new sample, preserving timestamp
    // output_sample_data<uint8_t[]> oOut(pSample->GetTime(), nPayloadLen);
    // std::memcpy(oOut.begin(), pPayload, nPayloadLen);
    // RETURN_IF_FAILED(m_pPayloadWriter->Write(oOut.Release()));

    RETURN_NOERROR;
}