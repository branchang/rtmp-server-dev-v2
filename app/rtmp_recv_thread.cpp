#include <app/rtmp_recv_thread.hpp>
#include <protocol/rtmp_consts.hpp>
#include <common/utils.hpp>
#include <common/error.hpp>
#include <common/config.hpp>


IMessageHandler::IMessageHandler()
{

}

IMessageHandler::~IMessageHandler()
{

}


RTMPRecvThread::RTMPRecvThread(IMessageHandler *handler,
                               RTMPServer *rtmp,
                               int32_t timeout) : handler_(handler),
                                                  rtmp_(rtmp),
                                                  timeout_(timeout)
{
    thread_ = new internal::Thread("recv", this, 0, true);
}

RTMPRecvThread::~RTMPRecvThread()
{
    Stop();
    rs_freep(thread_);
}

int32_t RTMPRecvThread::GetID()
{
    return thread_->GetID();
}

int32_t RTMPRecvThread::Start()
{
    return thread_->Start();
}

void RTMPRecvThread::Stop()
{
    thread_->Stop();
}

void RTMPRecvThread::StopLoop()
{
    thread_->StopLoop();
}

int32_t RTMPRecvThread::Cycle()
{
    int32_t ret = ERROR_SUCCESS;

    while(thread_->CanLoop())
    {
        if(!handler_->CanHandler())
        {
            st_usleep(timeout_ * 1000);
            continue;
        }
        rtmp::CommonMessage *msg = nullptr;
        if ((ret = rtmp_->RecvMessage(&msg)) != ERROR_SUCCESS)
        {
            if (!IsClientGracefullyClose(ret))
            {
                rs_error("thread process message failed, ret=%d", ret);
            }
            thread_->StopLoop();
            handler_->OnRecvError(ret);
            return ret;
        }
        else
        {
            handler_->Handle(msg);
        }
    }

    return ret;
}

void RTMPRecvThread::OnThreadStop()
{
    rtmp_->SetRecvTimeout(timeout_ * 1000);
    handler_->OnThreadStop();
}

void RTMPRecvThread::OnThreadStart()
{
    // set never timeout can impove 33% performance
    rtmp_->SetRecvTimeout(RTMP_ST_NO_TIMEOUT);
    handler_->OnThreadStart();
}

PublishRecvThread::PublishRecvThread(RTMPServer *rtmp,
                      rtmp::Request *request,
                      int mr_socket_fd,
                      int timeout_ms,
                      RTMPConnection *conn,
                      rtmp::Source *source,
                      bool is_fmle,
                      bool is_edge)
{
    thread_ = new RTMPRecvThread(this, rtmp, timeout_ms);
    rtmp_ = rtmp;
    reqeust_ = request;
    nb_msgs_ = 0;
    video_frames_ = 0;
    mr_ = _config->GetMREnabled(request->vhost);
    mr_fd_ = mr_socket_fd;
    mr_sleep_ = _config->GetMRSleepMS(request->vhost);
    real_time_ = _config->GetRealTimeEnabled(request->vhost);
    recv_error_code_ = ERROR_SUCCESS;
    conn_ = conn;
    source_ = source;
    is_fmle_ = is_fmle;
    is_edge_ = is_edge;
    error_ = st_cond_new();
    cid = 0;
    ncid = 0;

    _config->Subscribe(this);
}

PublishRecvThread::~PublishRecvThread()
{
    _config->UnSubScribe(this);
    thread_->Stop();
    rs_freep(thread_);
    st_cond_destroy(error_);
}

int PublishRecvThread::Wait(int timeout_ms)
{
    if (recv_error_code_ != ERROR_SUCCESS)
    {
        return recv_error_code_;
    }

    st_cond_timedwait(error_, timeout_ms * 1000);
    return ERROR_SUCCESS;
}

int PublishRecvThread::Start()
{
    int ret = thread_->Start();
    ncid = cid = thread_->GetID();
    return ret;
}

void PublishRecvThread::Stop()
{
    thread_->Stop();
}

