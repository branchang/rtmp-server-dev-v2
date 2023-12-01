#include <protocol/rtmp_source.hpp>
#include <protocol/rtmp_message.hpp>
#include <common/error.hpp>
#include <protocol/rtmp_consumer.hpp>

namespace rtmp
{

IWakeable::IWakeable()
{
}

IWakeable::~IWakeable()
{
}

Consumer::Consumer(Source *s, Connection *c)
{
    source_ = s;
    conn_ = c;
    jitter_ = new Jitter;
    queue_ = new MessageQueue;
    mw_wait_ = st_cond_new();
    mw_waiting_ = false;
    mw_min_msgs_ = 0;
    mw_duration_ = 0;
}

Consumer::~Consumer()
{
    source_->OnConsumerDestory(this);
    rs_freep(jitter_);
    rs_freep(queue_);
    st_cond_destroy(mw_wait_);
}

void Consumer::SetQueueSize(double second)
{
    queue_->SetQueueSize(second);
}

int Consumer::GetTime()
{
    return jitter_->GetTime();
}

int Consumer::Enqueue(SharedPtrMessage *shared_msg, bool atc, JitterAlgorithm ag)
{
    int ret = ERROR_SUCCESS;

    SharedPtrMessage *msg = shared_msg->Copy();

    if (!atc)
    {
        if ((ret = jitter_->Correct(msg, ag)) != ERROR_SUCCESS)
        {
            rs_freep(msg);
            return ret;
        }
    }

    if ((ret = queue_->Enqueue(msg, nullptr)) != ERROR_SUCCESS)
    {
        return ret;
    }

    if (mw_wait_)
    {
        int duration_ms = queue_->Duration();
        bool match_min_msgs = queue_->Size() - mw_min_msgs_;

        //for atc,maybe the sequeue header timestamp bigger than AV packet
        //when encode republish or overflow
        if (atc && duration_ms < 0)
        {
            st_cond_signal(mw_wait_);
            mw_waiting_ = false;
            return ret;
        }

        if (match_min_msgs && duration_ms > mw_duration_)
        {
            st_cond_signal(mw_wait_);
            mw_waiting_ = false;
            return ret;
        }
    }

    return ret;
}

int Consumer::DumpPackets(MessageArray *msg_arr, int &count)
{
    int ret = ERROR_SUCCESS;
    int max = count ? rs_min(count, msg_arr->max) : msg_arr->max;

    count = 0;

    if (should_update_source_id_)
    {
        should_update_source_id_ = false;
        rs_trace("update source_id=%d", source_->SourceId());
    }
    if (pause_)
    {
        return ret;
    }

    if ((ret  = queue_->DumpPackets(max, msg_arr->msgs, count)) != ERROR_SUCCESS)
    {
        return ret;
    }

    return ret;
}

void Consumer::Wait(int nb_msgs, int duration)
{
    if (pause_)
    {
        // st_usleep(RTMP_PULSE_TIMEOUT_US);
        st_usleep(-1);
        return;
    }

    mw_min_msgs_ = nb_msgs;
    mw_duration_ = duration;

    int duration_ms = queue_->Duration();
    bool match_min_msgs = queue_->Size() > mw_min_msgs_;

    if (match_min_msgs && duration_ms > mw_duration_)
    {
        return;
    }

    mw_waiting_ = true;
    st_cond_wait(mw_wait_);
}

int Consumer::OnPlayClientPause(bool is_pause)
{
    int ret = ERROR_SUCCESS;

    pause_ = is_pause;

    return ret;
}

void Consumer::WakeUp()
{

    if (mw_waiting_)
    {
        st_cond_signal(mw_wait_);
        mw_waiting_ = false;
    }
}

void Consumer::UpdateSourceId()
{
    should_update_source_id_ = true;
}

}
