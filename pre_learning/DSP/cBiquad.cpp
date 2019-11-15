#include "gpio_modules/cBiquad.h"
#include <sstream>
#include <cmath>
#include <iostream>
using namespace std;

cBiQuad::cBiQuad( string name, const vector<float> &poles, const vector<float> &zeros, int order )
    : _name( name )
{
    (void)order;
    this->poles = poles;
    this->zeros = zeros;
    z.resize( poles.size() );
    reset(0);
}

cBiQuad::cBiQuad( string name, size_t Fs, size_t Fc, double Q, cBiQuad::Numerology _topology )
    : _name( name ), topology( _topology )
{
    poles.resize(3);
    zeros.resize( poles.size() );
    z.resize( poles.size() );
    ZeroPoleMap( Fs, Fc, Q, topology );
}

cBiQuad::~cBiQuad(){ std::cout << __PRETTY_FUNCTION__ << "." << _name << endl; }
int cBiQuad::reset( float input, bool hardcore )
{
    (void)hardcore;
    for( size_t i = 0; i < z.size() -1; i++ )
        z[i] = input;
    std::cout << __PRETTY_FUNCTION__ << "." << _name << endl;
    return 0;
}

float cBiQuad::step( float input )
{
    size_t i = 0;
    float out = ( input * zeros[ 0 ] ) + z[ 0 ];
    for( i = 0; i < ( z.size() -1 ); i++ )
        z[ i ] = ( input * zeros[ i +1 ] ) - ( poles[ i +1 ]  * out )  + z[ i +1 ];
    o = out;
    return o;
}

string cBiQuad::report()
{
    std::ostringstream oss;
    oss << std::endl << "\tZeros( ";
    for( size_t i = 0; i < z.size(); i++ )
        oss << "a" << i << " " << zeros[ i ] << " ";
    oss << ")" << std::endl;

    oss << "\tPoles( ";
    for( size_t i = 0; i < z.size(); i++ )
        oss << "b" << i << " " << poles[ i ] << " ";
    oss << ")";
    return oss.str();
}

void cBiQuad::ZeroPoleMap(size_t Fs, size_t Fc, double Q, cBiQuad::Numerology type)
{

    double a0, a1, a2, b0 = 1, b1, b2, norm;
    double K = tan( 3.14159 * ( Fc * 1.0  / ( Fs ) ) );

    switch (type) {

    case LowPass:
        norm = 1 / (1 + K / Q + K * K);
        a0 = K * K * norm;
        a1 = 2 * a0;
        a2 = a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / Q + K * K) * norm;
        break;

    case Notch:
        norm = 1 / (1 + K / Q + K * K);
        a0 = (1 + K * K) * norm;
        a1 = 2 * (K * K - 1) * norm;
        a2 = a0;
        b1 = a1;
        b2 = (1 - K / Q + K * K) * norm;
        break;

    default:
        return;
    }

    poles[0] = static_cast<float>(b0);
    poles[1] = static_cast<float>(b1);
    poles[2] = static_cast<float>(b2);
    zeros[0] = static_cast<float>(a0);
    zeros[1] = static_cast<float>(a1);
    zeros[2] = static_cast<float>(a2);
}




