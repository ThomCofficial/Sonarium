#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace sonarium {
constexpr float pi = 3.14159265358979323846f;
constexpr float tau = 6.2831853071795864769f;
inline float clamp(float a,float lo,float hi) noexcept { return std::max(lo,std::min(a,hi)); }
inline float soft(float x) noexcept { return x/(1.0f+std::abs(x)*0.42f); }
inline float noise(uint32_t& s) noexcept { s^=s<<13; s^=s>>17; s^=s<<5; return (static_cast<float>(s&0x00ffffffu)*(2.0f/16777216.0f)-1.0f); }
inline float blep(float t,float dt) noexcept {
    if(t<dt) { t/=dt; return t+t-t*t-1.0f; }
    if(t>1.0f-dt) { t=(t-1.0f)/dt; return t*t+t+t+1.0f; }
    return 0.0f;
}
inline float lpAlpha(float hz,float sr) noexcept { return 1.0f-std::exp(-tau*clamp(hz,10.0f,0.43f*sr)/sr); }
inline float tick(double& p,double hz,double sr) noexcept { p+=hz/sr; if(p>=1.0) p-=std::floor(p); return static_cast<float>(p); }
struct Stereo {float l=0,r=0;};
struct Params {
    int mode=0,poly=16,tapeType=0;
    float sub=.65f,drive=.25f,mute=.45f,electric=.28f,tone=.55f;
    float stat=.35f,drift=.4f,dust=.3f,padBloom=.58f,texture=.45f;
    float ping=.6f,depth=.45f,echo=.4f,pluck=.65f,sea=.48f,signal=.6f;
    float tapeTime=.4f,tapeFeedback=.35f,tapeMix=.25f;
    float room=.12f,bloom=.17f,plate=.12f,glide=0.0f,master=.75f,mod=0.0f,pitch=0.0f;
};

