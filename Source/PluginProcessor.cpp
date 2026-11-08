#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <limits>

namespace {
using APVTS=juce::AudioProcessorValueTreeState;
juce::StringArray choices(const char* a,const char* b,const char* c) {return {a,b,c};}
struct ParamDef {const char* id; const char* name;float value;};
constexpr ParamDef controls[]={
 {"sub","Sub",.65f},{"drive","Drive",.25f},{"mute","Mute",.45f},{"electric","Electric",.28f},{"tone","Tone",.55f},
 {"static","Static",.35f},{"drift","Drift",.40f},{"dust","Dust",.30f},{"padbloom","Pad Bloom",.58f},{"texture","Texture",.45f},
 {"ping","Ping",.83f},{"depth","Depth",.70f},{"echo","Echo",.65f},{"pluck","Pluck",.75f},{"sea","Sea",.72f},{"signal","Signal",.60f},
 {"tapetime","Tape Time",.53f},{"tapefb","Tape Feedback",.50f},{"tapemix","Tape Mix",.40f},
 {"room","Room Reverb",.10f},{"bloom","Bloom Reverb",.52f},{"plate","Plate Reverb",.24f},
 {"glide","Glide",.0f},{"master","Master",.77f}};
struct Preset {
    const char* name;int mode,tape;std::array<float,24> controls;
};
const std::array<Preset,9>& presets(){
    static const std::array<Preset,9> p{{
        {"Abyssal Transmission",2,1,{{.65f,.25f,.45f,.28f,.55f,.35f,.4f,.3f,.58f,.45f,.83f,.7f,.65f,.75f,.72f,.6f,.53f,.5f,.4f,.10f,.52f,.24f,.0f,.77f}}},
        {"Harbour Sub",0,0,{{.96f,.10f,.35f,.02f,.18f,.2f,.12f,.1f,.4f,.5f,.6f,.4f,.4f,.5f,.4f,.5f,.3f,.2f,.04f,.04f,.05f,.02f,.02f,.75f}}},
        {"The Rusted Octave",0,2,{{.40f,.91f,.72f,.83f,.33f,.15f,.2f,.4f,.4f,.6f,.5f,.5f,.3f,.7f,.3f,.5f,.2f,.35f,.17f,.1f,.12f,.12f,.08f,.65f}}},
        {"Electric Mooring",0,1,{{.30f,.30f,.73f,.95f,.63f,.0f,.0f,.15f,.15f,.3f,.3f,.3f,.3f,.3f,.3f,.5f,.24f,.15f,.08f,.24f,.13f,.15f,.05f,.79f}}},
        {"Numbers Station",1,2,{{.4f,.2f,.3f,.3f,.5f,.93f,.77f,.66f,.82f,.67f,.6f,.5f,.5f,.5f,.5f,.5f,.45f,.42f,.35f,.12f,.71f,.26f,.17f,.67f}}},
        {"Saltwater Choir",1,1,{{.4f,.2f,.3f,.3f,.5f,.28f,.7f,.4f,.93f,.42f,.6f,.5f,.5f,.5f,.5f,.5f,.66f,.58f,.53f,.15f,.87f,.31f,.25f,.8f}}},
        {"Broken Receiver",1,2,{{.4f,.2f,.3f,.3f,.5f,.85f,.94f,.81f,.54f,.77f,.6f,.5f,.5f,.5f,.5f,.5f,.33f,.67f,.48f,.09f,.62f,.25f,.0f,.7f}}},
        {"Glass Bathymetry",2,0,{{.5f,.2f,.4f,.2f,.5f,.2f,.3f,.2f,.5f,.5f,.99f,.8f,.79f,.22f,.83f,.82f,.47f,.56f,.43f,.03f,.7f,.28f,.0f,.78f}}},
        {"Guitar from the Deep",2,1,{{.5f,.2f,.4f,.2f,.5f,.2f,.3f,.2f,.5f,.5f,.38f,.58f,.4f,.99f,.78f,.72f,.4f,.42f,.32f,.18f,.73f,.34f,.02f,.77f}}}
    }};
    return p;
}
}

SonariumAudioProcessor::APVTS::ParameterLayout SonariumAudioProcessor::createLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> ps;
    ps.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"mode",1},"Sound Engine",choices("Bass","Pad","Sonar"),2));
    ps.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"tape",1},"Tape Character",choices("Warm","Worn","Warble"),1));
    ps.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"poly",1},"Polyphony",1,16,16));
    for(const auto& d:controls)
        ps.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{d.id,1},d.name,juce::NormalisableRange<float>{0.0f,1.0f},d.value));
    return {ps.begin(),ps.end()};
}

SonariumAudioProcessor::SonariumAudioProcessor():
    juce::AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),
    state(*this,nullptr,"SONARIUM",createLayout()) {}

bool SonariumAudioProcessor::isBusesLayoutSupported(const BusesLayout& layout) const {
    return layout.getMainOutputChannelSet()==juce::AudioChannelSet::stereo()
        && layout.getMainInputChannelSet().isDisabled();
}

