#include "helloWorld.h"

// Register the filter class with the plugin system
ADTF_PLUGIN("My Demo Plugin", cMyFilter)

cMyFilter::cMyFilter()
{

    m_pWriter = CreateOutputPin("OutputPin1");
    m_pReader = CreateInputPin("InputPin1", true, true);

    // Expose a property that can be configured in the Configuration Editor
    RegisterPropertyVariable("threshold", m_fThreshold);

    // Bind the callback: whenever a sample arrives on m_oInput, call ProcessInput
    SetDescription("A simple demo filter that forwards data above a threshold.");
}

tResult cMyFilter::ProcessInput(ISampleReader* pReader,
                                const iobject_ptr<const ISample>& pSample)
{
    // // Read the raw buffer from the incoming sample
    // adtf::ucom::object_ptr<const adtf::streaming::ISample> pInSample;
    // RETURN_IF_FAILED(pReader->GetNextSample(pInSample));

    // // Write a new output sample
    // adtf::ucom::object_ptr<adtf::streaming::ISample> pOutSample;
    // RETURN_IF_FAILED(alloc_sample(pOutSample, pInSample->GetTime()));

    // {
    //     // Lock the output sample for writing
    //     adtf::streaming::SampleWriter oWriter(*pOutSample);
    //     // ... copy or transform data here ...
    // }

    // // Emit on the output pin
    // m_oOutput << pOutSample;
    // RETURN_NOERROR;

    LOG_ERROR("stocazza");

    RETURN_NOERROR;
}