class Voice {
public:
    bool active=false,held=false,sustained=false;
    int note=-1,engine=0;
    uint64_t stamp=0;
    void start(int midi,float vel,int sound,double sampleRate,uint32_t& seed,uint64_t tickCount) noexcept {
        active=true;held=true;sustained=false;note=midi;engine=sound;stamp=tickCount;
        velocity=clamp(vel,0.0f,1.0f);sr=sampleRate;hz=440.0*std::pow(2.0,(midi-69)/12.0);
        glideHz=hz;env=0;age=0;phaseA=phaseB=phaseC=phaseD=0;low=0;worn=0;lastString=0;
        stringSize=std::max(8,std::min(static_cast<int>(stringData.size())-2,static_cast<int>(sr/hz)));
        for(int i=0;i<stringSize;i++) stringData[static_cast<size_t>(i)]=noise(seed)*.7f;
        stringAt=0;
    }
    void release() noexcept { held=false; sustained=false; }
    Stereo render(const Params& p,uint32_t& seed) noexcept {
        if(!active) return {};
        float attack=engine==1 ? .16f+1.15f*p.padBloom : (engine==2 ? .0045f : .003f);
        float releaseTime=engine==1 ? 1.1f+5.0f*p.padBloom : (engine==2 ? 0.4f+3.8f*p.sea : .07f+.75f*p.mute);
        float rate=1.0f-std::exp(-1.0f/(static_cast<float>(sr)*(held?attack:releaseTime)));
        env+=(held?1.0f-env:-env)*rate;
        if(!held && env<.00008f) { active=false;return {}; }
        ++age;
        float lifetime=static_cast<float>(age/sr);
        double target=hz*std::pow(2.0,static_cast<double>(p.pitch)*2.0/12.0);
        double glideSec=.0005+static_cast<double>(p.glide)*.8;
        glideHz+=(target-glideHz)*(1.0-std::exp(-1.0/(sr*glideSec)));
        double f=std::min(glideHz,0.43*sr), dt=f/sr;
        float a=tick(phaseA,f,sr), b=tick(phaseB,f*(1.003+p.drift*.012),sr);
        float c=tick(phaseC,f*(.994-p.drift*.011),sr);
        float lfo=tick(phaseD,.11+.47*p.drift+.30*p.sea,sr);
        const float rnd=noise(seed);
        const float str=stringData[static_cast<size_t>(stringAt)];
        int prev=stringAt==0?stringSize-1:stringAt-1;
        float next=(str+stringData[static_cast<size_t>(prev)])*.5f;
        next*=.992f+(.006f*p.pluck);
        stringData[static_cast<size_t>(stringAt)]=next;
        if(++stringAt>=stringSize)stringAt=0;
        const float stringSample=(str*.82f+lastString*.18f);
        lastString=str;
        float x=0,side=0;
        if(engine==0) {
            float sine=std::sin(tau*a);
            float saw=2.0f*a-1.0f-blep(a,static_cast<float>(dt));
            float harmonics=soft((.25f+.8f*p.drive)*(saw*.80f+std::sin(tau*b*2.0f)*.16f)*(1.0f+9.0f*p.drive));
            float electric=stringSample*(.35f+.65f*std::exp(-lifetime*(1.4f+9.0f*p.mute)));
            x=(sine*(.3f+.70f*p.sub)+harmonics*(.12f+.65f*p.drive)+electric*p.electric)*.72f;
            float cutoff=75.0f+std::pow(p.tone,2.0f)*9000.0f;
            cutoff*=1.0f+.95f*std::exp(-lifetime*(2.0f+7.0f*p.mute));
            low+=lpAlpha(cutoff,static_cast<float>(sr))*(x-low);
            x=low*(1.0f-.32f*p.mute);side=0.012f*electric;
        } else if(engine==1) {
            float saw1=2.0f*a-1.0f-blep(a,static_cast<float>(dt));
            float saw2=2.0f*b-1.0f-blep(b,static_cast<float>(dt*1.015));
            float saw3=2.0f*c-1.0f-blep(c,static_cast<float>(dt*.985));
            float choir=(saw1*.38f+saw2*.33f+saw3*.29f);
            float fog=(std::sin(tau*a)+std::sin(tau*b*1.005f)+std::sin(tau*c*.994f))*.24f;
            float flutter=std::sin(tau*lfo)*(p.drift*.42f)+std::sin(tau*lfo*7.0f)*p.drift*.06f;
            worn+=lpAlpha(150.0f+1300.0f*p.texture,static_cast<float>(sr))*(rnd-worn);
            float radio=(rnd-worn)*p.stat*(.07f+.11f*std::max(0.0f,flutter+.1f)+.09f*p.mod);
            float tide=(1.0f-.42f*p.dust)*(1.0f+.15f*flutter+.11f*p.mod*std::sin(tau*lfo));
            x=(choir*p.texture+fog*(1.0f-p.texture)+radio)*tide;
            low+=lpAlpha(320.0f+p.texture*7200.0f,static_cast<float>(sr))*(x-low);
            x=low*.40f;side=(saw2-saw3)*(.10f+.22f*p.drift)*p.texture;
        } else {
            float damp=std::exp(-lifetime*(1.3f+2.0f*(1.0f-p.sea)));
            float sweep=std::sin(tau*(a+p.ping*.12f*std::exp(-lifetime*8.0f)));
            float bell=std::sin(tau*b*2.47f)*.35f+std::sin(tau*c*3.79f)*.19f;
            float pingTone=(sweep*(.5f+.5f*p.signal)+bell*p.depth)*damp;
            float guitar=stringSample*std::exp(-lifetime*(.25f+2.5f*(1.0f-p.pluck)));
            float sonarPulse=std::sin(tau*(lifetime*(.32f+p.echo*.6f)))*.16f*p.echo;
            x=(pingTone*p.ping+guitar*(.2f+.8f*p.pluck)+sonarPulse*guitar)*.67f;
            low+=lpAlpha(250.0f+p.depth*10800.0f,static_cast<float>(sr))*(x-low);
            x=low;side=(bell*.1f+sonarPulse)*p.sea*damp;
        }
        const float noteGain=velocity*velocity*.65f+velocity*.35f;
        float amp=env*noteGain;
        float gain=(engine==0?.68f:engine==1?.38f:.52f);
        float l=soft((x-side)*amp*gain),r=soft((x+side)*amp*gain);
        if(!std::isfinite(l)||!std::isfinite(r)) {active=false;return {};}
        return {l,r};
    }
private:
    double sr=48000.0,hz=220.0,glideHz=220.0,phaseA=0,phaseB=0,phaseC=0,phaseD=0;
    float velocity=0,env=0,low=0,worn=0,lastString=0;
    double age=0;
    std::array<float,4096> stringData{};
    int stringSize=1,stringAt=0;
};

