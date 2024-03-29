#include <protocol/rtmp_message.hpp>
#include <protocol/rtmp_consts.hpp>
#include <common/utils.hpp>
#include <common/log.hpp>
#include <common/error.hpp>
#include <muxer/flv.hpp>

namespace rtmp
{

int chunk_header_c0(int perfer_cid, uint32_t timestamp, int32_t payload_length,
                            int8_t message_type, int32_t stream_id, char *buf)
{
    char *pp = nullptr;
    char *p = buf;

    *p++ = 0x00 | (0x3f & perfer_cid);
    if (timestamp < RTMP_EXTENDED_TIMESTAMPE)
    {
        pp = (char *)&timestamp;
        *p++ = pp[2];
        *p++ = pp[1];
        *p++ = pp[0];
    }
    else{
        *p++ = 0xff;
        *p++ = 0xff;
        *p++ = 0xff;
    }

    pp = (char *)&payload_length;
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];

    *p++ = message_type;

    // little-endian
    pp = (char *)&stream_id;
    *p++ = pp[0];
    *p++ = pp[1];
    *p++ = pp[2];
    *p++ = pp[3];

    if (timestamp >= RTMP_EXTENDED_TIMESTAMPE)
    {
        pp = (char *)&timestamp;
        *p++ = pp[3];
        *p++ = pp[2];
        *p++ = pp[1];
        *p++ = pp[0];
    }

    return p - buf;
}

int chunk_header_c3(int perfer_cid, uint32_t timestamp, char *buf)
{
    char *pp = nullptr;
    char *p = buf;

    *p++ = 0xC0 | (0x3f & perfer_cid);
    if (timestamp >= RTMP_EXTENDED_TIMESTAMPE)
    {
        pp = (char *)&timestamp;
        *p++ = pp[3];
        *p++ = pp[2];
        *p++ = pp[1];
        *p++ = pp[0];
    }
    return p - buf;
}

MessageHeader::MessageHeader()
{
    timestamp_delta = 0;
    payload_length = 0;
    message_type = 0;
    stream_id = 0;
    timestamp = 0;
    perfer_cid = 0;
}

MessageHeader::~MessageHeader()
{

}


bool MessageHeader::IsAudio()
{
    if (message_type == RTMP_MSG_AUDIO_MESSAGE)
        return true;
    return false;
}

bool MessageHeader::IsVideo()
{
    if (message_type == RTMP_MSG_VIDEO_MESSAGE)
        return true;
    return false;
}

bool MessageHeader::IsAMF0Command()
{
    if (message_type == RTMP_MSG_AMF0_COMMAND)
        return true;
    return false;
}

bool MessageHeader::IsAMF0Data()
{
    if (message_type == RTMP_MSG_AMF0_DATA)
        return true;
    return false;
}

bool MessageHeader::IsAMF3Command()
{
    if (message_type == RTMP_MSG_AMF3_COMMAND)
        return true;
    return false;
}

bool MessageHeader::IsAMF3Data()
{
    if (message_type == RTMP_MSG_AMF3_DATA)
        return true;
    return false;
}

bool MessageHeader::IsWindowAckledgementSize()
{
    if (message_type == RTMP_MSG_WINDOW_ACK_SIZE)
        return true;
    return false;
}

bool MessageHeader::IsAckledgememt()
{
    if (message_type == RTMP_MSG_ACK)
        return true;
    return false;
}

bool MessageHeader::IsSetChunkSize()
{
    if (message_type == RTMP_MSG_SET_CHUNK_SIZE)
        return true;
    return false;
}

bool MessageHeader::IsUserControlMessage()
{
    if (message_type == RTMP_MSG_USER_CONTROL_MESSAGE)
        return true;
    return true;
}

bool MessageHeader::IsSetPeerBandWidth()
{
    if (message_type == RTMP_MSG_SET_PERR_BANDWIDTH)
        return true;
    return true;
}

bool MessageHeader::IsAggregate()
{
    if (message_type == RTMP_MSG_AGGREGATE)
        return true;
    return true;
}

void MessageHeader::InitializeAMF0Script(int32_t size, int32_t stream)
{
    message_type = RTMP_MSG_AMF0_DATA;
    payload_length = size;
    timestamp_delta = 0;
    timestamp = 0;
    stream_id = stream;
    perfer_cid = RTMP_CID_OVER_CONNECTION2;

}

