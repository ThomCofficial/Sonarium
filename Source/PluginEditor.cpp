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
void plate(juce::Graphics& g,juce::Rectangle<float> r,float corner=16.0f){
    // Deep walnut shadow, aged metal bevel and engraved edge.
    g.setColour(juce::Colour(0xff130d0b).withAlpha(.87f));
    g.fillRoundedRectangle(r.translated(4.0f,7.0f),corner);
    juce::ColourGradient fill(juce::Colour(0xffd5b47e),r.getX(),r.getY(),
                              juce::Colour(0xff705133),r.getRight(),r.getBottom(),false);
    fill.addColour(.18,juce::Colour(0xffbaa076));
    fill.addColour(.51,juce::Colour(0xff80613d));
    fill.addColour(.77,juce::Colour(0xffa78651));
    g.setGradientFill(fill);g.fillRoundedRectangle(r,corner);
    g.setColour(juce::Colour(0xff29180f));g.drawRoundedRectangle(r,corner,3.3f);
    g.setColour(juce::Colour(0xfff3d7a2).withAlpha(.55f));
    g.drawRoundedRectangle(r.reduced(4.1f),corner-2,1.3f);
    g.setColour(juce::Colour(0xff3c2919).withAlpha(.55f));
    g.drawRoundedRectangle(r.reduced(8.3f),corner-5,1.2f);
    for(const float x:{r.getX()+17,r.getRight()-17})
        for(const float y:{r.getY()+17,r.getBottom()-17}){
            g.setColour(juce::Colour(0xff281810));g.fillEllipse(x-6,y-6,12,12);
            g.setColour(juce::Colour(0xffecc48a));g.drawEllipse(x-5,y-5,10,10,1.5f);
            g.setColour(juce::Colour(0xff5b3821));g.drawLine(x-3,y+2,x+3,y-2,1.4f);
        }
}

void banner(juce::Graphics& g,juce::Rectangle<float> r,const juce::String& text){
    plate(g,r,8.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff523820),r.getX(),r.getY(),
        juce::Colour(0xff20140e),r.getRight(),r.getBottom(),false));
    g.fillRoundedRectangle(r.reduced(7.0f,6.0f),5.0f);
    g.setColour(juce::Colour(0xffefc685));g.drawRoundedRectangle(r.reduced(7.0f,6.0f),5.0f,1.0f);
    label(g,text,r.getX(),r.getY()+3,r.getWidth(),r.getHeight()-7,26,true,cream);
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
void section(juce::Graphics& g,const juce::String& title,juce::Rectangle<float> body){
    plate(g,body,17);
    auto tag=juce::Rectangle<float>(body.getCentreX()-body.getWidth()*.29f,body.getY()-49,
                                     body.getWidth()*.58f,49);
    banner(g,tag,title);
}


}

void SonariumLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&){
    if(w<16||h<16)return;
    const auto rect=juce::Rectangle<float>(float(x),float(y),float(w),float(h));
    const auto c=rect.getCentre();
    const float radius=std::min(rect.getWidth(),rect.getHeight())*.365f;
    const float angle=start+pos*(end-start);
    // Faceplate engraving remains stationary as only the pointer moves.
    g.setColour(juce::Colour(0xff2c1b12).withAlpha(.7f));
    g.fillEllipse(c.x-radius-7,c.y-radius-4,2*radius+14,2*radius+14);
    g.setColour(juce::Colour(0xfff3d09a));
    g.drawEllipse(c.x-radius-3,c.y-radius-3,radius*2+6,radius*2+6,1.5f);
    for(int i=0;i<13;i++){
        float a=start+(end-start)*float(i)/12.0f;
        float inner=radius+7,outer=inner+(i%3==0?8.5f:4.0f);
        g.setColour(juce::Colour(0xff382719));
        g.drawLine(c.x+std::sin(a)*inner,c.y-std::cos(a)*inner,
                   c.x+std::sin(a)*outer,c.y-std::cos(a)*outer,1.4f);
    }
    juce::ColourGradient rim(juce::Colour(0xffedcf8c),c.x-radius,c.y-radius,
                             juce::Colour(0xff56351d),c.x+radius,c.y+radius,false);
    rim.addColour(.21,juce::Colour(0xffa16c36));
    rim.addColour(.54,juce::Colour(0xffe7b86c));
    rim.addColour(.81,juce::Colour(0xff83542e));
    g.setGradientFill(rim);
    g.fillEllipse(c.x-radius,c.y-radius,2*radius,2*radius);
    g.setColour(juce::Colour(0xff21150e));
    g.drawEllipse(c.x-radius,c.y-radius,2*radius,2*radius,2.2f);
    // Machined knurl notches (static outer housing).
    for(int i=0;i<40;i++){
        const float a=float(i)*sonarium::tau/40.0f;
        const float rin=radius*.83f,rout=radius*.97f;
        g.setColour(i%2==0?juce::Colour(0xff492b15):juce::Colour(0xffe4af63));
        g.drawLine(c.x+std::sin(a)*rin,c.y-std::cos(a)*rin,
                   c.x+std::sin(a)*rout,c.y-std::cos(a)*rout,.95f);
    }
    const float inner=radius*.77f;
    juce::ColourGradient cap(juce::Colour(0xfff8d499),c.x-inner,c.y-inner,
                             juce::Colour(0xff5f3e23),c.x+inner,c.y+inner,false);
    cap.addColour(.45,juce::Colour(0xffaa733e));
    g.setGradientFill(cap);g.fillEllipse(c.x-inner,c.y-inner,2*inner,2*inner);
    g.setColour(juce::Colour(0xffefce9b).withAlpha(.7f));
    g.drawEllipse(c.x-inner+2,c.y-inner+2,2*inner-4,2*inner-4,1.2f);
    // Glossy recessed centre and fully parameter-driven indicator.
    g.setColour(juce::Colour(0xff2b190f).withAlpha(.4f));
    g.fillEllipse(c.x-5,c.y-5,10,10);
    const float pLen=radius*.75f;
    g.setColour(juce::Colour(0xff29180f));
    g.drawLine(c.x+std::sin(angle)*radius*.18f,c.y-std::cos(angle)*radius*.18f,
               c.x+std::sin(angle)*pLen,c.y-std::cos(angle)*pLen,5.5f);
    g.setColour(juce::Colour(0xffffd999));
    g.drawLine(c.x+std::sin(angle)*radius*.28f,c.y-std::cos(angle)*radius*.28f,
               c.x+std::sin(angle)*pLen,c.y-std::cos(angle)*pLen,2.4f);
    const auto led=juce::Colour(0xffffad42).withAlpha(.32f+.58f*pos);
    g.setColour(led);g.fillEllipse(c.x-3,c.y-radius-7,6,6);
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

SonariumRadar::SonariumRadar(SonariumAudioProcessor& p):processor(p){setOpaque(false);setTooltip("SONAR — drag in the display to control depth and transmitter pulse");startTimerHz(30);}
void SonariumRadar::mouseDown(const juce::MouseEvent& e){interact(e.position);}
void SonariumRadar::mouseDrag(const juce::MouseEvent& e){interact(e.position);}
void SonariumRadar::interact(juce::Point<float> pt){
    const auto d=pt-getLocalBounds().toFloat().getCentre();
    const float rr=std::min(getWidth(),getHeight())*.47f;
    if(rr<=0)return;
    float distance=sonarium::clamp(std::sqrt(d.x*d.x+d.y*d.y)/rr,0,1);
    float a=std::atan2(d.x,-d.y);if(a<0)a+=sonarium::tau;
    if(auto* param=processor.state.getParameter("depth"))param->setValueNotifyingHost(param->convertTo0to1(distance));
    if(auto* param=processor.state.getParameter("ping"))param->setValueNotifyingHost(param->convertTo0to1(a/sonarium::tau));
}

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
    const float output=processor.signalActivity.load(std::memory_order_relaxed);
    const int playing=processor.voiceCount.load(std::memory_order_relaxed);
    const int note=processor.playingNote.load(std::memory_order_relaxed);
    const int mode=juce::jlimit(0,2,int(processor.state.getRawParameterValue("mode")->load()));
    juce::Path seabed;
    for(int x=0;x<=static_cast<int>(w);x+=3){
        float xx=static_cast<float>(x);
        float yy=cy+r*.49f+(12+output*30)*std::sin(xx*(mode==0?.052f:mode==1?.023f:.082f)+angle*.8f)+14*std::sin(xx*.063f+angle*.27f)+contour*18*std::sin(xx*.095f);
        if(x==0)seabed.startNewSubPath(xx,yy);else seabed.lineTo(xx,yy);
    }
    g.setColour(green.withAlpha(.42f));g.strokePath(seabed,juce::PathStrokeType(6));
    g.setColour(green.withAlpha(.83f));g.strokePath(seabed,juce::PathStrokeType(1.5f));
    for(int i=0;i<13;i++){
        float x=cx+std::sin(i*6.41f)*r*.81f,y=cy+std::cos(i*3.41f)*r*.81f;
        float alpha=.10f+.35f*(.5f+.5f*std::sin(angle*1.3f+i*2.4f));
        g.setColour(green.withAlpha(alpha));g.fillEllipse(x-2,y-2,4,4);
    }
    if(playing>0&&note>=0){
        const float d=sonarium::clamp((note-24)/72.0f,.1f,.9f)*r;
        const float ax=angle*.33f+note*.11f;
        const float px=cx+std::sin(ax)*d,py=cy-std::cos(ax)*d;
        g.setGradientFill(juce::ColourGradient(green.withAlpha(.4f*output+.12f),px,py,green.withAlpha(0),px+24,py+24,true));
        g.fillEllipse(px-24,py-24,48,48);
        g.setColour(green);g.fillEllipse(px-3,py-3,6,6);
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
    g.setColour(green.withAlpha(.8f));
    g.drawText((mode==0?"LOW FREQ / SUB":mode==1?"POLYVOLT / DRIFT":"TX / RX SONAR")+juce::String("   ")+juce::String(int(output*100))+"%",juce::Rectangle<int>(int(cx-r*.6f),int(cy+r*.7f),int(r*1.2f),18),juce::Justification::centred);
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
        {"glide","GLIDE",268,703,64},{"master","MASTER",1486,672,70}
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
    setResizeLimits(1000,563,1600,900);
    if(auto* c=getConstrainer())c->setFixedAspectRatio(1600.0/900.0);
    setSize(1280,720);
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
    illuminationPhase+=.14f;
    if(illuminationPhase>sonarium::tau)illuminationPhase-=sonarium::tau;
    repaint(18,24,115,185); // dynamic glass oscillator lamp, not a baked status light
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
    put(radar,609,174,418,418);
    put(meter,125,448,418,128);
    put(reels[0],340,624,115,87);
    put(reels[1],836,624,105,87);
    for(auto& dial:dials)put(*dial.slider,dial.x-dial.diameter*.5f,dial.y-dial.diameter*.5f,dial.diameter,dial.diameter);
    put(presetPrev,534,114,65,48);put(presetLabel,600,114,390,48);put(presetNext,989,114,61,48);put(saveButton,1060,114,76,48);put(loadButton,1142,114,76,48);
    for(int i=0;i<3;i++){
        put(modeButtons[static_cast<size_t>(i)],45,600+i*54.0f,185,43);
        put(tapeButtons[static_cast<size_t>(i)],375+i*175.0f,735,154,35);
    }
    put(polyLabel,37,795,120,34);put(*polySlider,156,797,129,31);
    put(keyboardView,302,788,1259,99);
}
void SonariumAudioProcessorEditor::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(0xff100d0b));
    juce::Graphics::ScopedSaveState save(g);
    g.addTransform(juce::AffineTransform::scale(getWidth()/W,getHeight()/H));
    // Cabinet and physically layered brass platework. The background is
    // not a static image: knobs, meter, radar, buttons and reels remain
    // independently composited native JUCE components above this faceplate.
    juce::ColourGradient walnut(juce::Colour(0xff3e281b),0,0,
                                 juce::Colour(0xff120d0b),W,H,false);
    walnut.addColour(.35,juce::Colour(0xff56331d));
    walnut.addColour(.65,juce::Colour(0xff24160f));
    g.setGradientFill(walnut);g.fillRoundedRectangle(3,4,1594,892,24);
    g.setColour(juce::Colour(0xff080605));g.drawRoundedRectangle(5,5,1590,886,23,8);
    g.setColour(juce::Colour(0xffc99b5a));g.drawRoundedRectangle(12,12,1576,876,19,3);
    // Fine engraved crosshatching and age, deterministic every repaint.
    std::minstd_rand rng(1827);
    for(int i=0;i<2400;i++){
        float x=float(rng()%1590)+5,y=float(rng()%885)+5;
        const float alpha=.014f+.015f*float(rng()%10)/10;
        g.setColour((i%3?juce::Colour(0xffedc98e):juce::Colour(0xff160e09)).withAlpha(alpha));
        g.fillRect(x,y,1.0f+float(rng()%6),.6f);
    }
    for(float x:{25.0f,1574.0f})
        for(float y:{26.0f,872.0f}){
            g.setColour(juce::Colour(0xff21150e));g.fillEllipse(x-12,y-12,24,24);
            g.setColour(juce::Colour(0xffc68e48));g.drawEllipse(x-9,y-9,18,18,2.5f);
            g.drawLine(x-5,y+3,x+5,y-3,2);
        }
    // Signal-driven tube: illumination fluctuates with actual sound, never
    // merely a decorative baked-in photograph.
    vacTube(g);
    const float level=processor.signalActivity.load(std::memory_order_relaxed);
    const float flicker=.28f+.36f*(.5f+.5f*std::sin(illuminationPhase))+.32f*level;
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffffa62a).withAlpha(flicker*.28f),59,109,
                               juce::Colour(0xffffa62a).withAlpha(0.0f),136,109,true));
    g.fillEllipse(-14,36,146,146);
    // Deep patinated engraving in the headboard.
    g.setColour(juce::Colour(0xffbf945c).withAlpha(.4f));
    g.drawLine(395,100,1180,100,1.2f);
    g.drawLine(430,104,1140,104,.5f);
    label(g,"SONARIUM",420,9,766,91,71,true,juce::Colour(0xffecc78b));
    label(g,"M A R I N E    T E S T    S Y N T H E S I Z E R",
          472,94,650,25,17,true,juce::Colour(0xffe8bd80));
    label(g,"THOMC  /  INSTRUMENTS",1295,39,241,28,15,false,juce::Colour(0xffcdab79));
    label(g,"PLATE  07   •   MMXXVI",60,34,265,27,15,false,juce::Colour(0xffd0a976));
    // Architectural panels and labels.
    section(g,"B A S S",juce::Rectangle<float>(117,224,463,186));
    section(g,"P A D",juce::Rectangle<float>(1067,224,504,186));
    section(g,"S O N A R",juce::Rectangle<float>(1067,428,504,180));
    plate(g,juce::Rectangle<float>(111,431,445,163),17);
    label(g,"DUAL SIGNAL METERS",164,426,333,22,16,true,juce::Colour(0xffe8c18b));
    plate(g,juce::Rectangle<float>(310,609,650,167),17);
    banner(g,juce::Rectangle<float>(498,572,275,48),"T A P E    E C H O");
    section(g,"R E V E R B",juce::Rectangle<float>(988,619,415,155));
    banner(g,juce::Rectangle<float>(1430,580,139,46),"MASTER");
    plate(g,juce::Rectangle<float>(22,584,252,182),19);
    label(g,"SIGNAL  /  SOURCE",34,565,213,28,17,true,juce::Colour(0xffe8c18b));
    plate(g,juce::Rectangle<float>(14,780,280,112),11);
    plate(g,juce::Rectangle<float>(297,780,1270,112),10);
    // Floating black glass preset display below title.
    plate(g,juce::Rectangle<float>(513,116,714,67),11);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff16120e),600,126,
                                            juce::Colour(0xff42301e),1010,171,false));
    g.fillRoundedRectangle(598,122,395,56,9);
    g.setColour(juce::Colour(0xffd2a873));g.drawRoundedRectangle(598,122,395,56,9,1.1f);
    // Draw fine engine strip around radar porthole (radar itself is a separate
    // animated instrument with real input telemetry).
    g.setColour(juce::Colour(0xff8f6237).withAlpha(.37f));
    for(int a=0;a<360;a+=12){
        float theta=a*sonarium::pi/180.0f;
        g.drawLine(820+std::cos(theta)*223,389+std::sin(theta)*223,
                   820+std::cos(theta)*229,389+std::sin(theta)*229,1.2f);
    }
    // Small engraved maritime compass to anchor the empty upper corner.
    const float mx=1425,my=149;
    g.setColour(juce::Colour(0xffc9a36f).withAlpha(.68f));
    g.drawEllipse(mx-28,my-28,56,56,1.5f);
    g.drawLine(mx-35,my,mx+35,my,.8f);
    g.drawLine(mx,my-35,mx,my+35,.8f);
    juce::Path needle;needle.addTriangle(mx,my-27,mx-7,my+12,mx+7,my+12);
    g.fillPath(needle);
    label(g,"N",mx-14,my-51,28,23,17,true,cream);
    // Matching lighthouse silhouette on the other side.
    g.setColour(juce::Colour(0xffad8656).withAlpha(.65f));
    juce::Path tower;tower.startNewSubPath(99,164);tower.lineTo(108,92);
    tower.lineTo(128,92);tower.lineTo(138,164);tower.closeSubPath();g.fillPath(tower);
    g.fillRect(103.0f,89.0f,29.0f,7.0f);
    g.drawLine(101,84,118,63,2);g.drawLine(118,63,133,84,2);
    g.setColour(juce::Colour(0xffffb84a).withAlpha(.2f));
    juce::Path beam;beam.addTriangle(118,97,165,57,273,79);g.fillPath(beam);
    // Individual brass calibration legends are in precise positions below
    // each rotary; never print over the tape switches or keyboard.
    for(const auto& dial:dials){
        const float bottom=dial.y+dial.diameter*.5f+11;
        g.setColour(juce::Colour(0xff452c1b));
        g.drawLine(dial.x-18,bottom-2,dial.x+18,bottom-2,.8f);
        label(g,dial.label,dial.x-64,bottom,128,24,15.5f,true,ink);
    }
    label(g,"RX  •  TX",706,612,224,20,13,true,juce::Colour(0xffeac694));
    label(g,"ROOM  •  BLOOM  •  PLATE",1000,747,385,15,11,false,juce::Colour(0xff4e331f));
    label(g,"TAPE SPEED / COLOUR",523,744,223,23,12,false,juce::Colour(0xffdac298));
    label(g,"SONARIUM   /   THOMC   /   2026",1408,756,179,17,10,false,juce::Colour(0xffd1a46e));
    label(g,"VOICES",31,810,118,28,20,true,ink);
    label(g,"MIDI POLYPHONY",37,861,126,20,12,false,ink);
}


}
