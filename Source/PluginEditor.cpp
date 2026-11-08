#include "PluginEditor.h"
#include <cmath>
#include <random>

namespace {
constexpr float W=1600.0f,H=900.0f;
const auto ink=juce::Colour::fromRGB(58,39,23);
const auto brass=juce::Colour::fromRGB(176,118,54);
const auto aged=juce::Colour::fromRGB(229,196,142);
const auto cream=juce::Colour::fromRGB(238,214,169);
const auto deep=juce::Colour::fromRGB(35,26,20);
const auto green=juce::Colour::fromRGB(98,248,176);
void label(juce::Graphics& g,const juce::String& text,float x,float y,float w,float h,float size=21,bool bold=false,juce::Colour color=ink) {
    g.setColour(color);
    g.setFont(juce::Font(juce::FontOptions("Georgia",size,bold?juce::Font::bold:juce::Font::plain)));
    g.drawText(text,juce::Rectangle<float>(x,y,w,h),juce::Justification::centred,false);
}
void plate(juce::Graphics& g,juce::Rectangle<float> r,float corner=20.0f){
    g.setColour(juce::Colours::black.withAlpha(.43f));g.fillRoundedRectangle(r.translated(4,6),corner);
    g.setGradientFill(juce::ColourGradient(cream,r.getX(),r.getY(),aged.darker(.27f),r.getRight(),r.getBottom(),false));
    g.fillRoundedRectangle(r,corner);
    g.setColour(ink.withAlpha(.69f));g.drawRoundedRectangle(r,corner,3);
    g.setColour(brass.brighter(.5f).withAlpha(.55f));g.drawRoundedRectangle(r.reduced(6),corner-4,1.3f);
    for(float x:{r.getX()+14,r.getRight()-14})
        for(float y:{r.getY()+14,r.getBottom()-14}){
            g.setColour(ink.darker());g.fillEllipse(x-4,y-4,8,8);
            g.setColour(brass.brighter());g.drawLine(x-2,y-2,x+2,y+2,.9f);
        }
}
void banner(juce::Graphics& g,juce::Rectangle<float> r,const juce::String& text) {
    plate(g,r,12);
    label(g,text,r.getX(),r.getY()+3,r.getWidth(),r.getHeight()-7,32,true);
}
void mapMotif(juce::Graphics& g) {
    g.setColour(ink.withAlpha(.20f));
    for(int i=0;i<14;i++){
        juce::Path p;
        float y=170.0f+i*26.0f;
        p.startNewSubPath(20,y);
        for(float x=20;x<W;x+=16)
            p.lineTo(x,y+11*std::sin(x*.006f+i*.73f)+6*std::sin(x*.017f+i));
        g.strokePath(p,juce::PathStrokeType(.85f));
    }
    for(int i=0;i<6;i++){
        auto x=85.0f+i*270;
        g.drawEllipse(x,80+std::sin(x*.03f)*55,180,80,1.0f);
    }
}
void lighthouse(juce::Graphics& g){
    auto p=juce::Path();
    p.startNewSubPath(72,156);p.lineTo(78,88);p.lineTo(101,88);p.lineTo(107,156);p.closeSubPath();
    g.setColour(ink.withAlpha(.65f));g.fillPath(p);
    g.setColour(aged);g.fillRect(84.0f,100.0f,8.0f,12.0f);
    g.setColour(brass);g.fillRect(72.0f,84.0f,35.0f,6.0f);
    g.setColour(ink);g.drawLine(70,81,89,65,2);g.drawLine(89,65,109,81,2);
    g.setColour(juce::Colours::orange.withAlpha(.15f));
    juce::Path beams;beams.addTriangle(89,96,4,52,190,70);g.fillPath(beams);
}
void ship(juce::Graphics& g) {
    g.setColour(ink.withAlpha(.66f));
    juce::Path hull;hull.startNewSubPath(1400,145);hull.quadraticTo(1491,177,1560,145);hull.lineTo(1538,173);hull.lineTo(1422,173);hull.closeSubPath();g.fillPath(hull);
    g.drawLine(1480,57,1480,148,2);g.drawLine(1525,78,1525,148,2);
    juce::Path sail;sail.startNewSubPath(1477,61);sail.lineTo(1444,132);sail.lineTo(1477,132);sail.closeSubPath();g.fillPath(sail);
    sail.clear();sail.startNewSubPath(1487,68);sail.lineTo(1520,138);sail.lineTo(1487,138);sail.closeSubPath();g.fillPath(sail);
}
void vacTube(juce::Graphics& g) {
    g.setGradientFill(juce::ColourGradient(juce::Colours::orange.brighter(),55,84,deep,55,185,true));
    g.fillRoundedRectangle(30,16,58,181,28);
    g.setColour(brass);g.drawRoundedRectangle(30,16,58,181,28,6);
    g.setColour(cream.withAlpha(.37f));g.drawRoundedRectangle(38,26,42,148,20,2);
    g.setColour(juce::Colours::orange.withAlpha(.6f));g.drawLine(56,72,56,156,7);
    g.setColour(juce::Colours::yellow.withAlpha(.74f));g.drawLine(57,77,57,154,2);
    g.setColour(ink);g.fillRect(30.0f,181.0f,59.0f,20.0f);
    g.setColour(brass);g.drawRect(30,181,59,20,3);
}
void section(juce::Graphics& g,const juce::String& title,juce::Rectangle<float> body) {
    plate(g,body,16);
    juce::Rectangle<float> tag(body.getCentreX()-body.getWidth()*.34f,body.getY()-18,body.getWidth()*.68f,48);
    banner(g,tag,title);
}
}

