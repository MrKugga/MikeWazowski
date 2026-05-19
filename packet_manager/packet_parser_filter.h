#pragma once
#include <adtffiltersdk/adtf_filtersdk.h>

using namespace adtf::util;
using namespace adtf::ucom;
using namespace adtf::streaming;
using namespace adtf::filter;

class cPacketParserFilter : public cFilter
{
public:
    ADTF_CLASS_ID_NAME(cPacketParserFilter,
        "packetparser.filter.radar.adtf",
        "UDP SOMEIP Parser Filter");

    cPacketParserFilter();

    tResult ProcessInput(adtf::streaming::ISampleReader* pReader,
        const   adtf::ucom::iobject_ptr<const adtf::streaming::ISample>& pSample) override;

private:
    ISampleReader* m_pRawReader1 = nullptr;
    ISampleReader* m_pRawReader2 = nullptr;
    ISampleWriter* m_pPayloadWriter = nullptr;
};