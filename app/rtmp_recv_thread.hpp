#ifndef RS_RECV_THREAD_HPP
#define RS_RECV_THREAD_HPP

#include <common/core.hpp>
#include <common/thread.hpp>
#include <protocol/rtmp_stack.hpp>
// #include <protocol/buffer.hpp>
#include <app/rtmp_connection.hpp>
#include <app/server.hpp>

class RTMPRecvThread : virtual public internal::IThreadHandler
{
public:
    RTMPRecvThread(rtmp::IMessageHander *message_handler, RTMPServer *rtmp, int32_t timeout);
    virtual ~RTMPRecvThread();

public:
    virtual int32_t GetID();
    virtual int32_t Start();
    virtual void Stop();
    virtual void StopLoop();

    // virtual void OnThreadStart() override;
    virtual void OnThreadStart() override;
    virtual int32_t Cycle() override;
    virtual void OnThreadStop() override;
private:
    internal::Thread *thread_;
    rtmp::IMessageHander *handler_;
    RTMPServer *rtmp_;
    int32_t timeout_;
};

class PublishRecvThread : virtual public rtmp::IMessageHander
{
public:
    PublishRecvThread();
    virtual ~PublishRecvThread();
};
#endif