void SonariumLook::drawRotarySlider(juce::Graphics& g,int x,int y,int width,int height,float pos,float start,float end,juce::Slider&) {
    auto b=juce::Rectangle<float>(static_cast<float>(x),static_cast<float>(y),static_cast<float>(width),static_cast<float>(height));
    auto c=b.getCentre();
    float radius=std::min(b.getWidth(),b.getHeight())*.40f;
    float a=start+pos*(end-start);
    g.setColour(ink.withAlpha(.6f));g.fillEllipse(c.x-radius-5,c.y-radius-3,2*radius+10,2*radius+10);
    g.setColour(juce::Colour::fromRGB(75,51,30));
    g.fillEllipse(c.x-radius-2,c.y-radius-2,2*radius+4,2*radius+4);
    g.setColour(brass.brighter(.56f));g.drawEllipse(c.x-radius-2,c.y-radius-2,2*radius+4,2*radius+4,3);
    auto angle0=start,angle1=end;
    juce::Path ticks;
    for(int i=0;i<=10;i++){
        float t=angle0+(angle1-angle0)*i/10.0f;
        float s=radius+6,e=radius+(i%5==0?14:10);
        ticks.startNewSubPath(c.x+std::sin(t)*s,c.y-std::cos(t)*s);
        ticks.lineTo(c.x+std::sin(t)*e,c.y-std::cos(t)*e);
    }
    g.setColour(ink.withAlpha(.74f));g.strokePath(ticks,juce::PathStrokeType(1.45f));
    juce::ColourGradient metal(juce::Colour::fromRGB(252,207,127),c.x-radius,c.y-radius,
       juce::Colour::fromRGB(90,55,30),c.x+radius,c.y+radius,false);
    metal.addColour(.43,juce::Colour::fromRGB(188,127,65));
    g.setGradientFill(metal);g.fillEllipse(c.x-radius,c.y-radius,2*radius,2*radius);
    g.setColour(cream.withAlpha(.48f));g.drawEllipse(c.x-radius+5,c.y-radius+5,2*radius-10,2*radius-10,1);
    g.setColour(ink.withAlpha(.57f));g.drawEllipse(c.x-radius,c.y-radius,2*radius,2*radius,1.6f);
    g.setColour(deep);g.drawLine(c.x,c.y,c.x+std::sin(a)*radius*.84f,c.y-std::cos(a)*radius*.84f,6.0f);
    g.setColour(cream);g.drawLine(c.x,c.y,c.x+std::sin(a)*radius*.79f,c.y-std::cos(a)*radius*.79f,2.2f);
    g.setColour(juce::Colour::fromRGB(255,165,57).withAlpha(pos>.7f?.7f:.2f));
    g.drawEllipse(c.x-radius-5,c.y-radius-5,radius*2+10,radius*2+10,1.3f);
}
void SonariumLook::drawButtonBackground(juce::Graphics& g,juce::Button& button,const juce::Colour&,bool hovering,bool down) {
    auto b=button.getLocalBounds().toFloat().reduced(1.5f);
    auto selected=button.getToggleState();
    g.setGradientFill(juce::ColourGradient(selected?brass.brighter(.42f):juce::Colour::fromRGB(83,56,35),
       0,0,selected?aged:juce::Colour::fromRGB(45,32,23),b.getRight(),b.getBottom(),false));
    g.fillRoundedRectangle(b,7);
    g.setColour(selected?cream:brass.brighter(.25f));g.drawRoundedRectangle(b,7,2);
    if(hovering||down) {
        g.setColour(cream.withAlpha(down?.16f:.10f));g.fillRoundedRectangle(b.reduced(3),5);
    }
}
void SonariumLook::drawButtonText(juce::Graphics& g,juce::TextButton& button,bool,bool) {
    g.setColour(button.getToggleState()?deep:cream);
    g.setFont(juce::Font(juce::FontOptions("Georgia",juce::jmax(12.0f,button.getHeight()*.43f),juce::Font::bold)));
    g.drawFittedText(button.getButtonText(),button.getLocalBounds(),juce::Justification::centred,1);
}

