#include "Observers.h"


ExponentialSmoother::ExponentialSmoother( std::string name_, int N ) : name( name_ ), alpha_decay( 1./N ), yk_1( 0 ) {  }
int ExponentialSmoother::plateau(){ return ( static_cast<int>( round((1 / alpha_decay)))); }
float ExponentialSmoother::update(float xk, float aux)
{
    yk_1 = ( aux == 100000) ? yk_1 : aux;
    return yk_1 += alpha_decay * (xk - yk_1) ;
}

MedianFilter::MedianFilter(std::string name, int window, const std::vector<float> &weights)
    : index_( 0 ), _xk( 0 ), _name( name ),
      _window( std::vector<float>( window, _xk ) ), _buffer( window ), _weights( weights )
{
    j = _window.begin() + _window.size()/2 - ( ( _weights.size() -1 )/2 );
}
void MedianFilter::testing(){}
int MedianFilter::reset(float init_val, bool)
{
    _xk = init_val;
    return 0;
}
float MedianFilter::state(){ return _xk ; }

#include <algorithm>
float MedianFilter::step(float xm)
{
    _buffer[ index_++ ] = xm;
    index_ %= _window.size();

    _window = _buffer;
    std::sort( _window.begin(), _window.end() );
    _xk = 0;

    std::vector <float>::iterator i = j;
    std::vector <float>::iterator w = _weights.begin() ;
    for( w = _weights.begin(); w != _weights.end(); w++ )
    {
        _xk = _xk + ( w[0] * i[0] );
        i++ ;
    }
    return _xk;
}

AlphaBetaObserver::AlphaBetaObserver(std::string name, float alpha, float beta) :
    _name( name ), _alpha( alpha ), _beta( beta )
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
    oss << _name << " :: parameters \u03b1(" << _alpha << ")," << "\u03b2(" << _beta << ")";
    return oss.str();
}
float AlphaBetaObserver::step(float xm, float elapsed)
{
    /* The estimations for x and v */
    float xk_e = _xk_1 + ( _vk_1 * elapsed );
    float vk_e = _vk_1;

    /* The x error */
    float rk = xm - xk_e;

    /* Now the error additive/learning part to the estimation */
    _xk  = xk_e + ( _alpha * rk );
    _vk  = vk_e + ( ( _beta * rk ) / elapsed );

    /* History the current output */
    _xk_1 = _xk;
    _vk_1 = _vk;
    _state = state_t( _xk, _vk );
    return( _xk );
}
void  AlphaBetaObserver::settings(float alpha, float beta)
{
    _alpha = alpha;
    _beta  = beta;
}
int   AlphaBetaObserver::reset(float measure, bool)
{
    _xk_1 = _xk = measure;
    _vk_1 = _vk = 0;
    return 0;
}
void  AlphaBetaObserver::testing(){}
float AlphaBetaObserver::state() { return _xk_1; }
float AlphaBetaObserver::step(float xm){ return step( xm, 0.1 ); }

ScalarKalman::ScalarKalman(std::string name, float Q, float R)
    : _name( name ), _Q( Q ), _R( R ), _A(1.0), _H(1.0) { reset(0,true); }
void  ScalarKalman::settings(float Q, float R)
{
    _Q = Q;
    _R = R;
}
float ScalarKalman::step(float xm)
{
    /* Prediction */
    float xk_e = _A * _xk_1;
    float Pk_e = _A * _Pk_1 + _Q;

    /* Update the Kalman Gain */
    _K = Pk_e / ( Pk_e + _R );

    /* Update estimators and store new historic */
    _xk_1 = ( xk_e += ( _K * ( ( xm * _H ) - xk_e ) ) );
    _Pk_1  = Pk_e * ( 1 - _K );
    return _xk_1;
}
void  ScalarKalman::testing(){}
int   ScalarKalman::reset(float initial_val, bool)
{
    _xk_1 = initial_val;
    _Pk_1 = 0.2;
    return(0);
}
float ScalarKalman::state()
{
    return( _xk_1 );
}
