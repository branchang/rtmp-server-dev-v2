#ifndef RS_RTMP_SOURCE_HPP
#define RS_RTMP_SOURCE_HPP
#include <common/core.hpp>
#include <common/connection.hpp>
#include <protocol/rtmp_stack.hpp>

namespace rtmp
{

enum class JitterAlgorithm
{
    FULL = 1,
    ZERO,
    OFF
};

class Source;
class MessageQueue;

class ISourceHandler
{
public:
    ISourceHandler();
    virtual ~ISourceHandler();
public:
    virtual int OnPublish(Source *s, Request *r) = 0;
    virtual int OnUnPublish(Source *s, Request *r) = 0;
};

class IWakeable
{
public:
    IWakeable();
    virtual ~IWakeable();

public:
    virtual void WakeUp() = 0;
};

class FastVector
{
public:
    FastVector();
    virtual ~FastVector();

public:
    virtual int Size();
    virtual int Begin();
    virtual int End();
    virtual SharedPtrMessage **Data();
    virtual SharedPtrMessage *At(int index);
    virtual void Clear();
    virtual void Free();
    virtual void Erase(int begin, int end);
    virtual void PushBack(SharedPtrMessage *msg);

private:
    SharedPtrMessage **msgs_;
    int nb_msgs_;
    int count_;
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

class Consumer : public IWakeable
{
public:
    Consumer(Source *s, Connection *c);
    virtual ~Consumer();

public:
    virtual void SetQueueSize(double queue_size);
    virtual int GetTime();
    virtual int Enqueue(SharedPtrMessage *shared_msg, bool atc, JitterAlgorithm ag);
    virtual int DumpPackets(MessageArray *msg_arr, int &count);
    virtual void Wait(int nb_msgs, int duration);
    virtual int OnPlayClientPause(bool is_pause);
    //IWakeable
    virtual void WakeUp() override;
private:
    Source *source_;
    Connection *conn_;
    bool pause_;
    Jitter *jitter_;
    MessageQueue *queue_;
    st_cond_t mw_wait_;
    bool mw_waiting_;
    int mw_min_msgs_;
    int mw_duration_;
};

class MessageQueue
{
public:
    MessageQueue();
    virtual ~MessageQueue();

public:
    virtual int Size();
    virtual int Duration();
    virtual void SetQueueSize(double second);
    virtual int Enqueue(SharedPtrMessage *msg, bool *is_overflow = nullptr);
    virtual int DumpPackets(int max_count, SharedPtrMessage **pmsgs, int &count);
    virtual int DumpPackets(Consumer *consumer, bool atc ,JitterAlgorithm ag);

protected:
    virtual void Shrink();
    virtual void Clear();

private:
    int64_t av_start_time_;
    int64_t av_end_time_;
    int64_t queue_size_ms_;
    FastVector msgs_;
};

class Source
{
public:
    Source();
    virtual ~Source();
public:
    static int FetchOrCreate(Request *r, ISourceHandler *h, Source **pps);
    int Initialize(Request *r, ISourceHandler *h);
    bool CanPublish(bool is_edge);
    void OnConsumerDestory(Consumer *consumer);
protected:
    static Source *Fetch(Request *r);

private:
    static std::map<std::string, Source *> pool_;
    Request *request_;
    bool atc_;
    ISourceHandler *handler_;
    bool can_publish_;
};

} //namespace rtmp

#endif