void MessageHeader::InitializeVideo(int32_t size, uint32_t time, int32_t stream)
{
    message_type = RTMP_MSG_VIDEO_MESSAGE;
    payload_length = size;
    timestamp_delta = time;
    timestamp = time;
    stream_id = stream;
    perfer_cid = RTMP_CID_VIDEO;
}

void MessageHeader::InitializeAudio(int32_t size, uint32_t time, int32_t stream)
{
    message_type = RTMP_MSG_AUDIO_MESSAGE;
    payload_length = size;
    timestamp_delta = time;
    timestamp = time;
    stream_id = stream;
    perfer_cid = RTMP_CID_AUDIO;
}


CommonMessage::CommonMessage() : size(0),
                                 payload(nullptr)
{

}

CommonMessage::~CommonMessage()
{
    rs_freepa(payload);
    payload = new char[size];
    rs_verbose("create payload for rtmp message, size=%d", size);
}

void CommonMessage::CreatePlayload(int32_t size)
{
    rs_freepa(payload);
    payload = new char[size];
    rs_verbose("create payload for rtmp message,size=%d", size);
}

ChunkStream::ChunkStream(int cid) : cid(cid),
                                    fmt(0),
                                    msg(nullptr),
                                    extended_timestamp(false),
                                    msg_count(0)
{

}

ChunkStream::~ChunkStream()
{

}

SharedPtrMessage::SharedPtrMessage() : ptr_(nullptr)
{

}

SharedPtrMessage::~SharedPtrMessage()
{
    if(ptr_)
    {
        if (ptr_->shared_count == 0)
        {
            rs_freep(ptr_);
        }
        else
        {
            ptr_->shared_count--;
        }
    }
}

SharedPtrMessage::SharedPtrPayload::SharedPtrPayload() : payload(nullptr),
                                                        size(0),
                                                        shared_count(0)
{

}

SharedPtrMessage::SharedPtrPayload::~SharedPtrPayload()
{
    rs_freep(payload);
}

int SharedPtrMessage::Create(MessageHeader *pheader, char *payload, int size)
{
    int ret = ERROR_SUCCESS;

    if (ptr_)
    {
        ret = ERROR_SYSTEM_ASSERT_FAILED;
        rs_error("couldn't set payload twice, ret=%d", ret);
        rs_assert(false);
    }
    ptr_ = new SharedPtrPayload;
    if (pheader)
    {
        ptr_->header.message_type = pheader->message_type;
        // ptr_->header.payload_length = pheader->payload_length;
        ptr_->header.payload_length = size;
        ptr_->header.perfer_cid = pheader->perfer_cid;
        this->timestamp = pheader->timestamp;
        this->stream_id = pheader->stream_id;
    }

    ptr_->payload = payload;
    ptr_->size = size;

    this->payload = ptr_->payload;
    this->size = ptr_->size;

    return ret;
}

int SharedPtrMessage::Create(CommonMessage *msg)
{
    int ret = ERROR_SUCCESS;

    if ((ret = Create(&msg->header, msg->payload, msg->size)))
    {
        return ret;
    }
    msg->payload = nullptr;
    msg->size = 0;

    return ret;
}

int SharedPtrMessage::Count()
{
    return ptr_->shared_count;
}

bool SharedPtrMessage::Check(int stream_id)
{
    if (ptr_->header.perfer_cid < 2 || ptr_->header.perfer_cid > 63)
    {
        rs_info("change the chunk_id=%d to default=%d", ptr_->header.perfer_cid, RTMP_CID_PROTOCOL_CONTROL);
        ptr_->header.perfer_cid = RTMP_CID_PROTOCOL_CONTROL;
    }
    if (this->stream_id == stream_id)
    {
        return true;
    }

    this->stream_id = stream_id;

    return false;
}

bool SharedPtrMessage::IsAV()
{
    return ptr_->header.message_type == RTMP_MSG_AUDIO_MESSAGE ||
           ptr_->header.message_type == RTMP_MSG_VIDEO_MESSAGE;
}

bool SharedPtrMessage::IsAudio()
{
    return ptr_->header.message_type == RTMP_MSG_AUDIO_MESSAGE;
}

bool SharedPtrMessage::IsVideo()
{
    return ptr_->header.message_type == RTMP_MSG_VIDEO_MESSAGE;
}

int SharedPtrMessage::ChunkHeader(char *buf, bool c0)
{
    if (c0)
    {
        return chunk_header_c0(ptr_->header.perfer_cid, timestamp, ptr_->header.payload_length, ptr_->header.message_type, stream_id, buf);
    }
    else
    {
        return chunk_header_c3(ptr_->header.perfer_cid, timestamp, buf);

    }
}

