#include "Observers.h"

ExponentialSmoother::ExponentialSmoother( std::string name_, int N ) : name( name_ ), alpha_decay( 1./N ), yk_1( 0 ) {  }
int ExponentialSmoother::plateau(){ return ( static_cast<int>( round((1 / alpha_decay)))); }
float ExponentialSmoother::update(float xk, float aux)
{
    yk_1 = ( aux == 100000) ? yk_1 : aux;
    return yk_1 += alpha_decay * (xk - yk_1) ;
}

MedianFilter::MedianFilter(std::string name, int window, const std::vector<float> &weights)
    : index_( 0 ), xk_( 0 ), name_( name ),
      window_( std::vector<float>( window, xk_ ) ), buffer_( window ), weights_( weights )
{
    j = window_.begin() + window_.size()/2 - ( ( weights_.size() -1 )/2 );
}
void MedianFilter::testing(){}
int  MedianFilter::reset(float init_val, bool)
{
    xk_ = init_val;
    return 0;
}

#include <algorithm>
float MedianFilter::step(float xm)
{
    buffer_[ index_++ ] = xm;
    index_ %= window_.size();

    window_ = buffer_;
    std::sort( window_.begin(), window_.end() );
    xk_ = 0;

    std::vector <float>::iterator i = j;
    std::vector <float>::iterator w = weights_.begin() ;
    for( w = weights_.begin(); w != weights_.end(); w++ )
    {
        xk_ = xk_ + ( w[0] * i[0] );
        i++ ;
    }
    return xk_;
}

AlphaBetaObserver::AlphaBetaObserver(std::string name, float alpha, float beta)
    : name_( name ), alpha_( alpha ), beta_( beta )
{ reset( 0, true); }
void AlphaBetaObserver::settings(float Q, float R, float sampling)
{
    float lambda = sampling * sampling * ( Q / R );
    float r = ( 4 + lambda - sqrt( lambda * ( 8 + lambda ) ) ) * .25;
    float alpha = 1 - ( r * r );
    float beta  = ( 2 *( 2 - alpha ) ) - ( 4 * sqrt( 1 - alpha ) );
    settings( alpha, beta );
}
std::string AlphaBetaObserver::report()
{
    std::ostringstream oss;
    oss << name_ << " :: parameters \u03b1(" << alpha_ << ")," << "\u03b2(" << beta_ << ")";
    return oss.str();
}
float AlphaBetaObserver::step(float xm, float elapsed )
{
    /* The estimations for x and v */
    float xk_e = xk_1 + ( vk_1 * elapsed );
    float vk_e = vk_1;

    /* The x error */
    float rk = xm - xk_e;

    /* Now the error additive/learning part to the estimation */
    xk  = xk_e + ( alpha_ * rk );
    vk  = vk_e + ( ( beta_ * rk ) / elapsed );

    /* History the current output */
    xk_1 = xk;
    vk_1 = vk;
    state_ = state_t( xk, vk );
    return xk;
}
void  AlphaBetaObserver::settings(float alpha, float beta)
{
    alpha_ = alpha;
    beta_  = beta;
}
int   AlphaBetaObserver::reset(float measure, bool)
{
    xk_1 = xk = measure;
    vk_1 = vk = 0;
    return 0;
}
void  AlphaBetaObserver::testing(){}
float AlphaBetaObserver::state() { return xk_1; }
float AlphaBetaObserver::step( float xm ){ return step( xm, 0.010 ); }

ScalarKalman::ScalarKalman(std::string name, float Q_, float R_)
    : _name( name ), Q( Q_ ), R( R_ ), A(1.0), H(1.0) { reset(0,true); }
void  ScalarKalman::settings(float Q_, float R_)
{
    Q = Q_;
    R = R_;
}
float ScalarKalman::step( float xm )
{
    /* Prediction */
    float xk_e = A * xk_1;
    float Pk_e = A * Pk_1 + Q;

    /* Update the Kalman Gain */
    K = Pk_e / ( Pk_e + R );

    /* Update estimators and store new historic */
    xk_1 = ( xk_e += ( K * ( ( xm * H ) - xk_e ) ) );
    Pk_1  = Pk_e * ( 1 - K );
    return xk_1;
}
void  ScalarKalman::testing(){}
int   ScalarKalman::reset( float initial_val, bool )
{
    xk_1 = initial_val;
    Pk_1 = 0.2;
    return 0;
}
float ScalarKalman::state(){ return xk_1; }
