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
    std::cout<<"PASS: 12 voice scenarios; 1.7 million samples; energy "<<total<<"\n";
    return 0;
}
