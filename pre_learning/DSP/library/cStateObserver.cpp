#include "cStateObserver.h"
#include <variant>

constexpr float _weights[]= { 0.05, 0.10, 0.7, 0.10, 0.05 };


cStateObserver::cStateObserver(string name, float Q, float R, int window ) : _name( name )
{
    filters[ reduce(Numerology::MA_MEDIAN_F)       ] = ( make_shared<iFilter>( MedianFilter      ( _name + ".Median", window, vector <float>( _weights, _weights + 2 ))));
    filters[ reduce(Numerology::KALMAN_DISCRETE_F) ] = ( make_shared<iFilter>( ScalarKalman      ( _name + ".SteadyKalman", Q, R ) ));
    filters[ reduce(Numerology::ALPHA_BETA_F)      ] = ( make_shared<iFilter>( AlphaBetaObserver ( _name + ".AlphaBeta", Q, R )));
}

void cStateObserver::settings(float, float, float){}

float cStateObserver::state( Numerology index )
{
    try {
        return filters.at( reduce( index ) )->state();
    } catch (...) {
        return -100000;
    }
}

#include <algorithm>
cStateObserver::ObserverState cStateObserver::sync(float xm, float )
{
    ObserverState tmp;
    std::transform ( filters.begin(), filters.end(), tmp.begin(),
                   [=]( shared_ptr<iFilter> & f) -> float { return f->step( xm ); });

    return tmp;
}

string cStateObserver::report(){ return string(); }
