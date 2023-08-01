#ifndef RS_RTMP_MESSAGE_HPP
#define RS_RTMP_MESSAGE_HPP

#include <common/core.hpp>

namespace rtmp
{

extern int chunk_header_c0(int perfer_cid, uint32_t timestamp, int32_t payload_length,
                            int8_t message_type, int32_t stream_id, char *buf);
extern int chunk_header_c3(int perfer_cid, uint32_t timestamp, char *buf);

class MessageHeader
{
public:
    int32_t timestamp_delta;
    int32_t payload_length;
    int8_t message_type;
    int32_t stream_id;
    int64_t timestamp;
    int32_t perfer_cid;

public:
    MessageHeader();
    virtual ~MessageHeader();

public:
    bool IsAudio();
    bool IsVideo();
    bool IsAMF0Command();
    bool IsAMF0Data();
    bool IsAMF3Command();
    bool IsAMF3Data();
    bool IsWindowAckledgementSize();
    bool IsAckledgememt();
    bool IsSetChunkSize();
    bool IsUserControlMessage();
    bool IsSetPeerBandWidth();
    bool IsAggregate();

    void InitializeAMF0Script(int32_t size, int32_t stream);
    void InitializeVideo(int32_t size, uint32_t time, int32_t stream);
    void InitializeAudio(int32_t size, uint32_t time, int32_t stream);
};


class CommonMessage
{
public:
    CommonMessage();
    virtual ~CommonMessage();
public:
    virtual void CreatePlayload(int32_t size);
public:
    int32_t size;
    char *payload;
    MessageHeader header;
};

struct SharedMesageHeader
{
    int32_t payload_length;
    int8_t message_type;
    int perfer_cid;
};

class SharedPtrMessage
{
public:
    SharedPtrMessage();
    virtual ~SharedPtrMessage();

public:
    virtual int Create(CommonMessage *msg);
    virtual int Create(MessageHeader *pheader, char *payload, int size);
    virtual int Count();
    virtual bool Check(int stream_id);
    virtual bool IsAV();
    virtual bool IsAudio();
    virtual bool IsVideo();
    virtual int ChunkHeader(char *buf, bool c0);
    virtual SharedPtrMessage *Copy();

private:
    class SharedPtrPayload
    {
    public:
        SharedPtrPayload();
        virtual ~SharedPtrPayload();
    public:
        SharedMesageHeader header;
        char *payload;
        int size;
        int shared_count;
    };
public:
    int64_t timestamp;
    int32_t stream_id;
    int size;
    char *payload;

private:
    SharedPtrPayload *ptr_;
};

class MessageArray
{
public:
    MessageArray(int max_msgs);
    virtual ~MessageArray();

public:
    virtual void Free(int count);
    virtual void Zero(int count);

public:
    SharedPtrMessage **msgs;
    int max;
};

}

#endif
