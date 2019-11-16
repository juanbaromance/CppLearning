#pragma once

#include <string>
#include <array>
#include <memory>
#include <tuple>
#include "Observers.h"

class cStateObserver {
public:
    enum class Numerology {
        MA_MEDIAN_F,
        KALMAN_DISCRETE_F,
        ALPHA_BETA_F,
        EXPONENTIAL_F,
        Dummy
    };
    static constexpr int NumOfObservers = static_cast<int>(Numerology::Dummy);

public:
    cStateObserver( std::string name = "unknown", float Q = 0.001, float R = 0.1, std::vector<float> v = { 0.05, 0.10, 0.7, 0.10, 0.05 } );
    void  settings( float Q, float R, float sampling = 0.1 );
    float state( enum Numerology index );

    using ObserverState = std::array<float, NumOfObservers>;
    ObserverState sync( float xm, float = 0.1);
    std::string report();

private:
    constexpr size_t reduce( Numerology index ){ return static_cast<size_t>( index ); }
    // std::array< std::shared_ptr<iFilter>, NumOfObservers> filters;
    std::string _name;
    using MedianView       = std::shared_ptr<iFilter<MedianFilter>>;
    using ScalarKalmanView = std::shared_ptr<iFilter<ScalarKalman>>;
    using AlphaBetaView    = std::shared_ptr<iFilter<AlphaBetaObserver>>;
    using ExponentialView  = std::shared_ptr<iFilter<ExponentialSmoother>>;
    std::tuple<MedianView,ScalarKalmanView,AlphaBetaView,ExponentialView> filters;
};

