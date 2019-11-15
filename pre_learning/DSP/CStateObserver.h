#ifndef CStateObserver_H
#define CStateObserver_H

#include "common_cpp/CFilter.h"
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <tr1/tuple>
#include <sstream>
#include <iostream>
#include <algorithm>
using namespace std;


#ifndef LOG_TRACE
#define LOG_TRACE( fmt, args... ) { printf(fmt,##args);cout << endl; }
#endif


class CStateObserver {

public:
    typedef enum PublicNumerology{
        KALMAN_DISCRETE_F,
        ALPHA_BETA_F,
        MA_MEDIAN_F
    }PublicNumerology;

protected:

    typedef struct Kalman_T : public CFilter {

    public:
        Kalman_T( string name, float Q = 0.01, float R = 0.1 ) : _name( name ), _Q( Q ), _R( R )
        {
            _xk_1 = 0;
            _Pk_1 = 0.2;
            _H    = 1.0;
            _A    = 1.0;
        }

        void settings( float Q, float R )
        {
            _Q = Q;
            _R = R;
        }

    protected:
        float step( float xm, float elapsed = 0.1  )
        {
            /* Prediction */
            float xk_e = _A * _xk_1;
            float Pk_e = _A * _Pk_1 + _Q;

            /* Update the Kalman Gain */
            _K = Pk_e / ( Pk_e + _R );

            /* Update estimators and store new historic */
            _xk_1 = ( xk_e += ( _K * ( ( xm * _H ) - xk_e ) ) );
            _Pk_1  = Pk_e * ( 1 - _K );
            return( _xk_1 );
        }

        void testing(){}

        int  reset( float unused )
        {
            _xk_1 = 0;
            _Pk_1 = 0.2;
            return(0);
        }


        float state()
        {
            return( _xk_1 );
        }

    private:
        string _name;
        float _Q, _R, _xk_1, _Pk_1, _A, _H, _K;

    }Kalman_T;


    typedef struct AlphaBeta_T : public CFilter {

    public:
        typedef tuple<float, float > state_t;

        AlphaBeta_T( string name, float alpha = 0.5, float beta = 0.01 ) : _name( name ), _alpha( alpha ), _beta( beta )
        {
            reset();
        }

        void settings( float Q, float R, float sampling )
        {
            float lambda = sampling * sampling * ( Q / R );
            float r = ( 4 + lambda - sqrt( lambda * ( 8 + lambda ) ) ) * .25;
            float alpha = 1 - ( r * r );
            float beta  = ( 2 *( 2 - alpha ) ) - ( 4 * sqrt( 1 - alpha ) );
            std::ostringstream oss;

            oss << "( Process/Noise :: " << Q << "/" << R << ")";
            settings( alpha, beta );
        }

        float state()
        {
            return( _xk_1 );
        }

        string report()
        {
             std::ostringstream oss;
             oss << _name << " :: parameters \u03b1(" << _alpha << ")," << "\u03b2(" << _beta << ")";
             return( oss.str() );
        }

    protected:

        string _name;
        state_t _state;
        float _alpha, _beta, _xk_1, _vk_1, _xk, _vk;

        float step( float xm, float elapsed = 0.1 )
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

        int  reset( float measure = 0 )
        {
            _xk_1 = _xk = measure;
            _vk_1 = _vk = 0;
            return(0);
        }

        void testing(){}

        /* see https://en.wikipedia.org/wiki/Alpha_beta_filter choice parameters stuff */
        void settings( float alpha, float beta, string backtrace = ""  )
        {
            if( backtrace.size() )
                LOG_TRACE("%15s.%s%s :: Filtering settled to :: \u03b1(%.4f -> %.4f) \u03b2( %.4f -> %.4f )",
                          _name.c_str(),
                          __FUNCTION__ ,
                          backtrace.c_str(),
                          _alpha, alpha, _beta, beta );
            _alpha = alpha;
            _beta  = beta;

        }

    }AlphaBeta_T;

    typedef struct MedianFilter_T : public CFilter {

    public:
        MedianFilter_T( string name = "unknown", int window = 9, vector <float> weights = vector <float>() ) :
            _name( name ), _weights( weights )
        {
            _xk = 0;
            _window.resize( window, _xk );
            _buffer = _window;
            _index = 0;
            j = _window.begin() + _window.size()/2 - ( ( _weights.size() -1 )/2 );
        }

        float step( float xm, float elapsed = 0.1 )
        {
            _buffer[ _index++ ] = xm;
            _index %= _window.size();

            _window = _buffer;
            std::sort( _window.begin(), _window.end() );
            _xk = 0;

            vector <float>::iterator i = j;
            vector <float>::iterator w = _weights.begin() ;
            for( w = _weights.begin(); w != _weights.end(); w++ )
            {
                _xk = _xk + ( w[0] * i[0] );
                i++ ;
            }
            return( _xk );
        }

        float state()
        {
            return( _xk );
        }

    protected:
        int _index;
        float _xk;
        string _name;
        vector <float> _window;
        vector <float> _buffer;
        vector <float> _weights;
        vector <float>::iterator j;

        int reset( float measure ){
            _xk = measure;
            return(0);
        }

        void testing(){}

    }MedianFilter_T;

    Kalman_T *kalman;
    AlphaBeta_T *alpha_beta;
    MedianFilter_T *median;
    map <PublicNumerology,CFilter*> filters;

public:


    CStateObserver( string name = "unknown", float Q = 0.005, float R = 0.1, int window = 5 )
    {
        kalman     = new Kalman_T   ( name + ".SteadyKalman" );
        alpha_beta = new AlphaBeta_T( name + ".AlphaBeta"    );

	float _weights[]= { 0.05, 0.10, 0.7, 0.10, 0.05 };
        vector <float> weights( _weights, _weights + sizeof( _weights ) / sizeof( _weights[0] ) );
        median = new MedianFilter_T( name + ".Median", window, weights );
        settings(Q,R);

        filters[ MA_MEDIAN_F       ] = median;
	filters[ KALMAN_DISCRETE_F ] = kalman;
	// filters[ ALPHA_BETA_F      ] = alpha_beta;
        _name = name;
    }

    void settings( float Q, float R, float sampling = 0.1 )
    {
        alpha_beta->settings( Q, R, sampling );
        kalman->settings(Q, R );
    }

    float state( PublicNumerology index )
    {
        if( filters.find(index) == filters.end() )
            return(-12121212);
        return( filters.find(index)->second->state() );
    }

    vector <float> sync( float xm, float elapsed = 0.1 )
    {
        vector <float> tmp;
        tmp.push_back( median->step(xm) );

	map <PublicNumerology,CFilter*>::iterator f;
	for( f = filters.begin(); f != filters.end(); f++ )
	    if( f->first != MA_MEDIAN_F )
		tmp.push_back( f->second->step( tmp[0] ) );

        return( tmp );
    }

    string report()
    {
        return( alpha_beta->report() );
    }

protected:
    vector <float> buffer;
    string _name;
    int _window;
};

#endif
