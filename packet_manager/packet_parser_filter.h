#pragma once
#include <adtffiltersdk/adtf_filtersdk.h>

#define CID_CUSTOM_FILTER "packetpareser.filter.radar.cid"
#define NAME_CUSTOM_FILTER "UDP Radar Decoder"


using namespace adtf::util;
using namespace adtf::ucom;
using namespace adtf::streaming;
using namespace adtf::filter;


struct tSomeIpHeader
{
    uint16_t service_id;
    uint16_t method_id;
    uint32_t length;       // lunghezza payload + 8 bytes (da RequestID in poi)
    uint16_t client_id;
    uint16_t session_id;
    uint8_t  protocol_version;
    uint8_t  interface_version;
    uint8_t  message_type;
    uint8_t  return_code;
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

    tResult ProcessSample(adtf::ucom::object_ptr<const adtf::streaming::ISample>& pSample);

private:
    adtf::streaming::ISampleReader* m_pReader = nullptr; // Anonymous stream (UDP Raw bytes)
    adtf::streaming::ISampleWriter* m_pWriter = nullptr; // Anonymous out (SOME/IP Payload)

    // Properties
    adtf::base::property_variable<uint16_t> m_nExpectedServiceId{0x0000};
    adtf::base::property_variable<uint16_t> m_nExpectedMethodId {0xFFFF};

};