SharedPtrMessage *SharedPtrMessage::Copy()
{
    SharedPtrMessage *copy = new SharedPtrMessage;
    copy->ptr_ = ptr_;
    ptr_->shared_count++;

    copy->timestamp = timestamp;
    copy->stream_id = stream_id;
    copy->payload = ptr_->payload;
    copy->size = ptr_->size;
    return copy;
}

MessageArray::MessageArray(int max_msgs)
{
    // TODO
    msgs = new SharedPtrMessage *[max_msgs];
    max = max_msgs;
    Zero(max_msgs);
}

MessageArray::~MessageArray()
{
    rs_freep(msgs);
}

void MessageArray::Free(int count)
{
    for (int i = 0; i < count; i++)
    {
        SharedPtrMessage *msg = msgs[i];
        rs_freep(msg);
        msgs[i] = nullptr;
    }

}

void MessageArray::Zero(int count)
{
    for (int i = 0;i < count; i++)
    {
        msgs[i] = nullptr;
    }
}


MessageQueue::MessageQueue()
{
    av_start_time_ = -1;
    av_end_time_ = -1;
    queue_size_ms_ = 0;
}

MessageQueue::~MessageQueue()
{
    Clear();
}

int MessageQueue::Size()
{
    return msgs_.Size();
}

int MessageQueue::Duration()
{
    return (int)(av_end_time_ - av_start_time_);
}

void MessageQueue::SetQueueSize(double second)
{
    queue_size_ms_ = (int)(second * 1000);
}

void MessageQueue::Shrink()
{
    SharedPtrMessage *video_sh = nullptr;
    SharedPtrMessage *audio_sh = nullptr;

    int msgs_size = msgs_.Size();
    for (int i=0;i<msgs_size;i++)
    {
        SharedPtrMessage *msg = msgs_.At(i);
        if (msg->IsAudio() && FlvDemuxer::IsAudioSeqenceHeader(msg->payload, msg->size))
        {
            rs_freep(audio_sh);
            audio_sh = msg;
            continue;
        }
        if (msg->IsAudio() && FlvDemuxer::IsVideoSeqenceHeader(msg->payload, msg->size))
        {
            rs_freep(video_sh);
            video_sh = msg;
            continue;
        }
        rs_freep(msg);
    }
    msgs_.Clear();

    av_start_time_ = av_end_time_;
    if(audio_sh)
    {
        audio_sh->timestamp = av_end_time_;
        msgs_.PushBack(audio_sh);
    }
    if(video_sh)
    {
        video_sh->timestamp = av_end_time_;
        msgs_.PushBack(video_sh);
    }
}

int MessageQueue::Enqueue(SharedPtrMessage *msg, bool *is_overflow)
{
    int ret = ERROR_SUCCESS;

    if (msg->IsAV())
    {
        if (av_end_time_ == -1)
        {
            av_end_time_ = msg->timestamp;
        }
        av_end_time_ = msg->timestamp;
    }
    msgs_.PushBack(msg);
    while (av_end_time_ - av_start_time_ > queue_size_ms_)
    {
        if (is_overflow)
        {
            *is_overflow = true;
        }
        Shrink();
    }

    return ret;
}

void MessageQueue::Clear()
{
    msgs_.Free();
    av_start_time_ = av_end_time_ = -1;
}

int MessageQueue::DumpPackets(int max_count, SharedPtrMessage **pmsgs, int &count)
{
    int ret = ERROR_SUCCESS;

    int nb_msgs = msgs_.Size();
    if (nb_msgs <= 0)
    {
        return ret;
    }
    count = rs_min(nb_msgs, max_count);

    SharedPtrMessage **omsgs = msgs_.Data();
    for(int i=0;i<count;i++)
    {
        pmsgs[i] = omsgs[i];
    }

    SharedPtrMessage *last = omsgs[count-1];
    av_start_time_ = last->timestamp;
    if (count>nb_msgs)
    {
        msgs_.Clear();
    }
    else
    {
        msgs_.Erase(msgs_.Begin(), msgs_.Begin() + count);
    }
    return ret;
}

int MessageQueue::DumpPackets(Consumer *consumer, bool atc ,JitterAlgorithm ag)
{
    int ret = ERROR_SUCCESS;
    return ret;
}

}