SonariumRadar::SonariumRadar(SonariumAudioProcessor& p):processor(p){setOpaque(false);startTimerHz(30);}
void SonariumRadar::paint(juce::Graphics& g) {
    auto w=static_cast<float>(getWidth()),h=static_cast<float>(getHeight());
    auto cx=w*.5f,cy=h*.5f,r=std::min(w,h)*.47f;
    g.setColour(deep);g.fillEllipse(cx-r-10,cy-r-10,2*r+20,2*r+20);
    juce::ColourGradient outer(brass.brighter(.7f),cx-r,cy-r,deep,cx+r,cy+r,false);
    g.setGradientFill(outer);g.fillEllipse(cx-r-5,cy-r-5,2*r+10,2*r+10);
    g.setColour(cream.withAlpha(.65f));g.drawEllipse(cx-r-2,cy-r-2,2*r+4,2*r+4,2);
    juce::ColourGradient crt(juce::Colour::fromRGB(21,78,69),cx-r,cy-r,juce::Colour::fromRGB(2,24,29),cx+r,cy+r,true);
    g.setGradientFill(crt);g.fillEllipse(cx-r+9,cy-r+9,2*r-18,2*r-18);
    juce::Graphics::ScopedSaveState saved(g);
    juce::Path mask;mask.addEllipse(cx-r+10,cy-r+10,2*r-20,2*r-20);g.reduceClipRegion(mask);
    g.setColour(green.withAlpha(.15f));
    for(int i=1;i<=5;i++)g.drawEllipse(cx-r*i/5,cy-r*i/5,2*r*i/5,2*r*i/5,1.4f);
    g.drawLine(cx-r,cy,cx+r,cy,.9f);g.drawLine(cx,cy-r,cx,cy+r,.9f);
    for(int d=0;d<360;d+=15){
        float a=d*sonarium::pi/180.0f;
        float ra=r*.92f,rb=r*(d%45==0?.99f:.96f);
        g.drawLine(cx+std::sin(a)*ra,cy-std::cos(a)*ra,cx+std::sin(a)*rb,cy-std::cos(a)*rb,1);
    }
    juce::Path sweep;
    sweep.startNewSubPath(cx,cy);
    for(int i=0;i<=60;i++) {
        float a=angle-.57f+(.57f*i/60.0f);
        sweep.lineTo(cx+std::sin(a)*r,cy-std::cos(a)*r);
    }
    sweep.closeSubPath();
    g.setGradientFill(juce::ColourGradient(green.withAlpha(.42f),cx,cy,green.withAlpha(.025f),cx+r,cy+r,true));
    g.fillPath(sweep);
    g.setColour(green.withAlpha(.86f));
    g.drawLine(cx,cy,cx+std::sin(angle)*r*.95f,cy-std::cos(angle)*r*.95f,2.7f);
    auto get=[this](const char* id){return processor.state.getRawParameterValue(id)->load();};
    float contour=get("depth")*.5f+get("texture")*.5f;
    juce::Path seabed;
    for(int x=0;x<=static_cast<int>(w);x+=3){
        float xx=static_cast<float>(x);
        float yy=cy+r*.49f+22*std::sin(xx*.023f)+14*std::sin(xx*.063f+angle*.27f)+contour*28*std::sin(xx*.095f);
        if(x==0)seabed.startNewSubPath(xx,yy);else seabed.lineTo(xx,yy);
    }
    g.setColour(green.withAlpha(.42f));g.strokePath(seabed,juce::PathStrokeType(6));
    g.setColour(green.withAlpha(.83f));g.strokePath(seabed,juce::PathStrokeType(1.5f));
    for(int i=0;i<13;i++){
        float x=cx+std::sin(i*6.41f)*r*.81f,y=cy+std::cos(i*3.41f)*r*.81f;
        float alpha=.10f+.35f*(.5f+.5f*std::sin(angle*1.3f+i*2.4f));
        g.setColour(green.withAlpha(alpha));g.fillEllipse(x-2,y-2,4,4);
    }
    g.setColour(green);g.fillEllipse(cx-4,cy-4,8,8);
    g.setColour(green.withAlpha(.83f));
    g.setFont(juce::Font(juce::FontOptions(13.0f)));
    for(int i=0;i<=4;i++)
        g.drawText(juce::String(i*250)+" m",juce::Rectangle<int>(static_cast<int>(cx+r*.47f),
            static_cast<int>(cy-r*.55f+i*r*.28f),65,18),juce::Justification::centredLeft);
    for(int i=0;i<12;i++){
        float a=i*sonarium::tau/12.0f;
        float x=cx+std::sin(a)*(r+1),y=cy-std::cos(a)*(r+1);
        g.setColour(brass.brighter(.24f));g.fillEllipse(x-4,y-4,8,8);
    }
    g.setColour(aged.withAlpha(.75f));g.drawEllipse(cx-r+10,cy-r+10,2*r-20,2*r-20,2.5f);
}

