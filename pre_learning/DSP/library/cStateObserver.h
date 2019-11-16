#pragma once

#include <string>
#include <array>
#include <memory>

#include "Observers.h"
using namespace std;

class cStateObserver {
public:
    enum class Numerology {
        KALMAN_DISCRETE_F,
        ALPHA_BETA_F,
        MA_MEDIAN_F,
        Dummy
    };
    static constexpr int NumOfObservers = static_cast<int>(Numerology::Dummy);

public:
    cStateObserver( string name = "unknown", float Q = 0.005, float R = 0.1, int window = 5 );
    void  settings( float Q, float R, float sampling = 0.1 );
    float state( enum Numerology index );

    using ObserverState = std::array<float, NumOfObservers>;
    ObserverState sync( float xm, float = 0.1);
    std::string report();

private:
    constexpr size_t reduce( Numerology index ){ return static_cast<size_t>( index ); }
    std::array< shared_ptr<iFilter>, NumOfObservers> filters;
    std::string _name;
    int _window;
};

