#pragma once
#include <JuceHeader.h>
#include "SonariumDSP.h"
#include <array>
#include <atomic>

class SonariumAudioProcessor final : public juce::AudioProcessor {
public:
    SonariumAudioProcessor();
    ~SonariumAudioProcessor() override = default;
    void prepareToPlay(double sampleRate,int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override {return true;}
    const juce::String getName() const override {return JucePlugin_Name;}
    bool acceptsMidi() const override {return true;}
    bool producesMidi() const override {return false;}
    bool isMidiEffect() const override {return false;}
    double getTailLengthSeconds() const override {return 15.0;}
    int getNumPrograms() override {return 1;}
    int getCurrentProgram() override {return 0;}
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override {return {};}
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    using APVTS=juce::AudioProcessorValueTreeState;
    APVTS state;
    juce::MidiKeyboardState keyboard;
    std::atomic<float> meterL{0},meterR{0};
    void loadPreset(int index);
    int getPresetIndex() const noexcept {return presetIndex.load();}
    juce::String getPresetName() const;
    static constexpr int presetCount=9;
    static APVTS::ParameterLayout createLayout();
private:
    sonarium::Params snapshot() const noexcept;
    std::array<sonarium::Voice,16> voices;
    sonarium::TapeEcho tape;
    std::array<juce::Reverb,3> reverbs;
    std::array<juce::AudioBuffer<float>,3> reverbBuffers;
    int reverbScratchSize=256;
    float parameterSmoothing=0.001f;
    sonarium::Params interpolatedParams{};
    uint32_t randomState=0x37aacc11u;
    uint64_t voiceClock=0;
    std::atomic<int> presetIndex{0};
    float lLevel=0,rLevel=0,masterSmooth=0.75f;
    float pitchBend=0.0f,modWheel=0.0f;
    std::array<float,3> wetSmoothed{};
    double currentSampleRate=48000.0,lastNoteHz=0.0;
    bool pedal=false;
    std::array<bool,128> keysDown{};
    void noteOn(int note,float velocity,int engine,int poly);
    void noteOff(int note);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SonariumAudioProcessor)
};
