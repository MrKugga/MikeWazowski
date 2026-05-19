#pragma once
#include <adtffiltersdk/adtf_filtersdk.h>

using namespace adtf::util;
using namespace adtf::ucom;
using namespace adtf::base;
using namespace adtf::streaming;
using namespace adtf::mediadescription;
using namespace adtf::filter;

class cMyFilter : public cFilter
{
public:
    ADTF_CLASS_ID_NAME(cMyFilter, "my_filter.filter.demo", "My Demo Filter");
    ADTF_CLASS_DEPENDENCIES(REQUIRE_INTERFACE(adtf::services::IReferenceClock));

    cMyFilter();

private:

    ISampleWriter* m_pWriter = nullptr;
    ISampleReader* m_pReader = nullptr;

    // A configurable property
    adtf::base::property_variable<tFloat64> m_fThreshold = 0.5;

    tResult ProcessInput(ISampleReader* pReader,
                         const iobject_ptr<const ISample>& pSample) override;
};