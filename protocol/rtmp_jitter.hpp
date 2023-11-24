#ifndef RS_RTMP_JITTER_HPP
#define RS_RTMP_JITTER_HPP

#include <common/core.hpp>

namespace rtmp
{

class SharedPtrMessage;

enum class JitterAlgorithm
{
    FULL = 1,
    ZERO,
    OFF
};

class Jitter
{
public:
    Jitter();
    virtual ~Jitter();
public:
    virtual int Correct(SharedPtrMessage *msg, JitterAlgorithm ag);
    virtual int GetTime();
private:
    int64_t last_pkt_time_;
    int64_t last_pkt_correct_time_;
};

}

#endif
