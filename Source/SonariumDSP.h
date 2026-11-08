#pragma once
// Sonarium v0.2 DSP — independent nonlinear maritime synthesis engines.
// All per-voice memory is fixed-size; no malloc, mutex or I/O in render().
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace sonarium {
constexpr float pi=3.14159265358979323846f;
constexpr float tau=6.2831853071795864769f;
inline float clamp(float x,float a,float b) noexcept {return std::max(a,std::min(b,x));}
inline float smooth(float a,float b,float rate) noexcept {return a+(b-a)*rate;}
inline float soft(float x) noexcept {return x/(1.0f+std::abs(x));}
inline float sat(float x) noexcept {return x/(1.0f+0.32f*std::abs(x)+0.045f*x*x);}
inline float noise(uint32_t& s) noexcept {if(s==0)s=0x9e3779b9u;s^=s<<13;s^=s>>17;s^=s<<5;return float(s&0x00ffffffu)*(2.0f/16777216.0f)-1.0f;}
inline float blep(float phase,float dt) noexcept {
    if(phase<dt){float t=phase/dt;return t+t-t*t-1.0f;}
    if(phase>1.0f-dt){float t=(phase-1.0f)/dt;return t*t+t+t+1.0f;}
    return 0.0f;
}
inline float tick(double& ph,double f,double sr) noexcept {
    ph+=f/sr;
    if(ph>=1.0)ph-=1.0;
    if(ph<0.0)ph+=1.0;
    return static_cast<float>(ph);
}
inline float saw(float phase,float step) noexcept {
    return 2.0f*phase-1.0f-blep(phase,clamp(step,0.0000001f,0.45f));
}
inline float pulse(float p,float d,float step) noexcept {
    float v=p<d?1.0f:-1.0f;
    v+=blep(p,step);
    float q=p-d;if(q<0)q+=1.0f;
    v-=blep(q,step);
    return v;
}
inline float pole(float hz,float sr) noexcept {
    float g=tau*clamp(hz,5.0f,sr*.42f)/sr;
    return g/(1.0f+g);
}
struct Stereo {float l=0,r=0;};
struct Params {
    int mode=2,poly=16,tapeType=1;
    float sub=.65f,drive=.25f,mute=.45f,electric=.28f,tone=.55f;
    float stat=.35f,drift=.4f,dust=.3f,padBloom=.58f,texture=.45f;
    float ping=.83f,depth=.70f,echo=.65f,pluck=.75f,sea=.72f,signal=.60f;
    float tapeTime=.53f,tapeFeedback=.50f,tapeMix=.40f;
    float room=.10f,bloom=.52f,plate=.24f,glide=0.0f,master=.77f,mod=0.0f,pitch=0.0f;
};

struct StateVariableFilter {
    // TPT 2-pole filter, stable at all resonances and sample rates.
    float low=0,band=0;
    void reset() noexcept {low=band=0;}
    struct Result{float low,band,high;};
    Result process(float x,float fc,float resonance,float sr) noexcept {
        const float omega=pi*clamp(fc,16.0f,sr*.39f)/sr;
        // tan(pi*f/sr) approximation; accurate at musical cutoffs and
        // avoids trigonometric calls per voice per sample.
        const float g=omega*(1.0f+0.333333f*omega*omega+0.133333f*omega*omega*omega*omega)
                 /(1.0f-0.15f*omega*omega);
        const float k=2.05f-1.87f*clamp(resonance,0.0f,.97f);
        const float a1=1.0f/(1.0f+g*(g+k));
        const float v1=(band+g*(x-low))*a1;
        const float v2=low+g*v1;
        band=clamp(2.0f*v1-band,-6.0f,6.0f);
        low=clamp(2.0f*v2-low,-6.0f,6.0f);
        return {v2,v1,x-k*v1-v2};
    }
};

class Voice {
public:
    bool active=false,held=false,sustained=false;
    int note=-1,engine=0;
    uint64_t stamp=0;
    void start(int midi,float vel,int sound,double sampleRate,uint32_t& seed,uint64_t stampValue,double initialFrequency=0.0) noexcept {
        active=true;held=true;sustained=false;note=midi;engine=sound;stamp=stampValue;
        velocity=clamp(vel,0.0f,1.0f);
        sr=clamp(static_cast<float>(sampleRate),22050.0f,192000.0f);
        hz=440.0f*std::pow(2.0f,(float(midi)-69.0f)/12.0f);
        glideHz=initialFrequency>0.0?float(initialFrequency):hz;
        env=0.0f;age=0;phA=phB=phC=phD=phE=0.0;
        resL.reset();resR.reset();radioHP=radioLP=wearFilter=0.0f;
        dcInL=dcOutL=dcInR=dcOutR=0.0f;
        driveSmooth=.25f;toneSmooth=.5f;staticSmooth=.3f;
        driftRandom=0.0f;phaseDrift=0.0f;
        lastString=0;
        stringSize=std::max(8,std::min(int(stringData.size())-1,int(sr/hz)));
        for(int i=0;i<stringSize;i++)
            stringData[size_t(i)]=noise(seed)*.48f;
        stringPos=0;
        pingPhase=0.0f;chirp=0.0f;radioBit=0.0f;radioCounter=0;
        level=0.0f;
    }
    void release() noexcept {held=false;sustained=false;}
    Stereo render(const Params& p,uint32_t& seed) noexcept {
        if(!active)return {};
        const float r=1.0f/sr;
        ++age;
        // Envelopes are deliberately different per instrument; no
        // shared harpsichord/pluck contour on sustained pads and bass.
        const float attack=engine==0?.005f:
                           engine==1?.06f+2.0f*p.padBloom:
                           .010f+.09f*(1.0f-p.signal);
        const float releaseTime=engine==0?.095f+.63f*(1.0f-p.mute):
                                engine==1?.75f+5.0f*p.padBloom:
                                .11f+2.7f*p.sea;
        const float envCoeff=r/(r+(held?attack:releaseTime));
        env+=(held?1.0f-env:-env)*envCoeff;
        if(!held&&env<0.00008f){active=false;return {};}
        float time=static_cast<float>(age)*r;
        float goal=hz*std::pow(2.0f,p.pitch*(2.0f/12.0f));
        float glideSec=.0001f+.75f*p.glide*p.glide;
        glideHz+=(goal-glideHz)*(r/(r+glideSec));
        float fundamental=clamp(glideHz,10.0f,sr*.42f);
        const float dt=clamp(fundamental*r,0.000001f,0.43f);
        const float a=tick(phA,fundamental,sr);
        const float lfo=tick(phD,.071f+p.drift*.47f+p.sea*.27f,sr);
        // Separate per-note modulation: slow deterministic drift plus
        // stochastic magnetic instability, strongly smoothed.
        driftRandom+=(noise(seed)-driftRandom)*.00008f;
        phaseDrift+=(driftRandom-phaseDrift)*.0007f;
        const float wow=std::sin(tau*lfo)*p.drift+phaseDrift*.4f;
        const float b=tick(phB,fundamental*(1.003f+.015f*p.drift)+wow*.4f,sr);
        const float c=tick(phC,fundamental*(.996f-.013f*p.drift)-wow*.5f,sr);
        const float d=tick(phE,fundamental*.502f,sr);
        const float rnd=noise(seed);

        driveSmooth=smooth(driveSmooth,p.drive,.0012f);
        toneSmooth=smooth(toneSmooth,p.tone,.0014f);
        staticSmooth=smooth(staticSmooth,p.stat,.001f);
        float xL=0,xR=0;
        if(engine==0){
            // GENERATIVE BASS — phase-modulated sine stack, wavefold
            // and continuously variable damped pickup character.
            const float subOsc=std::sin(tau*a);
            const float octave=std::sin(tau*d);
            const float modulator=std::sin(tau*b*2.0f);
            const float pm=.08f+driveSmooth*1.1f+.50f*p.mod;
            const float complex=std::sin(tau*a+pm*modulator);
            const float folded=std::sin((1.0f+driveSmooth*4.6f)*complex);
            const float harmonics=(.58f*folded+.42f*saw(a,dt))*(.11f+driveSmooth*.68f);
            const float unfiltered=clamp(subOsc*(.42f+.72f*p.sub)+
                                      octave*(.11f+.24f*p.sub)+harmonics,-3.0f,3.0f);
            // Analog-biased soft clipping produces rich harmonics without
            // turning every bass patch into a short attack transient.
            float shaped=sat((unfiltered+.07f*driveSmooth)*(1.0f+5.0f*driveSmooth))-
                         sat(.07f*driveSmooth*(1.0f+5.0f*driveSmooth));
            const float cutoff=55.0f+std::pow(toneSmooth,1.7f)*10000.0f;
            const float freqEnv=1.0f+1.6f*std::exp(-time*(2.0f+8.0f*p.mute));
            const float resonance=.12f+.48f*p.electric;
            auto vL=resL.process(shaped,cutoff*freqEnv,resonance,sr);
            auto vR=resR.process(shaped,cutoff*freqEnv*.991f,resonance,sr);
            const float dampSustain=1.0f-p.mute*.73f;
            const float dampEnv=dampSustain+(1.0f-dampSustain)*std::exp(-time*(2.5f+10.0f*p.mute));
            float pickup=0;
            if(p.electric>.004f){
                pickup=stringData[size_t(stringPos)];
                const int nextIndex=(stringPos+1)%stringSize;
                const float avg=.5f*(pickup+stringData[size_t(nextIndex)]);
                stringData[size_t(stringPos)]=avg*(.986f+.013f*(1-p.mute));
                stringPos=nextIndex;
                pickup=(pickup-lastString*.2f)*std::exp(-time*(.9f+5.0f*p.mute));
                lastString=pickup;
            }
            const float mass=(.32f+.18f*p.sub)*dampEnv;
            xL=(vL.low*.84f+vL.band*.17f)*mass+pickup*p.electric*.17f;
            xR=(vR.low*.84f+vR.band*.17f)*mass+pickup*p.electric*.15f;
        } else if(engine==1) {
            // COMPLEX WEST COAST / EASTERN-BLOC PAD — resonant sweeping
            // TPT filter and dynamically folded dual oscillators, evolving
            // across time (not a static three-saw sample).
            const float w=std::sin(tau*a);
            const float mod=std::sin(tau*b*1.004f);
            const float pm=mod*(.20f+.86f*p.texture)+.12f*std::sin(tau*c*2.0f);
            const float fold=std::sin((1.0f+p.texture*4.1f)*std::sin(tau*a+pm));
            const float sawL=saw(b,clamp(dt*(1.003f+.015f*p.drift),0.000001f,.43f));
            const float sawR=saw(c,clamp(dt*(.996f-.013f*p.drift),0.000001f,.43f));
            const float duty=.26f+.38f*(.5f+.5f*std::sin(tau*lfo*.53f));
            const float square=pulse(a,duty,dt);
            const float evolving=.49f*fold +.23f*(sawL+sawR)+.20f*square+.08f*w;
            // Russian-style filter saturation: input drive and an extra
            // clipped band component create a rough almost 'singing' peak.
            const float evolvingCut=std::sin(tau*lfo)*.5f+.5f;
            float cut=160.0f+std::pow(p.texture,.66f)*4900.0f;
            cut*=.56f+1.3f*evolvingCut+.33f*p.mod;
            const float resonance=.23f+.62f*p.dust;
            const float hot=sat(evolving*(1.0f+4.0f*p.texture));
            auto vl=resL.process(hot,cut*(1.0f+wow*.16f),resonance,sr);
            auto vr=resR.process(hot+.06f*(sawR-sawL),cut*(1.015f-wow*.18f),resonance,sr);
            const float gritL=sat(vl.band*(1.0f+4.0f*p.dust));
            const float gritR=sat(vr.band*(1.0f+4.0f*p.dust));
            // A real radio/static source with two independent filtered
            // stages: no raw full-spectrum digital white noise at the output.
            wearFilter+=pole(640.0f+p.stat*1700.0f,sr)*(rnd-wearFilter);
            float radio=(rnd-wearFilter);
            radioHP+=pole(200.0f,sr)*(radio-radioHP);
            radio-=radioHP;
            const float dropout=.65f+.35f*std::sin(tau*lfo*.49f);
            const float staticLevel=staticSmooth*(.020f+.055f*p.dust)*dropout;
            const float wear=(1.0f-.28f*p.dust)*(1.0f+.16f*wow);
            xL=(vl.low*.43f+gritL*.23f+radio*staticLevel)*wear;
            xR=(vr.low*.43f+gritR*.23f-radio*staticLevel*.55f)*wear;
        } else {
            // MARITIME SONAR / SHORTWAVE — repeated keyed transmitter
            // bursts, chirping crystal oscillators, receiver squelch,
            // bandwidth reduction and decaying underwater reflections.
            const float speed=.42f+4.0f*p.ping;
            pingPhase+=speed*r;if(pingPhase>=1.0f)pingPhase-=1.0f;
            const float pulseAge=pingPhase;
            const float chirpAmt=.045f+.22f*p.depth;
            const float fm=std::exp(-pulseAge*(3.0f+7.0f*p.depth));
            const float tone=std::sin(tau*a+chirpAmt*std::sin(tau*b*2.4f)*fm);
            const float crystal=std::sin(tau*b*2.73f)*.28f+std::sin(tau*c*3.2f)*.14f;
            const float gate=std::exp(-pulseAge*(7.0f+30.0f*p.signal));
            const float radioTone=(tone+crystal*p.depth)*gate*(.34f+.66f*p.signal);
            float guitar=0.0f;
            if(p.pluck>.005f) {
                const float old=stringData[size_t(stringPos)];
                const int nextIndex=(stringPos+1)%stringSize;
                const float avg=.5f*(old+stringData[size_t(nextIndex)]);
                stringData[size_t(stringPos)]=avg*(.966f+.029f*p.pluck);
                stringPos=nextIndex;
                guitar=old*std::exp(-time*(1.6f+3.4f*(1.0f-p.pluck)));
            }
            // Walkie-talkie squelch: grit only while the transmission opens,
            // punctuated by irregular loss of reception at high SEA.
            const float noiseGate=gate*(.22f+.62f*p.sea);
            const float staticTone=rnd*noiseGate*.11f;
            const float carrier=sat((radioTone+staticTone+guitar*p.pluck*.30f)*(1.4f+2.3f*p.sea));
            // Cascaded highpass/lowpass analogue-ish receiver bandpass.
            float hpFreq=160.0f+640.0f*(1.0f-p.signal);
            radioHP+=pole(hpFreq,sr)*(carrier-radioHP);
            float high=carrier-radioHP;
            float lpFreq=1500.0f+4400.0f*p.signal;
            radioLP+=pole(lpFreq,sr)*(high-radioLP);
            const float bw=radioLP;
            // Resonant sonar resonator becomes a musical synth tone with
            // DEPTH / PLUCK / SIGNAL; gated packet is not a generic harpsichord.
            auto vl=resL.process(bw,520.0f+2800.0f*p.depth,.25f+.52f*p.echo,sr);
            auto vr=resR.process(bw,540.0f+2900.0f*p.depth,.25f+.52f*p.echo,sr);
            const float underwater=std::sin(tau*lfo*1.3f)*.06f*p.sea;
            xL=vl.band*.36f+vl.low*.43f+underwater*gate;
            xR=vr.band*.37f+vr.low*.42f-underwater*gate;
        }

        const float noteGain=.25f+.75f*velocity*velocity;
        const float aGain=(engine==0?.66f:engine==1?.57f:.82f)*env*noteGain;
        const float l=aGain*xL,rOut=aGain*xR;
        // One-pole DC rejection is important when heavily driven controls
        // are automated or when many voices overlap.
        const float dcL=l-dcInL+.996f*dcOutL;
        const float dcR=rOut-dcInR+.996f*dcOutR;
        dcInL=l;dcInR=rOut;dcOutL=dcL;dcOutR=dcR;
        level=smooth(level,std::max(std::abs(dcL),std::abs(dcR)),.008f);
        if(!std::isfinite(dcL)||!std::isfinite(dcR)||std::abs(dcL)>50.0f||std::abs(dcR)>50.0f){
            active=false;resL.reset();resR.reset();return {};
        }
        return {clamp(dcL,-1.1f,1.1f),clamp(dcR,-1.1f,1.1f)};
    }
    float getActivity() const noexcept {return level;}
private:
    float sr=48000.0f,hz=220.0f,glideHz=220.0f,velocity=0.0f,env=0.0f;
    double phA=0,phB=0,phC=0,phD=0,phE=0;
    uint64_t age=0;
    float radioHP=0,radioLP=0,wearFilter=0,driftRandom=0,phaseDrift=0;
    float dcInL=0,dcOutL=0,dcInR=0,dcOutR=0;
    float driveSmooth=.25f,toneSmooth=.5f,staticSmooth=.3f;
    float lastString=0.0f,pingPhase=0.0f,chirp=0.0f,radioBit=0.0f,level=0.0f;
    int stringSize=8,stringPos=0,radioCounter=0;
    std::array<float,4096> stringData{};
    StateVariableFilter resL,resR;
};

class TapeEcho {
public:
    void prepare(double sampleRate){
        sr=clamp(float(sampleRate),22050.0f,192000.0f);
        for(auto& line:lines){
            line.l.assign(size_t(sr*2.5f)+4,0.0f);
            line.r.assign(size_t(sr*2.5f)+4,0.0f);
            line.pos=0;line.filterL=line.filterR=0.0f;
        }
        smoothSamples=sr*.33f;styleMix={0,1,0};phase=0;smoothMix=0;
    }
    void reset() noexcept{
        for(auto& line:lines){
            std::fill(line.l.begin(),line.l.end(),0.0f);
            std::fill(line.r.begin(),line.r.end(),0.0f);
            line.pos=0;line.filterL=line.filterR=0.0f;
        }
    }
    Stereo process(Stereo in,const Params& p) noexcept {
        if(lines[0].l.size()<4)return in;
        smoothSamples+=(sr*(.080f+.97f*p.tapeTime)-smoothSamples)*.00016f;
        smoothMix+=(p.tapeMix-smoothMix)*.0014f;
        const int idx=std::max(0,std::min(2,p.tapeType));
        for(int i=0;i<3;i++)
            styleMix[size_t(i)]+=((i==idx?1.0f:0.0f)-styleMix[size_t(i)])*.0009f;
        phase+=tau*.51f/sr;if(phase>tau)phase-=tau;
        Stereo wet{};
        for(int i=0;i<3;i++){
            auto& line=lines[size_t(i)];
            const float offset=(i==0?.81f:i==1?1.0f:1.27f);
            const float wow=(i==0?.46f:i==1?4.3f:12.0f)*std::sin(phase*(i==2?.81f:1.0f));
            const float delay=clamp(smoothSamples*offset+wow,2.0f,float(line.l.size()-3));
            float read=float(line.pos)-delay;
            if(read<0)read+=float(line.l.size());
            const int n=int(read),n2=(n+1)%int(line.l.size());
            const float t=read-float(n);
            const float a=smooth(line.l[size_t(n)],line.l[size_t(n2)],t);
            const float b=smooth(line.r[size_t(n)],line.r[size_t(n2)],t);
            const float damp=i==0?.25f:i==1?.16f:.075f;
            line.filterL+=damp*(a-line.filterL);
            line.filterR+=damp*(b-line.filterR);
            const float feedback=clamp(p.tapeFeedback,0.0f,.9f)*(i==0?.81f:i==1?.86f:.91f);
            const float left=i==1?line.filterR:line.filterL;
            const float right=i==1?line.filterL:line.filterR;
            line.l[line.pos]=soft(in.l+left*feedback);
            line.r[line.pos]=soft(in.r+right*feedback);
            if(++line.pos>=line.l.size())line.pos=0;
            wet.l+=a*styleMix[size_t(i)];
            wet.r+=b*styleMix[size_t(i)];
        }
        return {soft(in.l+wet.l*smoothMix*.64f),soft(in.r+wet.r*smoothMix*.64f)};
    }
private:
    struct Line {std::vector<float> l,r;size_t pos=0;float filterL=0,filterR=0;};
    std::array<Line,3> lines;
    std::array<float,3> styleMix{0,1,0};
    float sr=48000.0f,smoothSamples=0,phase=0,smoothMix=0;
};
}
