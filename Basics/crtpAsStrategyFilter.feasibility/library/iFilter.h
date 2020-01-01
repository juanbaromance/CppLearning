#ifndef CFilter_H
#define CFilter_H

#include <vector>
#include <string>
#include <bitset>

class iAcquisitionContext {
public:

    virtual void FrameUpdater(const std::vector<int> & filtered_channels, size_t _mask, const std::string & backtrace = "" ) = 0;
    virtual std::vector<int> valueOfSnapshot() = 0;
    virtual int valueOfSATick( const std::vector<float> & ) = 0;
    virtual int valueOfSAParams( const std::vector<int> & ) = 0;

    enum {
        IpPort = 0,
    };
    virtual const std::bitset<32>& operationWord() = 0;

public:
    static iAcquisitionContext *instance();
    virtual ~iAcquisitionContext(){}
};

template <typename T>
struct crtp {
    T& underly(){ return static_cast<T&>(*this); }
    T const& underly() const { return static_cast<T const&>(*this); }
};


template <typename T>
class iFilter : public crtp<T>
{
public:
    using measureT = float;
    using msecT = int;
    virtual float  step( float m ) = 0;
    virtual int    reset   ( measureT = 0, bool deep_reset = false ) = 0;
    virtual void   testing ( ) = 0;
    virtual float  state   ( ) = 0;
    virtual void   setSampling ( msecT );
    virtual ~iFilter(){}
};

template <typename T> float iFilter<T>::step( float m ){ return this->underly().step( m ); }
template <typename T> int   iFilter<T>::reset( measureT m, bool deep_reset ){ return this->underly().reset(m,deep_reset);}
template <typename T> void  iFilter<T>::testing ( ){ this->underly().testing(); }
template <typename T> float iFilter<T>::state( ){ return this->underly().state(); }
template <typename T> void  iFilter<T>::setSampling ( msecT msec){ this->underly().setSampling( msec );}

#endif