class TapeEcho {
public:
    void prepare(double sampleRate) {
        sr=static_cast<float>(sampleRate);
        for(auto& line:lines) {
            line.l.assign(static_cast<size_t>(sr*3.0f)+4,0.0f);
            line.r.assign(static_cast<size_t>(sr*3.0f)+4,0.0f);
            line.pos=0;line.filterL=0;line.filterR=0;
        }
        smoothSamples=sr*.35f;styleMix={1.0f,0.0f,0.0f};phase=0.0f;
    }
    void reset() noexcept {
        for(auto& line:lines) {std::fill(line.l.begin(),line.l.end(),0);std::fill(line.r.begin(),line.r.end(),0);line.pos=0;line.filterL=line.filterR=0;}
    }
    Stereo process(Stereo in,const Params& p) noexcept {
        smoothSamples+=(sr*(.07f+1.05f*p.tapeTime)-smoothSamples)*.00013f;
        int idx=std::max(0,std::min(2,p.tapeType));
        for(int i=0;i<3;i++) styleMix[static_cast<size_t>(i)]+=((i==idx?1.0f:0.0f)-styleMix[static_cast<size_t>(i)])*.0012f;
        phase+=tau*.46f/sr;if(phase>tau)phase-=tau;
        Stereo wet{};
        for(int i=0;i<3;i++) {
            auto& line=lines[static_cast<size_t>(i)];
            float offset=(i==0?1.0f:i==1?.55f:1.45f);
            float wobble=(i==0?.6f:i==1?3.5f:8.0f)*std::sin(phase*(i==2?.37f:1.0f));
            float delay=clamp(smoothSamples*offset+wobble,2.0f,static_cast<float>(line.l.size()-3));
            float read=static_cast<float>(line.pos)-delay;
            while(read<0)read+=static_cast<float>(line.l.size());
            int n=static_cast<int>(read);
            int n2=(n+1)%static_cast<int>(line.l.size());
            float frac=read-static_cast<float>(n);
            float a=line.l[static_cast<size_t>(n)]*(1-frac)+line.l[static_cast<size_t>(n2)]*frac;
            float b=line.r[static_cast<size_t>(n)]*(1-frac)+line.r[static_cast<size_t>(n2)]*frac;
            float damping=i==0?.18f:i==1?.34f:.07f;
            line.filterL+=damping*(a-line.filterL);
            line.filterR+=damping*(b-line.filterR);
            float feedback=clamp(p.tapeFeedback,0,.94f);
            float f=i==0?feedback*.74f:i==1?feedback*.85f:feedback*.9f;
            float pushL=(i==1?line.filterR:line.filterL);
            float pushR=(i==1?line.filterL:line.filterR);
            line.l[line.pos]=soft(in.l+pushL*f);
            line.r[line.pos]=soft(in.r+pushR*f);
            if(++line.pos>=line.l.size())line.pos=0;
            float width=i==1?.85f:1.0f;
            wet.l+=(a*width+b*(1-width)*.25f)*styleMix[static_cast<size_t>(i)];
            wet.r+=(b*width+a*(1-width)*.25f)*styleMix[static_cast<size_t>(i)];
        }
        return {soft(in.l+wet.l*p.tapeMix*.72f),soft(in.r+wet.r*p.tapeMix*.72f)};
    }
private:
    struct Line {std::vector<float> l,r;size_t pos=0;float filterL=0,filterR=0;};
    std::array<Line,3> lines;
    std::array<float,3> styleMix{1,0,0};
    float sr=48000,smoothSamples=0,phase=0;
};
}