SonariumTapeReel::SonariumTapeReel(){startTimerHz(24);setOpaque(false);}
void SonariumTapeReel::paint(juce::Graphics& g) {
    const float w=static_cast<float>(getWidth()),h=static_cast<float>(getHeight());
    if(w<10||h<10)return;
    g.setColour(deep.withAlpha(.84f));
    g.fillRoundedRectangle(0,0,w,h,8);
    g.setColour(brass);g.drawRoundedRectangle(1,1,w-2,h-2,8,2);
    for(int i=0;i<2;i++){
        float cx=w*(i==0?.28f:.72f),cy=h*.45f,r=std::min(h*.33f,w*.21f);
        g.setColour(juce::Colour::fromRGB(81,55,34));g.fillEllipse(cx-r-2,cy-r-2,r*2+4,r*2+4);
        g.setColour(brass.brighter(.4f));g.drawEllipse(cx-r-2,cy-r-2,r*2+4,r*2+4,2);
        g.setColour(juce::Colour::fromRGB(171,110,51));g.fillEllipse(cx-r*.84f,cy-r*.84f,r*1.68f,r*1.68f);
        g.setColour(deep);g.fillEllipse(cx-r*.29f,cy-r*.29f,r*.58f,r*.58f);
        for(int blade=0;blade<5;blade++){
            float a=rotation*(i==0?1.0f:-1.0f)+blade*sonarium::tau/5.0f;
            float rr=r*.64f;
            g.setColour(cream.withAlpha(.8f));
            g.fillEllipse(cx+std::cos(a)*rr-r*.12f,cy+std::sin(a)*rr-r*.12f,r*.24f,r*.24f);
        }
        g.setColour(ink);g.fillEllipse(cx-3,cy-3,6,6);
    }
    g.setColour(cream.withAlpha(.35f));g.drawLine(w*.30f,h*.8f,w*.7f,h*.8f,2);
}
SonariumMeter::SonariumMeter(SonariumAudioProcessor& p):processor(p){startTimerHz(28);}
void SonariumMeter::paint(juce::Graphics& g) {
    auto bounds=getLocalBounds().toFloat();
    g.setColour(deep);g.fillRoundedRectangle(bounds,12);
    for(int i=0;i<2;i++){
        auto r=bounds.withWidth(bounds.getWidth()*.5f-10).translated(i*(bounds.getWidth()*.5f),0).reduced(6);
        g.setGradientFill(juce::ColourGradient(cream,r.getTopLeft(),aged.darker(.21f),r.getBottomRight(),false));
        g.fillRoundedRectangle(r,7);
        g.setColour(brass);g.drawRoundedRectangle(r,7,2);
        auto cx=r.getCentreX(),cy=r.getBottom()-13;
        float radius=r.getWidth()*.38f;
        for(int t=0;t<=10;t++){
            float a=(-.91f+1.82f*t/10.0f);
            float x=cx+std::sin(a)*radius,y=cy-std::cos(a)*radius;
            g.setColour(t>7?juce::Colours::darkred:ink);g.drawLine(x,y,x+std::sin(a)*7,y-std::cos(a)*7,1);
        }
        float val=i==0?processor.meterL.load():processor.meterR.load();
        float position=sonarium::clamp(std::sqrt(val),0,1);
        float a=-.91f+1.82f*position;
        g.setColour(ink);g.drawLine(cx,cy,cx+std::sin(a)*radius*.95f,cy-std::cos(a)*radius*.95f,2.1f);
        g.fillEllipse(cx-3,cy-3,6,6);
        label(g,i==0?"L  VU":"R  VU",r.getX(),r.getY()+4,r.getWidth(),22,14,true);
    }
}

