#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "Oversampler.h"
const int kNumPrograms = 1;

enum EParams
{
    kMix = 0,
    kPhase,
    kOverSampling,
    kDelay,
    kFdelay,
    kNumParams
};

using namespace iplug;
using namespace igraphics;

class OverSmpTest final : public Plugin
{
public:
    OverSmpTest(const InstanceInfo& info);
    
#if IPLUG_DSP // http://bit.ly/2S64BDd
    void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
    void OnReset() override;
    void OnParamChange(int paramIdx, EParamSource, int sampleOffset) override;
    
private:
    bool mFactorChanged = true;
    OverSampler <sample> mOverSampL {kNone, true, 1};
    OverSampler <sample> mOverSampR {kNone, true, 1}; // init with no upsampling, block processing, mono
    double mix, fdelay;
    int delay, phase;
      double procs[2][8];
#endif
};
