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
        "UDP/SOMEIP Parser Filter");

    cPacketParserFilter();

    tResult ProcessInput(ISampleReader* pReader,
        const iobject_ptr<const ISample>& pSample) override;

private:
    ISampleReader* m_pRawReader  = nullptr;
    ISampleWriter* m_pPayloadWriter = nullptr;
};