SonariumAudioProcessorEditor::SonariumAudioProcessorEditor(SonariumAudioProcessor& p):
    juce::AudioProcessorEditor(&p),processor(p),radar(p),meter(p),
    keyboardView(p.keyboard,juce::MidiKeyboardComponent::horizontalKeyboard) {
    setLookAndFeel(&look);
    setOpaque(true);
    // Construct every child before setting size: setSize synchronously calls resized().
    addAndMakeVisible(radar);
    addAndMakeVisible(meter);
    for(auto& reel:reels)addAndMakeVisible(reel);

    const struct {const char* id;const char* title;float x,y,size;} specs[]={
        {"sub","SUB",205,289,75},{"drive","DRIVE",288,289,75},{"mute","MUTE",371,289,75},
        {"electric","ELECTRIC",454,289,75},{"tone","TONE",537,289,75},
        {"static","STATIC",1112,288,73},{"drift","DRIFT",1203,288,73},{"dust","DUST",1294,288,73},
        {"padbloom","BLOOM",1385,288,73},{"texture","TEXTURE",1476,288,73},
        {"ping","PING",1106,493,65},{"depth","DEPTH",1186,493,65},{"echo","ECHO",1266,493,65},
        {"pluck","PLUCK",1346,493,65},{"sea","SEA",1426,493,65},{"signal","SIGNAL",1506,493,65},
        {"tapetime","TIME",543,660,70},{"tapefb","FEEDBACK",656,660,70},{"tapemix","MIX",769,660,70},
        {"room","ROOM",1050,658,69},{"bloom","BLOOM",1162,658,69},{"plate","PLATE",1274,658,69},
        {"glide","GLIDE",236,720,64},{"master","MASTER",1486,672,70}
    };
    for(const auto& a:specs)addDial(a.id,a.title,a.x,a.y,a.size);

    addButton(presetPrev,"<<",[this]{processor.loadPreset(processor.getPresetIndex()-1);});
    addButton(presetNext,">>",[this]{processor.loadPreset(processor.getPresetIndex()+1);});
    addButton(saveButton,"SAVE",[this] {
        presetChooser=std::make_unique<juce::FileChooser>("Save Sonarium preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),"*.sonarium");
        auto editorSafe=juce::Component::SafePointer<SonariumAudioProcessorEditor>(this);
        presetChooser->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles,
            [safe=editorSafe](const juce::FileChooser& chooser){
                if(safe==nullptr)return;
                auto file=chooser.getResult();
                if(file==juce::File{})return;
                if(!file.hasFileExtension("sonarium"))file=file.withFileExtension(".sonarium");
                juce::MemoryBlock data;safe->processor.getStateInformation(data);
                file.replaceWithData(data.getData(),data.getSize());
            });
    });
    addButton(loadButton,"LOAD",[this]{
        presetChooser=std::make_unique<juce::FileChooser>("Load Sonarium preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),"*.sonarium");
        auto editorSafe=juce::Component::SafePointer<SonariumAudioProcessorEditor>(this);
        presetChooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
            [safe=editorSafe](const juce::FileChooser& chooser){
                if(safe==nullptr)return;
                auto file=chooser.getResult();
                if(!file.existsAsFile())return;
                juce::MemoryBlock data;
                if(file.loadFileAsData(data))safe->processor.setStateInformation(data.getData(),static_cast<int>(data.getSize()));
            });
    });
    presetLabel.setText(processor.getPresetName(),juce::dontSendNotification);
    presetLabel.setColour(juce::Label::textColourId,cream);
    presetLabel.setJustificationType(juce::Justification::centred);
    presetLabel.setFont(juce::Font(juce::FontOptions("Georgia",25,juce::Font::italic)));
    addAndMakeVisible(presetLabel);
    for(int i=0;i<3;i++){
        auto mode=i;
        addButton(modeButtons[static_cast<size_t>(i)],juce::StringArray{"BASS","PAD","SONAR"}[i],[this,mode]{
            if(auto* param=processor.state.getParameter("mode"))param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(mode)));
        });
        addButton(tapeButtons[static_cast<size_t>(i)],juce::StringArray{"WARM","WORN","WARBLE"}[i],[this,mode]{
            if(auto* param=processor.state.getParameter("tape"))param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(mode)));
        });
    }
    polySlider=std::make_unique<juce::Slider>();
    polySlider->setSliderStyle(juce::Slider::IncDecButtons);
    polySlider->setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_AutoDirection);
    polySlider->setTextBoxStyle(juce::Slider::TextBoxLeft,false,42,26);
    polySlider->setColour(juce::Slider::textBoxTextColourId,ink);
    addAndMakeVisible(*polySlider);
    polyAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,"poly",*polySlider);
    polyLabel.setText("VOICES",juce::dontSendNotification);
    polyLabel.setColour(juce::Label::textColourId,ink);
    polyLabel.setJustificationType(juce::Justification::centred);
    polyLabel.setFont(juce::Font(juce::FontOptions("Georgia",19,juce::Font::bold)));
    addAndMakeVisible(polyLabel);
    keyboardView.setAvailableRange(24,96);
    keyboardView.setLowestVisibleKey(36);
    keyboardView.setKeyWidth(22);
    keyboardView.setColour(juce::MidiKeyboardComponent::whiteNoteColourId,juce::Colour::fromRGB(239,216,174));
    keyboardView.setColour(juce::MidiKeyboardComponent::blackNoteColourId,juce::Colour::fromRGB(47,35,26));
    keyboardView.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId,brass.darker(.4f));
    keyboardView.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,brass.withAlpha(.4f));
    keyboardView.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId,juce::Colours::orange.withAlpha(.5f));
    addAndMakeVisible(keyboardView);
    setResizable(true,true);
    setResizeLimits(1100,620,2400,1350);
    setSize(1600,900);
    startTimerHz(12);
}

