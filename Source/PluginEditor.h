#pragma once
#include "PluginProcessor.h"
#include <memory>
#include <vector>

class SonariumLook final : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    void drawButtonText(juce::Graphics&,juce::TextButton&,bool,bool) override;
};

class SonariumRadar final : public juce::Component,private juce::Timer {
public:
    explicit SonariumRadar(SonariumAudioProcessor&);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    // Deterministic frame stepping also allows headless image regression tests.
    void advanceFrame() noexcept {angle=std::fmod(angle+.041f,sonarium::tau); repaint();}
    float getSweepAngle() const noexcept {return angle;}
private:
    void timerCallback() override {if(isShowing())advanceFrame();}
    SonariumAudioProcessor& processor;
    void interact(juce::Point<float>);
    float angle=0.0f;
};

class SonariumTapeReel final : public juce::Component,private juce::Timer {
public:
    SonariumTapeReel();
    void paint(juce::Graphics&) override;
    void advanceFrame() noexcept {rotation=std::fmod(rotation+.113f,sonarium::tau); repaint();}
    float getReelAngle() const noexcept {return rotation;}
private:
    void timerCallback() override {if(isShowing())advanceFrame();}
    float rotation=0.0f;
};

class SonariumMeter final : public juce::Component,private juce::Timer {
public:
    explicit SonariumMeter(SonariumAudioProcessor&);
    void paint(juce::Graphics&) override;
private:
    void timerCallback() override {if(isShowing())repaint();}
    SonariumAudioProcessor& processor;
};

class SonariumAudioProcessorEditor final : public juce::AudioProcessorEditor,private juce::Timer {
public:
    explicit SonariumAudioProcessorEditor(SonariumAudioProcessor&);
    ~SonariumAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    struct Dial {
        juce::String id,label;
        float x,y,diameter;
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };
    void addDial(const char*,const char*,float,float,float=72.0f);
    void addButton(juce::TextButton&,const juce::String&,std::function<void()>);
    void timerCallback() override;
    SonariumAudioProcessor& processor;
    SonariumLook look;
    std::vector<Dial> dials;
    SonariumRadar radar;
    SonariumMeter meter;
    std::array<SonariumTapeReel,2> reels;
    juce::MidiKeyboardComponent keyboardView;
    juce::TextButton presetPrev,presetNext,saveButton,loadButton;
    std::array<juce::TextButton,3> modeButtons;
    std::array<juce::TextButton,3> tapeButtons;
    juce::Label presetLabel,polyLabel;
    std::unique_ptr<juce::FileChooser> presetChooser;
    std::unique_ptr<juce::Slider> polySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> polyAttach;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SonariumAudioProcessorEditor)
};
