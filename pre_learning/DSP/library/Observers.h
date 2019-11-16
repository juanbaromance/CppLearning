#pragma once

#include <cmath>
#include <string>
#include "iFilter.h"

/* Further info see :: https://en.wikipedia.org/wiki/Exponential_smoothing */
class ExponentialSmoother: public iFilter
{
public:
    ExponentialSmoother( std::string name_, int N = 1 );

    // iFilter implementation
private:
    int   reset( float value, bool ) { yk_1 = value; return 0; }
    float step( float xk ){ return update( xk ); }
    float state() { return yk_1; }
    void  testing(){}

private:
    friend iFilter;
    int plateau();
    float update( float xk, float aux = 100000 );
    std::string name;
    float alpha_decay, yk_1;
};

#include <tuple>
#include <sstream>
class AlphaBetaObserver : public iFilter {

public:
    AlphaBetaObserver( std::string name, float alpha = 0.5, float beta = 0.01 );
    void settings( float Q, float R, float sampling );
    std::string report();

    // iFilter implementation
private:
    int  reset( float measure, bool );
    void testing();
    float state();
    float step( float xm );

private:
    std::string _name;
    using state_t  = std::tuple<float, float >;
    state_t _state;
    float _alpha, _beta, _xk_1, _vk_1, _xk, _vk;

    float step( float xm, float elapsed = 0.1 );
    /* see https://en.wikipedia.org/wiki/Alpha_beta_filter choice parameters stuff */
    void settings( float alpha, float beta );


};

#include <vector>
class MedianFilter : public iFilter {

public:
    MedianFilter( std::string name = "unknown", int window = 9, const std::vector <float> & weights = std::vector <float>() );

    // iFilter implementation
private:
    void testing();
    int reset( float init_val, bool );
    float state();
    float step( float xm );

private:
    int index_;
    float _xk;
    std::string _name;
    std::vector <float> _window;
    std::vector <float> _buffer;
    std::vector <float> _weights;
    std::vector <float>::iterator j;

};

class ScalarKalman : public iFilter {

public:
    ScalarKalman( std::string name, float Q = 0.01, float R = 0.1 );
    void settings( float Q, float R );

    // iFilter implementation
private:
    float step( float xm  );
    void  testing();
    int   reset( float initial_val, bool );
    float state();

private:
    std::string _name;
    float _Q, _R, _xk_1, _Pk_1, _A, _H, _K;

};