void SonariumAudioProcessor::prepareToPlay(double sr,int block) {
    currentSampleRate=sr;
    randomState=0x37aacc11u;voiceClock=0;pedal=false;keysDown.fill(false);
    pitchBend=modWheel=0.0f;lastNoteHz=0.0;wetSmoothed.fill(0.0f);
    for(auto& v:voices)v.active=false;
    tape.prepare(sr);
    for(auto& reverb:reverbs) reverb.setSampleRate(sr);
    reverbScratchSize=juce::jlimit(64,8192,std::max(64,block));
    for(auto& b:reverbBuffers)b.setSize(2,reverbScratchSize,false,true,true);
    parameterSmoothing=1.0f-std::exp(-1.0f/(static_cast<float>(sr)*.018f));
    interpolatedParams=snapshot();
    masterSmooth=state.getRawParameterValue("master")->load();
    lLevel=rLevel=0;meterL.store(0);meterR.store(0);
}
void SonariumAudioProcessor::releaseResources() {}
sonarium::Params SonariumAudioProcessor::snapshot() const noexcept {
    sonarium::Params p;
    const auto get=[this](const char* id){return state.getRawParameterValue(id)->load();};
    p.mode=static_cast<int>(get("mode"));p.tapeType=static_cast<int>(get("tape"));
    p.poly=std::max(1,static_cast<int>(get("poly")));
    p.sub=get("sub");p.drive=get("drive");p.mute=get("mute");p.electric=get("electric");p.tone=get("tone");
    p.stat=get("static");p.drift=get("drift");p.dust=get("dust");p.padBloom=get("padbloom");p.texture=get("texture");
    p.ping=get("ping");p.depth=get("depth");p.echo=get("echo");p.pluck=get("pluck");p.sea=get("sea");p.signal=get("signal");
    p.tapeTime=get("tapetime");p.tapeFeedback=get("tapefb");p.tapeMix=get("tapemix");
    p.room=get("room");p.bloom=get("bloom");p.plate=get("plate");p.glide=get("glide");p.master=get("master");
    return p;
}
void SonariumAudioProcessor::noteOn(int note,float velocity,int engine,int poly) {
    if(note<0||note>127)return;
    bool legato=false;
    for(const auto& v:voices)if(v.active&&v.held)legato=true;
    keysDown[static_cast<size_t>(note)]=true;
    sonarium::Voice* pick=nullptr;
    int running=0;
    for(auto& v:voices)if(v.active)++running;
    if(running<poly)for(auto& v:voices)if(!v.active) {pick=&v;break;}
    if(!pick) {
        for(auto& v:voices)
            if(!pick||(!v.held&&pick->held)||(v.held==pick->held&&v.stamp<pick->stamp))pick=&v;
    }
    const double hz=440.0*std::pow(2.0,(note-69)/12.0);
    const double previous=(poly==1&&legato&&lastNoteHz>0.0)?lastNoteHz:hz;
    if(pick)pick->start(note,velocity,engine,currentSampleRate,randomState,++voiceClock,previous);
    lastNoteHz=hz;
}
void SonariumAudioProcessor::noteOff(int note) {
    if(note<0||note>127)return;
    keysDown[static_cast<size_t>(note)]=false;
    if(!pedal)for(auto& v:voices)if(v.active&&v.note==note)v.release();
}
void SonariumAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    const int n=buffer.getNumSamples();
    if(n<=0)return;
    const auto target=snapshot();
    auto p=interpolatedParams;
    p.mode=target.mode;p.poly=target.poly;p.tapeType=target.tapeType;
    p.pitch=pitchBend;p.mod=modWheel;
    keyboard.processNextMidiBuffer(midi,0,n,true);
    juce::MidiBuffer::Iterator iterator(midi);
    juce::MidiMessage message;int eventSample=0;
    bool more=iterator.getNextEvent(message,eventSample);
    float* l=buffer.getWritePointer(0);
    float* r=buffer.getWritePointer(1);
    for(int i=0;i<n;i++) {
        // 18 ms smoothing for host-driven parameter steps; no allocation or locks.
        #define RAMP(x) p.x += (target.x-p.x)*parameterSmoothing
        RAMP(sub);RAMP(drive);RAMP(mute);RAMP(electric);RAMP(tone);
        RAMP(stat);RAMP(drift);RAMP(dust);RAMP(padBloom);RAMP(texture);
        RAMP(ping);RAMP(depth);RAMP(echo);RAMP(pluck);RAMP(sea);RAMP(signal);
        RAMP(tapeTime);RAMP(tapeFeedback);RAMP(tapeMix);
        RAMP(room);RAMP(bloom);RAMP(plate);RAMP(glide);RAMP(master);
        #undef RAMP
        while(more&&eventSample<=i) {
            if(message.isNoteOn()) noteOn(message.getNoteNumber(),message.getFloatVelocity(),p.mode,p.poly);
            else if(message.isNoteOff()) noteOff(message.getNoteNumber());
            else if(message.isAllNotesOff()||message.isAllSoundOff()) {
                for(auto& v:voices)v.release();
                keysDown.fill(false);pedal=false;
            } else if(message.isSustainPedalOn())pedal=true;
            else if(message.isSustainPedalOff()) {
                pedal=false;
                for(auto& v:voices)if(v.active&&!keysDown[static_cast<size_t>(v.note)])v.release();
            } else if(message.isPitchWheel())p.pitch=pitchBend=(static_cast<float>(message.getPitchWheelValue())-8192.0f)/8192.0f;
            else if(message.isController()&&message.getControllerNumber()==1)p.mod=modWheel=message.getControllerValue()/127.0f;
            more=iterator.getNextEvent(message,eventSample);
        }
        sonarium::Stereo sample{};
        for(auto& v:voices) {
            auto o=v.render(p,randomState);
            sample.l+=o.l;sample.r+=o.r;
        }
        sample.l=sonarium::soft(sample.l*.52f);
        sample.r=sonarium::soft(sample.r*.52f);
        auto echo=tape.process(sample,p);
        masterSmooth+=(p.master-masterSmooth)*.0018f;
        l[i]=echo.l*masterSmooth;
        r[i]=echo.r*masterSmooth;
    }
    // Bounded reverb scratch. Cubase may send blocks larger than requested.
    // Never resize/allocate from processBlock().
    const std::array<float,3> amounts{p.room,p.bloom,p.plate};
    for(int k=0;k<3;k++) {
        auto& rb=reverbBuffers[static_cast<size_t>(k)];
        juce::Reverb::Parameters rp;
        rp.roomSize=k==0?.25f:k==1?.97f:.64f;
        rp.damping=k==0?.60f:k==1?.15f:.30f;
        rp.wetLevel=1.0f;rp.dryLevel=0.0f;
        rp.width=k==0?.65f:1.0f;
        rp.freezeMode=0.0f;
        reverbs[static_cast<size_t>(k)].setParameters(rp);
        const float targetAmount=amounts[static_cast<size_t>(k)]*(k==1?.40f:.28f);
        auto& amount=wetSmoothed[static_cast<size_t>(k)];
        for(int offset=0;offset<n;offset+=reverbScratchSize){
            const int chunk=std::min(n-offset,reverbScratchSize);
            rb.copyFrom(0,0,buffer,0,offset,chunk);
            rb.copyFrom(1,0,buffer,1,offset,chunk);
            reverbs[static_cast<size_t>(k)].processStereo(rb.getWritePointer(0),rb.getWritePointer(1),chunk);
            for(int j=0;j<chunk;j++){
                amount+=(targetAmount-amount)*.00075f;
                l[offset+j]+=rb.getSample(0,j)*amount;
                r[offset+j]+=rb.getSample(1,j)*amount;
            }
        }
    }
    for(int i=0;i<n;i++){
        l[i]=sonarium::soft(l[i]);r[i]=sonarium::soft(r[i]);
        lLevel=std::max(std::abs(l[i]),lLevel*.9998f);
        rLevel=std::max(std::abs(r[i]),rLevel*.9998f);
    }
    interpolatedParams=p;
    meterL.store(sonarium::clamp(lLevel,0,1),std::memory_order_relaxed);
    meterR.store(sonarium::clamp(rLevel,0,1),std::memory_order_relaxed);
}

