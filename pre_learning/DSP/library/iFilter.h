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


class iFilter
{
public:
    using measureT = float;
    using msecT = int;

    virtual measureT step    ( measureT ) { return -1; };
    virtual int      reset   ( measureT = 0, bool deep_reset = false ) { return deep_reset; };
    virtual void     testing ( ) { };
    virtual measureT state   ( ) { return -1; };
    virtual void     setSampling ( msecT ){ };

    virtual ~iFilter(){}
};

#endif
