#include "OverSmpTest.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

OverSmpTest::OverSmpTest(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPrograms))
{
    GetParam(kMix)->InitDouble("Mix", 50., 0., 100., 1., "%");
    GetParam(kPhase)->InitInt("Flip", 0, 0, 1, "∅");
    GetParam(kDelay)->InitInt("Delay", 0, 0, 6, "smp");
    GetParam(kFdelay)->InitDouble("Fine", 50., 0., 100., 1, "%");
    GetParam(kOverSampling)->InitEnum("Oversample", 0, 5, "", 0, "", OVERSAMPLING_FACTORS_VA_LIST);
#if IPLUG_EDITOR // http://bit.ly/2S64BDd
    mMakeGraphicsFunc = [&]() {
        return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
    };
    
    mLayoutFunc = [&](IGraphics* pGraphics) {
        pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
        pGraphics->AttachPanelBackground(COLOR_GRAY);
        pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
        const IRECT b = pGraphics->GetBounds();
        
        pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetHShifted(-50).GetVShifted(-50), kMix));
        pGraphics->AttachControl(new IVRadioButtonControl(b.GetCentredInside(100).GetHShifted(50).GetVShifted(-50), kPhase));
        
        pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetHShifted(-50).GetVShifted(50), kDelay));
        pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetHShifted(-50).GetVShifted(150), kFdelay));
        pGraphics->AttachControl(new IVRadioButtonControl(b.GetCentredInside(100).GetHShifted(50).GetVShifted(50), kOverSampling));
    };
#endif
}

#if IPLUG_DSP
void OverSmpTest::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
    if(mFactorChanged)
    {
        mOverSampL.SetOverSampling((EFactor) GetParam(kOverSampling)->Int());
        mOverSampR.SetOverSampling((EFactor) GetParam(kOverSampling)->Int()); // Warning, this could allocate.
        mFactorChanged = false;
    }
    
    for (auto s = 0; s < nFrames; s++) {
        procs[0][0] = inputs[0][s];
        procs[1][0] = inputs[1][s];
        
        outputs[0][s] = mix * (mOverSampL.Process(inputs[0][s], [](sample input) {return input;}))
        + (1. - mix) * ((1. - fdelay) * procs[0][delay] + fdelay * procs[0][delay + 1]) * (double)phase;
        
        outputs[1][s] = mix * (mOverSampR.Process(inputs[1][s], [](sample input) {return input;}))
        + (1. - mix) * ((1. - fdelay) * procs[1][delay] + fdelay * procs[1][delay + 1]) * (double)phase;
        
        //buffer filler
        for (int i = 0; i < 7; i++) {
            procs[0][7 - i] = procs[0][6 - i];
            procs[1][7 - i] = procs[1][6 - i];
        }
    }
}
#endif

void OverSmpTest::OnReset()
{
    mOverSampL.Reset(GetBlockSize());
    mOverSampR.Reset(GetBlockSize());
}

void OverSmpTest::OnParamChange(int paramIdx, EParamSource, int sampleOffset)
{
    switch (paramIdx)
    {
        case kOverSampling:
            mFactorChanged = true; // TODO: maybe check existing factor
            break;
        case kMix:
            mix = GetParam(kMix)->Value() * 0.01;
            break;
        case kPhase:
            phase = 1 + GetParam(kPhase)->Value() * -2;
            break;
        case kDelay:
            delay = GetParam(kDelay)->Value();
            break;
        case kFdelay:
            fdelay = GetParam(kFdelay)->Value() * 0.01;
            break;
            
        default:
            break;
    }
}
