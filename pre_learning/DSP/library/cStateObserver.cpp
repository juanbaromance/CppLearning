#include <variant>

#include "cStateObserver.h"

/*
 * @cT cT is related with the concrete class
 * @iT iT is related with the interface class // deprecated through crtp
 * @...Args are the cT constructor arguments
*/
template<class cT, class... Args>
auto factory( Args... args ){ return [ args... ] { return std::make_shared<cT>(args...); }(); }

cStateObserver::cStateObserver(std::string name, float Q, float R, std::vector<float> v ) : _name( name )
{
    std::shared_ptr<AlphaBetaObserver> w = factory<AlphaBetaObserver>( _name + ".AlphaBeta");
    w->settings( 0.1, 0.3, 0.1 );
    std::shared_ptr<ScalarKalman> sk = factory<ScalarKalman>( _name + ".SteadyKalman", Q, R );

    filters = std::make_tuple(
        factory<MedianFilter>( _name + ".Median", v.size(), v ),
        sk,
        w,
        factory<ExponentialSmoother>( _name + ".Exponential"));

    sk->iFilter<ScalarKalman>::testing();
}

void cStateObserver::settings(float, float, float){}

float cStateObserver::state( Numerology index )
{
    std::array<float,NumOfObservers> v = {
        std::get<0>(filters)->state(),
        std::get<1>(filters)->state(),
        std::get<2>(filters)->state(),
        std::get<3>(filters)->state()
    };
    try {
        return v.at( reduce(index) );
    } catch (...) {
        return -100000;
    }
}

#include <algorithm>
cStateObserver::ObserverState cStateObserver::sync( float xm, float )
{
    ObserverState tmp;
    std::apply( [ & tmp, xm ]( MedianView m, ScalarKalmanView skv, AlphaBetaView abv, ExponentialView ev )
               { tmp = { m->step(xm), skv->step(xm), abv->step(xm), ev->step(xm) }; }, filters );
    return tmp;
}

std::string cStateObserver::report(){ return std::string(); }
