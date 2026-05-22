#pragma once
#include <adtffiltersdk/adtf_filtersdk.h>
#include "../assignment/RadarTypes.h"

#define CID_CUSTOM_FILTER "packetpareser.filter.radar.cid"
#define NAME_CUSTOM_FILTER "UDP Radar Decoder"
#define HEADERS_SIZE 24 


using namespace adtf::util;
using namespace adtf::ucom;
using namespace adtf::streaming;
using namespace adtf::filter;


struct tEthernetPacket {
    static constexpr const tChar* const MetaTypeName = "radar/IP_Stream";
    //RadarTypes::tEthernetHeader sEthernetHeader;                 // 4 bytes
    //RadarTypes::tIPHeader sIPHeader;                        // 20 bytes     
    //RadarTypes::tUDPHeader sUDPHeader;
    RadarTypes::tSOMEIPHeader sSOMEIPHeader;
    // tSOMEIPPayloadHeader sSOMEIPPayloadHeader;
};

struct tRDI_Near0_Packet {
    RadarTypes::tSOMEIPHeader sSOMEIPHeader;
    RadarTypes::tRDI_Near_Message_0 sRDI_Near0;
};

enum class cSomeIpMessageType : uint8_t {
    eRequest            = 0x00,
    eRequestNoReturn    = 0x01,
    eNotification       = 0x02,
    eResponse           = 0x80,
    eError              = 0x81,
};


class cPacketParserFilter : public adtf::filter::cFilter

{
public:
    ADTF_CLASS_ID_NAME(cPacketParserFilter,
        CID_CUSTOM_FILTER,
        NAME_CUSTOM_FILTER);

    cPacketParserFilter();
    virtual ~cPacketParserFilter() = default;

    tResult ProcessInput(adtf::streaming::ISampleReader* pReader,
        const   adtf::ucom::iobject_ptr<const adtf::streaming::ISample>& pSample) override;
    
    // lifecycle
    tResult Init(tInitStage eStage) override;
    // tResult Start() override;
    // tResult Stop() override;
    tResult Shutdown(tInitStage eStage) override;

    tResult ProcessSample(adtf::ucom::object_ptr<const adtf::streaming::ISample>& pInSample);
    tResult checkCompleteness(const tEthernetPacket* oMessage, const uint32_t nSize);
    tResult byteSwap(tEthernetPacket* oMessage);
    tResult decodeMessage(const tEthernetPacket* oMessage);


private:
    adtf::streaming::ISampleReader* m_pReader = nullptr; // Anonymous stream (UDP Raw bytes)
    adtf::streaming::ISampleWriter* m_pWriter = nullptr; // Anonymous out (SOME/IP Payload)

    char m_pEthHeaderBuffer[HEADERS_SIZE];
    tInt32 m_nByteRead = 0;
    tTimeStamp m_tmSampleTime = 0;

    // Properties
    adtf::base::property_variable<uint16_t> m_nExpectedServiceId{0x0000};
    adtf::base::property_variable<uint16_t> m_nExpectedMethodId {0xFFFF};

};

