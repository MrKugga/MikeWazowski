#pragma once
#include <adtffiltersdk/adtf_filtersdk.h>
#include "../assignment/RadarTypes.h"
#include "validator.h"
#include "decoded_messages.h"
#include "decoder.h"
#include "crc_utils.h"
#include "cycle_accumulator.h"
#include <variant>


#define CID_CUSTOM_FILTER "packetpareser.filter.radar.cid"
#define NAME_CUSTOM_FILTER "UDP Radar Decoder"
#define HEADERS_SIZE 24 


using namespace adtf::util;
using namespace adtf::ucom;
using namespace adtf::streaming;
using namespace adtf::filter;

class cPacketParserFilter : public adtf::filter::cFilter

{
public:
    ADTF_CLASS_ID_NAME(cPacketParserFilter,
        CID_CUSTOM_FILTER,
        NAME_CUSTOM_FILTER);

    cPacketParserFilter();
    virtual ~cPacketParserFilter() = default;

    tResult ProcessInput(
        adtf::streaming::ISampleReader* pReader,
        const   adtf::ucom::iobject_ptr<const adtf::streaming::ISample>& pSample) override;
    
    // lifecycle
    tResult Init(tInitStage eStage) override;
    // tResult Start() override;
    // tResult Stop() override;
    tResult Shutdown(tInitStage eStage) override;


private:
    //adtf::streaming::ISampleWriter* m_pWriter = nullptr; // Anonymous out (SOME/IP Payload) --> old implementation

    // ── Input ─────────────────────────────────────────────────────────────
    adtf::streaming::ISampleReader* m_pReader = nullptr;
    adtf::streaming::ISampleReader* m_pVehDynReader = nullptr;

    // ── Outputs — one UDP stream + one pin per decoded message type ────────────────────────
    adtf::streaming::ISampleWriter* m_pUDPWriter = nullptr;
    adtf::streaming::ISampleWriter* m_pRDIWriter        = nullptr;
    adtf::streaming::ISampleWriter* m_pObjectWriter     = nullptr;
    adtf::streaming::ISampleWriter* m_pStatusWriter     = nullptr;
    adtf::streaming::ISampleWriter* m_pVehDynWriter     = nullptr;

    // ── Cycle accumulator ─────────────────────────────────────────────────
    RadarDecoder::cCycleAccumulator m_oAccumulator;

    // ── Debug state ───────────────────────────────────────────────────────
    uint32_t m_nPacketCount  = 0;
    bool     m_bDebugDone    = false;

    // ── Internal write helpers ────────────────────────────────────────────

    tResult writeRDI       (const RadarDecoded::tRDIMessage&              msg,
                            uint32_t                                      nMessageID,
                            adtf::base::tNanoSeconds                      tmSample);
    tResult writeObject    (const RadarDecoded::tObjectMessage&           msg,
                            adtf::base::tNanoSeconds                      tmSample);
    tResult writeStatus    (const RadarDecoded::tSensorStatusDecoded&     msg,
                            adtf::base::tNanoSeconds                      tmSample);
    tResult writeVehDyn    (const RadarDecoded::tVehicleDynamicsDecoded&  msg,
                            adtf::base::tNanoSeconds                      tmSample);

    // Properties
    adtf::base::property_variable<uint16_t> m_nExpectedServiceId{0x0000};
    adtf::base::property_variable<uint16_t> m_nExpectedMethodId {0xFFFF};
};

