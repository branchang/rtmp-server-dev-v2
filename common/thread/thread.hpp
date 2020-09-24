#ifndef RS_THREAD_HPP
#define RS_THREAD_HPP
#include  <common/core.hpp>

namespace internal
{
class IThreadHandler
{
public:
    IThreadHandler();
    virtual ~IThreadHandler();

public:
    virtual void OnThreadStart();
    virtual int OnbeforeCycle();
    virtual int Cycle();
    virtual int OnEndCycle();
    virtual void OnThreadStop();
};

class SrsThread{
private:
    IThreadHandler *handler_;
    int64_t cycle_interval_us_;

};

}


#endif
