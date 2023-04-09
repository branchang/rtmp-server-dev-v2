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
    RTMPRecvThread(rtmp::IMessageHandler *message_handler, RTMPServer *rtmp, int32_t timeout);
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
    rtmp::IMessageHandler *handler_;
    RTMPServer *rtmp_;
    int32_t timeout_;
};

class PublishRecvThread : virtual public rtmp::IMessageHandler,
                          virtual public IMergeReadHandler,
                          virtual public IReloadHandler
{
public:
    PublishRecvThread(RTMPServer *rtmp,
                      rtmp::Request *request,
                      int mr_socket_fd,
                      int timeout_ms,
                      RTMPConnection *conn,
                      rtmp::Source *source,
                      bool is_fmle,
                      bool is_edge);
    virtual ~PublishRecvThread();

public:
    virtual int Wait(int timeout_ms);
    virtual int Start();
    virtual int Stop();
    virtual bool CanHandle();
    //rtmp::IMessageHandler
    virtual void OnThreadStart() override;
    virtual void OnThreadStop() override;
    virtual int Handle(rtmp::CommonMessage *msg) override;
    virtual void OnRecvError(int32_t ret) override;
    virtual void OnRead(ssize_t nread) override;

private:
    void set_socket_buffer(int sleep_ms);

private:
    RTMPRecvThread *thread_;
    RTMPServer *rtmp_;
    rtmp::Request *reqeust_;
    int64_t nb_msgs_;
    uint64_t video_frames_;
    bool mr_;
    int mr_fd_;
    int mr_sleep_;
    bool real_time_;
    int recv_error_code_;
    RTMPConnection *conn_;
    rtmp::Source *source_;
    bool is_fmle_;
    bool is_edge_;
    st_cond_t error_;
    int cid;
    int ncid;
};
#endif