SonariumAudioProcessorEditor::~SonariumAudioProcessorEditor(){
    stopTimer();
    polyAttach.reset();
    for(auto& d:dials)d.attach.reset();
    setLookAndFeel(nullptr);
}
void SonariumAudioProcessorEditor::addButton(juce::TextButton& b,const juce::String& text,std::function<void()> cb){
    b.setButtonText(text);b.setClickingTogglesState(false);b.onClick=std::move(cb);
    addAndMakeVisible(b);
}
void SonariumAudioProcessorEditor::addDial(const char* id,const char* title,float x,float y,float size){
    Dial dial;dial.id=id;dial.label=title;dial.x=x;dial.y=y;dial.diameter=size;
    dial.slider=std::make_unique<juce::Slider>();
    dial.slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dial.slider->setRotaryParameters(juce::MathConstants<float>::pi*1.24f,
        juce::MathConstants<float>::pi*2.76f,true);
    dial.slider->setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
    dial.slider->setPopupDisplayEnabled(true,true,this);
    dial.slider->setDoubleClickReturnValue(true,processor.state.getParameter(id)->convertFrom0to1(processor.state.getParameter(id)->getDefaultValue()));
    dial.slider->setTitle(title);dial.slider->setName(title);
    addAndMakeVisible(*dial.slider);
    dial.attach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,id,*dial.slider);
    dials.push_back(std::move(dial));
}
void SonariumAudioProcessorEditor::timerCallback(){
    if (!isShowing()) return;
    presetLabel.setText(processor.getPresetName(),juce::dontSendNotification);
    int mode=static_cast<int>(processor.state.getRawParameterValue("mode")->load());
    int tape=static_cast<int>(processor.state.getRawParameterValue("tape")->load());
    for(int i=0;i<3;i++){
        modeButtons[static_cast<size_t>(i)].setToggleState(i==mode,juce::dontSendNotification);
        tapeButtons[static_cast<size_t>(i)].setToggleState(i==tape,juce::dontSendNotification);
    }
}
void SonariumAudioProcessorEditor::resized(){
    if (!polySlider || dials.size()!=24) return; // safe during host resize or partial construction
    float sx=getWidth()/W,sy=getHeight()/H;
    auto put=[this,sx,sy](juce::Component& c,float x,float y,float w,float h){
        c.setBounds(juce::roundToInt(x*sx),juce::roundToInt(y*sy),juce::roundToInt(w*sx),juce::roundToInt(h*sy));
    };
    put(radar,593,160,455,455);
    put(meter,125,448,418,128);
    put(reels[0],340,624,115,87);
    put(reels[1],836,624,105,87);
    for(auto& dial:dials)put(*dial.slider,dial.x-dial.diameter*.5f,dial.y-dial.diameter*.5f,dial.diameter,dial.diameter);
    put(presetPrev,534,114,65,48);put(presetLabel,600,114,390,48);put(presetNext,989,114,61,48);put(saveButton,1060,114,76,48);put(loadButton,1142,114,76,48);
    for(int i=0;i<3;i++){
        put(modeButtons[static_cast<size_t>(i)],45,600+i*54.0f,185,43);
        put(tapeButtons[static_cast<size_t>(i)],375+i*175.0f,724,154,37);
    }
    put(polyLabel,37,795,120,34);put(*polySlider,156,797,129,31);
    put(keyboardView,302,788,1259,99);
}
void SonariumAudioProcessorEditor::paint(juce::Graphics& g){
    g.fillAll(deep);
    juce::Graphics::ScopedSaveState s(g);
    g.addTransform(juce::AffineTransform::scale(getWidth()/W,getHeight()/H));
    juce::ColourGradient paper(juce::Colour::fromRGB(225,185,123),0,0,
        juce::Colour::fromRGB(241,214,166),W*.62f,H*.70f,false);
    paper.addColour(.35,juce::Colour::fromRGB(235,205,158));
    g.setGradientFill(paper);g.fillRect(0.0f,0.0f,W,H);
    std::minstd_rand rng(248801);
    for(int i=0;i<3600;i++) {
        float x=static_cast<float>(rng()%1600),y=static_cast<float>(rng()%900);
        float a=static_cast<float>(rng()%12)/150.0f;
        g.setColour((rng()%2?ink:cream).withAlpha(a));
        g.fillRect(x,y,1.0f+static_cast<float>(rng()%3),1.0f);
    }
    mapMotif(g);lighthouse(g);ship(g);vacTube(g);
    g.setColour(deep);g.drawRoundedRectangle(juce::Rectangle<float>(6,6,1588,888),28,10);
    g.setColour(brass.brighter(.6f));g.drawRoundedRectangle(juce::Rectangle<float>(12,11,1576,878),24,3);
    for(float x:{18.0f,1582.0f})for(float y:{18.0f,882.0f}) {
        g.setColour(ink);g.fillEllipse(x-8,y-8,16,16);
        g.setColour(brass.brighter());g.drawEllipse(x-7,y-7,14,14,2);
        g.drawLine(x-4,y-4,x+4,y+4,2);
    }
    label(g,"SONARIUM",412,4,760,90,76,true,ink);
    label(g,"M A R I N E   T E S T   S Y N T H E S I Z E R",514,84,580,26,17,false);
    g.setColour(ink.withAlpha(.52f));g.drawLine(470,110,1102,110,1.3f);
    plate(g,juce::Rectangle<float>(515,110,639,57),13);
    g.setColour(deep);g.fillRoundedRectangle(juce::Rectangle<float>(600,116,390,45),8);
    section(g,"B A S S",juce::Rectangle<float>(122,216,460,164));
    section(g,"P A D",juce::Rectangle<float>(1070,216,502,164));
    section(g,"S O N A R",juce::Rectangle<float>(1070,421,502,158));
    plate(g,juce::Rectangle<float>(115,432,439,153),13);
    // Helm and vintage radio knobs
    g.setColour(ink.withAlpha(.54f));g.drawEllipse(12,548,246,246,3);
    g.setColour(brass);g.drawEllipse(24,560,218,218,9);
    g.setColour(ink);g.drawLine(125,572,125,769,4);g.drawLine(34,663,222,663,4);
    label(g,"SIGNAL SOURCE",43,580,170,29,20,true);
    plate(g,juce::Rectangle<float>(310,598,648,162),16);
    banner(g,juce::Rectangle<float>(488,588,285,44),"T A P E   E C H O");
    section(g,"R E V E R B",juce::Rectangle<float>(988,602,406,155));
    banner(g,juce::Rectangle<float>(1432,608,133,46),"OUTPUT");
    // faux tape reel etching
    for(float xx:{358.0f,875.0f}){
        g.setColour(ink.withAlpha(.15f));
        g.drawEllipse(xx-18,620,69,69,2);
        for(int a=0;a<3;a++){
            float angle=a*sonarium::tau/3;
            g.drawLine(xx+17,654,xx+17+std::cos(angle)*27,654+std::sin(angle)*27,3);
        }
    }
    plate(g,juce::Rectangle<float>(18,779,285,112),14);
    plate(g,juce::Rectangle<float>(297,774,1276,118),9);
    label(g,"41° 17' N  /  70° 56' W",1310,25,270,35,15);
    label(g,"PL. 07",180,30,110,30,17);
    label(g,"SONARIUM  •  THOMC  •  MMXXVI",1210,743,340,25,12);
    for(const auto& d:dials){
        float dy=d.diameter*.5f+12;
        label(g,d.label,d.x-67,d.y+dy,134,23,d.diameter<70?16:17,true);
    }
    label(g,"TAPE CHARACTER",465,699,445,26,16,true);
    label(g,"POLY",32,837,94,26,18,true);
}
