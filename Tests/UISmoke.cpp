#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <JuceHeader.h>
#include <cmath>
#include <iostream>
#include <memory>
#include <cstdlib>
static void verify(bool v,const char* msg){
    if(!v){std::cerr<<"FAIL SonariumUIStress: "<<msg<<"\n";std::exit(1);}
}
static uint64_t signature(const juce::Image& img){
    uint64_t h=1469598103934665603ULL;
    for(int y=0;y<img.getHeight();y+=3)
        for(int x=0;x<img.getWidth();x+=3){
            h^=img.getPixelAt(x,y).getARGB();h*=1099511628211ULL;
        }
    return h;
}
int main(){
    juce::ScopedJuceInitialiser_GUI gui;
    SonariumAudioProcessor plugin;
    plugin.prepareToPlay(48000,128);
    juce::AudioBuffer<float> buffer(2,2048);
    for(int k=0;k<8;k++){
        std::unique_ptr<juce::AudioProcessorEditor> editor(plugin.createEditor());
        verify(editor!=nullptr,"editor could not open");
        editor->setSize(k%2?1170:1600,k%2?660:900);
        verify(editor->getNumChildComponents()>30,"missing UI components");
        juce::Image frame(juce::Image::ARGB,editor->getWidth(),editor->getHeight(),true);
        juce::Graphics g(frame);
        // Direct paint is correct for a detached editor in headless CI.
        editor->paint(g);
        verify(frame.getPixelAt(1,1).getAlpha()>0,"editor failed to paint");
        if(k==0){
            juce::Slider* drive=nullptr;
            for(int i=0;i<editor->getNumChildComponents();i++)
                if(auto* sl=dynamic_cast<juce::Slider*>(editor->getChildComponent(i)))
                    if(sl->getName()=="DRIVE"){drive=sl;break;}
            verify(drive!=nullptr,"drive knob absent");
            auto paintKnob=[drive](){
                juce::Image image(juce::Image::ARGB,drive->getWidth(),drive->getHeight(),true);
                {juce::Graphics graphics(image);drive->paint(graphics);}
                return signature(image);
            };
            drive->setValue(.04,juce::sendNotificationSync);
            const auto low=drive->getValue();
            const auto before=paintKnob();
            drive->setValue(.96,juce::sendNotificationSync);
            verify(std::abs(drive->getValue()-low)>.7,"knob parameter does not respond");
            verify(before!=paintKnob(),"knob value does not change graphics");
            SonariumRadar radar(plugin);
            radar.setSize(410,410);
            auto paintRadar=[&radar](){
                juce::Image image(juce::Image::ARGB,410,410,true);
                {juce::Graphics graphics(image);radar.paint(graphics);}return signature(image);
            };
            const auto radarBefore=paintRadar();
            for(int i=0;i<15;i++)radar.advanceFrame();
            verify(radarBefore!=paintRadar(),"sonar sweep is static");
            SonariumTapeReel reel;
            reel.setSize(118,86);
            auto paintReel=[&reel](){
                juce::Image image(juce::Image::ARGB,118,86,true);
                {juce::Graphics graphics(image);reel.paint(graphics);}return signature(image);
            };
            const auto reelBefore=paintReel();
            for(int i=0;i<12;i++)reel.advanceFrame();
            verify(reelBefore!=paintReel(),"tape reels are static");
        }
        for(int m=0;m<3;m++){
            if(auto* param=plugin.state.getParameter("mode"))
                param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(m)));
            juce::MidiBuffer midi;
            midi.addEvent(juce::MidiMessage::noteOn(1,48+m*12,juce::uint8(105)),0);
            midi.addEvent(juce::MidiMessage::noteOff(1,48+m*12),800);
            buffer.setSize(2,2048,false,false,true); // 16x declared block: no callback allocations
            plugin.processBlock(buffer,midi);
            for(int channel=0;channel<2;channel++)
                for(int i=0;i<buffer.getNumSamples();i++)
                    verify(std::isfinite(buffer.getSample(channel,i))&&std::abs(buffer.getSample(channel,i))<1.01f,
                           "non-finite or unsafe output");
        }
        editor.reset(); // destruction path Cubase uses on closing plugin windows
    }
    juce::MemoryBlock state;
    plugin.getStateInformation(state);
    verify(state.getSize()>0,"state serialization failed");
    plugin.setStateInformation(state.getData(),static_cast<int>(state.getSize()));
    plugin.releaseResources();
    std::cout<<"PASS: 8 editor open/close cycles, 2 sizes, paint, rotating knobs, 3 engines, "
                "oversized audio blocks, animated radar/reels, state roundtrip\n";
    return 0;
}
