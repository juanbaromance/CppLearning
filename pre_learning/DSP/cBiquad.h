#ifndef __BIQUAD_iface
#define __BIQUAD_iface

#include <vector>
#include <string>

#include "gpio_modules/public/iFilter.h"

class cBiQuad : public iFilter
{
    enum Numerology {
        LowPass,
        Notch,
    };

public:
    cBiQuad( std::string _name, const std::vector<float>& poles, const std::vector<float>& zeros, int order );
    cBiQuad (std::string _name, size_t Fs, size_t Fc, double Q = 0.7071, enum Numerology _topology = LowPass );
    std::string report();
    void  tune( size_t Fs, size_t Fc, double Q ){ ZeroPoleMap( Fs, Fc, Q, topology ); }
    virtual ~cBiQuad();

public:
    // iFilter (direct) exposition
    int reset( float input = 0, bool hardcore = false );
    float step ( float measure );
    inline float state(){ return o; }    

    // iFilter interface implementation
private:
    inline void  testing(){ return; }
    inline void  setSampling( int Fs ){ return (void)Fs; }
    std::string name(){ return _name; }

private:
    void ZeroPoleMap(size_t Fs, size_t Fc, double Q = 0.7071, enum Numerology type = LowPass );
    std::string _name;
    std::vector <float> z;
    std::vector <float> poles, zeros;
    float o;
    Numerology topology;
};

#endif


/**
  * @class  cBiQuad
  * @brief IIR BiQuad support
  *
  * @ingroup Signal Filtering
  *
  * @fn cBiQuad::cBiQuad (std::string _name, size_t Fs, size_t Fc, double Q = 0.7071, enum Numerology _topology = LowPass );
  * @brief BiQuad 2nd order calculator using Frequency domain parameters.
  * @param _name The Filter namea.
  * @param _Fs Sampling time in Hz.
  * @param _Fc Cutting Hz where -3db is applied
  * @param Q Boost/Flat(ButterWorth)/Reductor factor.
  * @param _topology Only LowPass and Notch modalities are calculated.
  *
  * @fn std::string cBiQuad::report();
  * @brief Reports the current poles-zero configuration
  *
  * @fn void cBiQuad::tune( size_t Fs, size_t Fc, double Q ){ ZeroPoleMap( Fs, Fc, Q, topology ); }
  * @brief Tune current BiQuad specifications (aks the zero/poles map) using Frequency domain parameters.
  * @param _Fs Sampling time in Hz.
  * @param _Fc Cutting Hz where -3db is applied
  * @param Q Boost/Flat(ButterWorth)/Reductor factor.
*/
