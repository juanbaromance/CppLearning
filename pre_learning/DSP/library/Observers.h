#pragma once

#include <cmath>
#include <string>
#include "iFilter.h"

/* Further info see :: https://en.wikipedia.org/wiki/Exponential_smoothing */
class ExponentialSmoother: public iFilter<ExponentialSmoother>
{
public:
    ExponentialSmoother( std::string name_, int N = 15 );

    // iFilter implementation
private:
    friend iFilter<ExponentialSmoother>;
    int   reset( float value, bool ) { yk_1 = value; return 0; }
    float step( float xk ){ return update( xk ); }
    float state() { return yk_1; }
    void  testing(){}

private:
    int plateau();
    float update( float xk, float aux = 100000 );
    std::string name;
    float alpha_decay, yk_1;
};

#include <tuple>
#include <sstream>
class AlphaBetaObserver : public iFilter<AlphaBetaObserver> {

public:
    AlphaBetaObserver( std::string name, float alpha = 0.5, float beta = 0.01 );
    void settings( float Q, float R, float sampling );
    std::string report();

    // iFilter implementation
private:
    friend iFilter<AlphaBetaObserver>;
    int  reset( float measure, bool ) override;
    void testing() override;
    float state() override;
    float step( float xm ) override;

private:
    std::string name_;
    using state_t  = std::tuple<float, float >;
    state_t state_;
    float alpha_, beta_, xk_1, vk_1, xk, vk;

    float step( float xm, float elapsed );
    /* see https://en.wikipedia.org/wiki/Alpha_beta_filter choice parameters stuff */
    void settings( float alpha, float beta );


};

#include <vector>
class MedianFilter : public iFilter<MedianFilter> {

public:
    MedianFilter( std::string name = "unknown", int window = 9, const std::vector <float> & weights = std::vector <float>() );

    // iFilter implementation
private:
    friend iFilter<MedianFilter>;
    void  testing() override;
    int   reset( float init_val, bool ) override;
    float state() override { return xk_ ; }
    float step( float xm ) override;
    void setSampling(msecT) override {};

private:
    int index_;
    float xk_;
    std::string name_;
    std::vector <float> window_;
    std::vector <float> buffer_;
    std::vector <float> weights_;
    std::vector <float>::iterator j;

};

class ScalarKalman : public iFilter<ScalarKalman> {

public:
    ScalarKalman( std::string name, float Q = 0.001, float R = 0.1 );
    void settings( float Q, float R );

    // iFilter implementation
private:
    friend iFilter<ScalarKalman>;
    float step( float xm  ) override;
    void  testing() override;
    int   reset( float initial_val, bool ) override;
    float state() override;

private:
    std::string _name;
    float Q, R, xk_1, Pk_1, A, H, K;

};