juce::String SonariumAudioProcessor::getPresetName() const {
    int index=presetIndex.load();
    return presets()[static_cast<size_t>(juce::jlimit(0,presetCount-1,index))].name;
}
void SonariumAudioProcessor::loadPreset(int index){
    index=(index+presetCount)%presetCount;
    const auto& preset=presets()[static_cast<size_t>(index)];
    const auto set=[this](const char* id,float val){
        if(auto* parameter=state.getParameter(id))
            parameter->setValueNotifyingHost(parameter->convertTo0to1(val));
    };
    set("mode",static_cast<float>(preset.mode));set("tape",static_cast<float>(preset.tape));
    for(size_t k=0;k<std::size(controls);k++)set(controls[k].id,preset.controls[k]);
    presetIndex.store(index);
}
void SonariumAudioProcessor::getStateInformation(juce::MemoryBlock& destination){
    auto tree=state.copyState();
    tree.setProperty("presetIndex",presetIndex.load(),nullptr);
    auto xml=tree.createXml();
    copyXmlToBinary(*xml,destination);
}
void SonariumAudioProcessor::setStateInformation(const void* data,int size){
    if(auto xml=getXmlFromBinary(data,size))
        if(xml->hasTagName(state.state.getType())){
            auto tree=juce::ValueTree::fromXml(*xml);
            presetIndex.store(juce::jlimit(0,presetCount-1,static_cast<int>(tree.getProperty("presetIndex",0))));
            state.replaceState(tree);
        }
}
juce::AudioProcessorEditor* SonariumAudioProcessor::createEditor(){
    return new SonariumAudioProcessorEditor(*this);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){
    return new SonariumAudioProcessor();
}