void PublishRecvThread::set_socket_buffer(int sleep_ms)
{
    //bitreate 5000k
    int kbps = 5000;
    int socket_buffer_size = sleep_ms * kbps;

    int fd = mr_fd_;
    //old num of recv buffer
    int onb_rbuf = 0;
    socklen_t sock_buf_size = sizeof(int);
    ::getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &onb_rbuf, &sock_buf_size);

    //socket recv buffer, system will double it
    int nb_rbuf = socket_buffer_size / 2;
    if ((::setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &nb_rbuf, sock_buf_size)) < 0)
    {
        rs_error("set socket recv buffer failed");
    }
    ::getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &onb_rbuf, &sock_buf_size);

    rs_trace("mr change sleep %dms=>%dms, erbuf=%d, rbuf %d=%d", mr_sleep_, sleep_ms, socket_buffer_size, onb_rbuf, nb_rbuf);

    rtmp_->SetRecvBuffer(nb_rbuf);
}

void PublishRecvThread::OnThreadStart()
{
    if(mr_)
    {
        set_socket_buffer(mr_sleep_);
        rtmp_->SetMargeRead(mr_, this);
    }
}

void PublishRecvThread::OnThreadStop()
{
    st_cond_signal(error_);
    if (mr_)
    {
        rtmp_->SetMargeRead(false, nullptr);
    }

}

void PublishRecvThread::OnRead(ssize_t nread)
{
    if (!mr_ || real_time_)
    {
        return;
    }
    if (nread < 0 || mr_sleep_ <= 0)
    {
        return;
    }

    if (nread < RTMP_MR_SMALL_BYTES)
    {
        st_usleep(mr_sleep_ * 1000);
    }

}

int PublishRecvThread::Handle(rtmp::CommonMessage *msg)
{
    int ret = ERROR_SUCCESS;

    if (ncid != cid)
    {
        _context->SetID(ncid);
        cid = ncid;
    }
    nb_msgs_++;

    if (msg->header.IsVideo())
    {
        video_frames_++;
    }

    ret = conn_->handle_publish_message(source_, msg, is_fmle_, is_edge_);

    rs_freep(msg);
    return ret;
}

void PublishRecvThread::OnRecvError(int32_t ret)
{
   recv_error_code_ = ret;
   st_cond_signal(error_);
}

bool PublishRecvThread::CanHandler()
{
    return true;
}

int PublishRecvThread::GetCID()
{
    return ncid;
}

void PublishRecvThread::SetCID(int cid)
{
    ncid = cid;
}

int PublishRecvThread::ErrorCode()
{
    return recv_error_code_;
}

int64_t PublishRecvThread::GetMsgNum()
{
    return nb_msgs_;
}



QueueRecvThread::QueueRecvThread(rtmp::Consumer* consumer,
                                 RTMPServer* rtmp,
                                 int timeout_ms)
{
    consumer_ = consumer;
    rtmp_ = rtmp;
    thread_ = new RTMPRecvThread(this, rtmp, timeout_ms);
    recv_error_code_ = ERROR_SUCCESS;
}

QueueRecvThread::~QueueRecvThread()
{
    Stop();
    //clear queue
    std::vector<rtmp::CommonMessage*>::iterator it;
    for (it = queue_.begin(); it != queue_.end(); it++)
    {
        rtmp::CommonMessage* msg = *it;
        rs_freep(msg);
    }
    rs_freep(thread_);
}

int QueueRecvThread::Start()
{
    return thread_->Start();
}

void QueueRecvThread::Stop()
{
    return thread_->Stop();
}

bool QueueRecvThread::Empty()
{
    return queue_.empty();
}

int QueueRecvThread::Size()
{
    return (int)queue_.size();
}

rtmp::CommonMessage* QueueRecvThread::Pump()
{
    rtmp::CommonMessage* msg = *queue_.begin();
    queue_.erase(queue_.begin());
    return msg;
}

int QueueRecvThread::ErrorCode()
{
    return recv_error_code_;
}

bool QueueRecvThread::CanHandler()
{
    return queue_.empty();
}

int QueueRecvThread::Handle(rtmp::CommonMessage* msg)
{
    queue_.push_back(msg);
    if (consumer_) {
        consumer_->WakeUp();
    }

    return ERROR_SUCCESS;
}

void QueueRecvThread::OnThreadStart()
{
    rtmp_->SetAutoResponse(true);
}

void QueueRecvThread::OnThreadStop()
{
    rtmp_->SetAutoResponse(false);
}

void QueueRecvThread::OnRecvError(int32_t ret)
{
    recv_error_code_ = ret;
    if (consumer_) {
        consumer_->WakeUp();
    }
}
