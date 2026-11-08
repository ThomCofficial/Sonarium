#include "SonariumDSP.h"
#include <iostream>
#include <cmath>
#include <cstdint>
#include <algorithm>
int main() {
    sonarium::Params p;
    uint32_t seed=0xBADC0DEu;
    sonarium::TapeEcho echo; echo.prepare(48000);
    double total=0;
    constexpr int samples=48000*3;
    for(int mode=0;mode<3;mode++) {
        for(int note: {36,48,60,72}) {
            sonarium::Voice v;
            v.start(note,.84f,mode,48000,seed,1);
            p.mode=mode;p.tapeType=mode;
            double energy=0;
            for(int n=0;n<samples;n++) {
                if(n==48000) v.release();
                float t=static_cast<float>(n%48000)/48000;
                p.drive=t;p.stat=t;p.depth=t;p.tapeFeedback=t*.9f;p.tapeTime=t;p.tapeMix=.5f;
                auto x=v.render(p,seed);
                auto y=echo.process(x,p);
                if(!std::isfinite(y.l)||!std::isfinite(y.r)||std::abs(y.l)>3||std::abs(y.r)>3) {
                    std::cerr << "FAIL: non-finite/unbounded sample at mode=" <<mode<< " note="<<note<<" n="<<n<<"\n";
                    return 1;
                }
                energy+=static_cast<double>(x.l)*x.l;
            }
            if(energy<.00001) {std::cerr << "FAIL: silent voice mode="<<mode<<" note="<<note<<"\n";return 2;}
            total+=energy;
        }
    }
    echo.reset();
    // v0.2: every sound engine must respond audibly to its signature controls.
    // Same random seed/voice/note for A/B; compare full rendered waveforms,
    // not simply peak levels. This catches cosmetic/inert knobs.
    for(int mode=0;mode<3;mode++){
        sonarium::Params a,b;
        a.mode=b.mode=mode;
        a.tapeMix=b.tapeMix=0;
        if(mode==0){a.sub=1;a.drive=0;a.electric=0;a.mute=0;b.sub=.05f;b.drive=1;b.electric=1;b.mute=.8f;}
        if(mode==1){a.stat=0;a.drift=0;a.dust=.05f;a.texture=.05f;b.stat=1;b.drift=1;b.dust=1;b.texture=1;}
        if(mode==2){a.ping=.10f;a.depth=.05f;a.signal=1;a.sea=0;b.ping=1;b.depth=1;b.signal=.18f;b.sea=1;}
        uint32_t sa=0x7127abcd,sb=0x7127abcd;
        sonarium::Voice va,vb;
        va.start(48,.92f,mode,48000,sa,1);
        vb.start(48,.92f,mode,48000,sb,1);
        double difference=0,totalA=0,totalB=0;
        for(int n=0;n<48000;n++){
            auto xa=va.render(a,sa),xb=vb.render(b,sb);
            double delta=xa.l-xb.l;
            difference+=delta*delta;totalA+=xa.l*xa.l;totalB+=xb.l*xb.l;
            if(!std::isfinite(xa.l)||!std::isfinite(xb.l))
                {std::cerr<<"FAIL: non-finite extreme parameter signal\n";return 3;}
        }
        if(totalA<.000001||totalB<.000001||difference<.001) {
            std::cerr<<"FAIL: inert/silent controls mode "<<mode<<" differences "<<difference<<"\n";
            return 4;
        }
    }
    // Deliberately hostile host automation: high rate, full modulation and
    // release across sample rates, including low and high MIDI notes.
    for(float sampleRate:{44100.0f,48000.0f,96000.0f,192000.0f}){
      for(int mode=0;mode<3;mode++){
        uint32_t s=0x749203abu;
        sonarium::Voice v;
        sonarium::Params q;
        q.mode=mode;
        v.start(mode==0?24:mode==1?69:108,1.0f,mode,sampleRate,s,1);
        const int nMax=int(sampleRate*.35f);
        float last=0,maxAbs=0;
        for(int n=0;n<nMax;n++){
          float z=(n/109)%2==0?0.0f:1.0f;
          q.sub=z;q.drive=z;q.mute=1-z;q.electric=z;q.tone=z;
          q.stat=z;q.drift=z;q.dust=z;q.padBloom=z;q.texture=z;
          q.ping=z;q.depth=z;q.echo=z;q.pluck=z;q.sea=z;q.signal=z;
          q.pitch=(n/113)%2?1:-1;q.mod=z;q.glide=z;
          if(n==nMax*2/3)v.release();
          auto x=v.render(q,s);
          if(!std::isfinite(x.l)||!std::isfinite(x.r)||std::abs(x.l)>1.101f||std::abs(x.r)>1.101f) {
            std::cerr<<"FAIL: unsafe signal mode="<<mode<<" rate="<<sampleRate<<" sample="<<n<<"\n";
            return 5;
          }
          maxAbs=std::max(maxAbs,std::abs(x.l));
          // Spike guard: a jump greater than 1.7 full scale in a single
          // sample is never a desirable result of parameter automation.
          if(std::abs(x.l-last)>1.70f) {
            std::cerr<<"FAIL: abrupt transient under automation\n";
            return 6;
          }
          last=x.l;
        }
        if(maxAbs<.00001f) {std::cerr<<"FAIL: silent extreme signal\n";return 7;}
      }
    }
    echo.reset();
    std::cout<<"PASS: 12 voice scenarios; 1.7 million samples; energy "<<total<<"\n";
    return 0;
}
