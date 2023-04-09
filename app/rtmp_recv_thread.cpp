#include <app/rtmp_recv_thread.hpp>
#include <protocol/rtmp_consts.hpp>
#include <common/utils.hpp>
#include <common/error.hpp>
#include <common/config.hpp>


RTMPRecvThread::RTMPRecvThread(rtmp::IMessageHander *handler,
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

PublishRecvThread::PublishRecvThread()
{

}

PublishRecvThread::~PublishRecvThread()
{

}
