#include <protocol/rtmp_packet.hpp>
#include <protocol/rtmp_consts.hpp>
#include <common/utils.hpp>
#include <common/error.hpp>
#include <common/log.hpp>


namespace rtmp
{
Packet::Packet()
{

}

Packet::~Packet()
{
}

int Packet::GetPreferCID()
{
    return 0;
}

int Packet::GetMessageType()
{
    return 0;
}

int Packet::GetSize()
{
    return 0;
}

int Packet::Encode(int &psize, char*& ppayload)
{
    int ret = ERROR_SUCCESS;

    int size = GetSize();
    char *payload = nullptr;

    BufferManager manager;

    if (size > 0)
    {
        payload = new char[size];
        if ((ret = manager.Initialize(payload, size)) != ERROR_SUCCESS)
        {
            rs_error("initialize buffer manager failed, ret=%d", ret);
            rs_freepa(payload);
            return ret;
        }
    }

    if ((ret = EncodePacket(&manager)) != ERROR_SUCCESS)
    {
        rs_error("encode the package failed, ret=%d", ret);
        rs_freepa(payload);
        return ret;
    }

    psize = size;
    ppayload = payload;
    rs_verbose("encode the packet success, size=%d", size);

    return ret;
}

int Packet::Decode(BufferManager *manager)
{
    return ERROR_SUCCESS;
}

int Packet::EncodePacket(BufferManager *manager)
{
    return ERROR_SUCCESS;
}

SetChunkSizePacket::SetChunkSizePacket() : chunk_size(RTMP_DEFALUT_CHUNK_SIZE)
{

}

SetChunkSizePacket::~SetChunkSizePacket()
{

}

int SetChunkSizePacket::Decode(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;

    if (!manager->Require(4))
    {
        ret = ERROR_RTMP_MESSAGE_DECODE;
        rs_error("decode chunk size failed,ret=%d", ret);
        return ret;
    }

    chunk_size = manager->Read4Bytes();
    rs_verbose("decode chunk size success, chunk_size=%d", chunk_size);
    return ret;
}

int SetChunkSizePacket::GetPreferCID()
{
    return RTMP_CID_PROTOCOL_CONTROL;
}

int SetChunkSizePacket::GetMessageType()
{
    return RTMP_MSG_SET_CHUNK_SIZE;
}

int SetChunkSizePacket::GetSize()
{
    return 4;
}

int SetChunkSizePacket::EncodePacket(BufferManager *manager)
{

    int ret = ERROR_SUCCESS;
    if (!manager->Require(4))
    {
        ret = ERROR_RTMP_MESSAGE_ENCODE;
        rs_error("encode setchunk packet failed,ret =%d", ret);
    }

    manager->Write4Bytes(chunk_size);
    rs_verbose("encode setchunk packet succes, chunk_size=%d", chunk_size);
    return ret;
}

ConnectAppPacket::ConnectAppPacket(): command_name(RTMP_AMF0_COMMAND_CONNECT),
                                      transaction_id(1),
                                      args(nullptr)
{
    command_object = AMF0Any::Object();
}

ConnectAppPacket::~ConnectAppPacket()
{
    rs_freep(command_object);
    rs_freep(args);
}

int ConnectAppPacket::Decode(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    if ((ret = AMF0ReadString(manager, command_name)) != ERROR_SUCCESS)
    {
        rs_error("amf0 decode connect command_name failed,ret=%d", ret);
        return ret;
    }

    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_CONNECT)
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("amf0 decode connect command_name failed, command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }

    if ((ret = AMF0ReadNumber(manager, transaction_id)) != ERROR_SUCCESS)
    {
        rs_error("amf0 decode connect transaction_id failed, ret=%d" ,ret);
        return ret;
    }

    if (transaction_id != 1.0)
    {
        // beacause some client don't send transcode_id=1.0, only warn them
        rs_warn("amf0 decode connect transaction_id incorrect, transaction_id:%.2f, required:%.2f", 1.0, transaction_id);
    }
    {
        AMF0Any *p = nullptr;
        if ((ret=AMF0ReadAny(manager, &p)) != ERROR_SUCCESS)
        {
            rs_freep(args);
            rs_error("amf0 decode connect args failed, ret=%d",ret);
            return ret;
        }

        if (!p->IsObject())
        {
            rs_warn("drop connect args,marker=%#x", p->marker);
            rs_freep(p);
            ret = ERROR_PROTOCOL_AMF0_DECODE;
            rs_error("decode connect_app packet: amf0 read object failed.ret=%d", ret);
            return ret;
        }
        else
        {
            rs_freep(command_object);
            command_object = p->ToObject();
        }
    }

    AMF0Any *p = nullptr;
    if (!manager->Empty())
    {
        if ((ret=AMF0ReadAny(manager, &p)) != ERROR_SUCCESS)
        {
            rs_freep(args);
            rs_error("amf0 decode connect args failed, ret=%d",ret);
            return ret;
        }

        if (!p->IsObject())
        {
            rs_warn("drop connect args,marker=%#x", p->marker);
            rs_freep(p);
        }
        else{
            rs_freep(args);
            args = p->ToObject();
            // rs_freep(command_object);
            // command_object = p->ToObject();
        }
    }
    rs_info("amf0 decode connect request success");

    return ret;
}

int ConnectAppPacket::GetPreferCID()
{
    return RTMP_CID_OVER_CONNECTION;
}

int ConnectAppPacket::GetMessageType()
{
    return RTMP_MSG_AMF0_COMMAND;
}

int ConnectAppPacket::GetSize()
{
    int size = 0;
    size += AMF0_LEN_STR(command_name);
    size += AMF0_LEN_NUMBER;
    size += AMF0_LEN_OBJECT(command_object);
    if (args)
    {
        size += AMF0_LEN_OBJECT(command_object);
    }
    rs_verbose("encode ConnectApp packet succes, size=%d", size);
    return size;
}

int ConnectAppPacket::EncodePacket(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    if ((ret = AMF0WriteString(manager, command_name)) != ERROR_SUCCESS)
    {
        rs_error("amf0 encode connect command_name failed,ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0WriteNumber(manager, transaction_id)) != ERROR_SUCCESS)
    {
        rs_error("amf0 encode connect transaction_id failed,ret=%d", ret);
        return ret;
    }

    if ((ret = command_object->Write(manager)) != ERROR_SUCCESS)
    {
        rs_error("amf0 encode connect command_object failed,ret=%d", ret);
        return ret;
    }

    if ((ret = args->Write(manager)) != ERROR_SUCCESS)
    {
        rs_error("amf0 encode connect args failed,ret=%d", ret);
        return ret;
    }

    return ret;
}

ConnectAppResPacket::ConnectAppResPacket() : command_name(RTMP_AMF0_COMMAND_RESULT),
                                             transaction_id(1),
                                             props(AMF0Any::Object()),
                                             info(AMF0Any::Object())
{

}

ConnectAppResPacket::~ConnectAppResPacket()
{
    rs_freep(props);
    rs_freep(info);
}

int ConnectAppResPacket::GetPreferCID()
{
    return RTMP_CID_PROTOCOL_CONTROL;
}

int ConnectAppResPacket::GetMessageType()
{
    return RTMP_MSG_AMF0_COMMAND;
}

int ConnectAppResPacket::Decode(BufferManager* manager)
{
    int ret = ERROR_SUCCESS;
    if ((ret = AMF0ReadString(manager, command_name)) != ERROR_SUCCESS)
    {
        rs_error("amf0 decode connect command_name failed, ret=%d", ret);
        return ret;
    }

    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_RESULT)
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("amf0 decode connect command_name failed, command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }

    if ((ret = AMF0ReadNumber(manager, transaction_id)) != ERROR_SUCCESS)
    {
        rs_error("amf0 decode connect transction failed, ret=%d", ret);
        return ret;
    }

    if (transaction_id != 1.0)
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("amf0 decode connect transction failed, transcation_id=%.1f, ret=%d", transaction_id, ret);
        return ret;
    }

    {
        AMF0Any *p = nullptr;
        if ((ret = AMF0ReadAny(manager, &p)) != ERROR_SUCCESS)
        {
            rs_freep(p);
            rs_error("amf0 decode connect properties failed, ret=%d", ret);
            return ret;
        }

        if (!p->IsObject())
        {
            rs_warn("ignore decode connect properties,marker=%#x", p->marker);
            rs_freep(p);
        }else
        {
            rs_freep(props);
            props = p->ToObject();
        }
    }

    // if ((ret = info->Read(manager)) != ERROR_SUCCESS)
    // {
    //     rs_error("amf0 decode connect info failed, ret=%d", ret);
    //     return ret;
    // }
    {
        AMF0Any *p = nullptr;
        if ((ret = AMF0ReadAny(manager, &p)) != ERROR_SUCCESS)
        {
            rs_freep(p);
            rs_error("decode connect_app response packet: amf0 read info failed. ret=%d", ret);
            return ret;
        }

        if (!p->IsObject())
        {
            rs_freep(p);
            ret = ERROR_PROTOCOL_AMF0_DECODE;
            rs_error("decode connect_app response packet: amf0 read info failed. type wrong, ret=%d", ret);
            return ret;
        }
        else
        {
            rs_freep(info);
            info = p->ToObject();
        }
    }

    rs_trace("decode connect_app response packet success");
    return ret;
}

int ConnectAppResPacket::GetSize()
{
    int size = 0;
    size += AMF0_LEN_STR(command_name);
    size += AMF0_LEN_NUMBER;
    size += AMF0_LEN_OBJECT(props);
    size += AMF0_LEN_OBJECT(info);
    return size;
}

int ConnectAppResPacket::EncodePacket(BufferManager* manager)
{
    int ret = ERROR_SUCCESS;

    if ((ret = AMF0WriteString(manager, command_name)) != ERROR_SUCCESS)
    {
        rs_error("amf0 encode connect.response.command_name failed,ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0WriteNumber(manager, transaction_id)) != ERROR_SUCCESS)
    {
        rs_error("amf0 encode connect.response.transaction_id failed,ret=%d", ret);
        return ret;
    }

    if ((ret = props->Write(manager)) != ERROR_SUCCESS)
    {
        rs_error("amf0 encode connect.response.props failed, ret=%d", ret);
        return ret;
    }

    if ((ret = info->Write(manager)) != ERROR_SUCCESS)
    {
        rs_error("amf0 encode connect.response.info failed, ret=%d", ret);
        return ret;
    }

    return ret;

}


SetWindowAckSizePacket::SetWindowAckSizePacket() : ackowledgement_window_size(0)
{

}

SetWindowAckSizePacket::~SetWindowAckSizePacket()
{

}

int SetWindowAckSizePacket::Decode(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    if (!manager->Require(4))
    {
        ret = ERROR_RTMP_MESSAGE_DECODE;
        rs_error("decode set_ackowledgement_window_size packet failed,ret=%d", ret);
        return ret;
    }

    ackowledgement_window_size = manager->Read4Bytes();
    return ret;
}

int SetWindowAckSizePacket::GetPreferCID()
{
    return RTMP_CID_PROTOCOL_CONTROL;
}

int SetWindowAckSizePacket::GetMessageType()
{
    return RTMP_MSG_WINDOW_ACK_SIZE;
}

int SetWindowAckSizePacket::GetSize()
{
    return 4;
}
int SetWindowAckSizePacket::EncodePacket(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;

    if (!manager->Require(4))
    {
        ret = ERROR_RTMP_MESSAGE_ENCODE;
        rs_error("encode setackowledgement-window_size packet failed,ret=%d", ret);
        return ret;
    }

    manager->Write4Bytes(ackowledgement_window_size);
    return ret;
}

SetPeerBandwidthPacket::SetPeerBandwidthPacket() : bandwidth(0),
                                                   type((int8_t)PeerBandwidthType::DYNAMIC)
{
}

SetPeerBandwidthPacket::~SetPeerBandwidthPacket()
{
}

int SetPeerBandwidthPacket::GetPreferCID()
{
    return RTMP_CID_PROTOCOL_CONTROL;
}

int SetPeerBandwidthPacket::GetMessageType()
{
    return RTMP_MSG_SET_PERR_BANDWIDTH;
}

int SetPeerBandwidthPacket::GetSize()
{
    return 5;
}

int SetPeerBandwidthPacket::EncodePacket(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    if (!manager->Require(5))
    {
        ret = ERROR_SUCCESS;
        rs_error("encode set_peer_bandwidth_packet failed, ret=%d", ret);
        return ret;
    }
    manager->Write4Bytes(bandwidth);
    manager->Write1Bytes(type);

    return ret;
}

FMLEStartPacket::FMLEStartPacket() : command_name(RTMP_AMF0_COMMAND_RELEASE_STREAM),
                                     transaction_id(0),
                                     stream_name("")
{
    command_object = AMF0Any::Null();

}

FMLEStartPacket::~FMLEStartPacket()
{

}

int FMLEStartPacket::GetPreferCID()
{
    return RTMP_CID_OVER_CONNECTION;
}

int FMLEStartPacket::GetMessageType()
{
    return RTMP_MSG_AMF0_COMMAND;
}

int FMLEStartPacket::Decode(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;

    if ((ret = AMF0ReadString(manager, command_name)) != ERROR_SUCCESS)
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("amf0 decode FMLE start packet command_name failed, ret=%d", ret);
        return ret;
    }

    if (command_name.empty() || (command_name != RTMP_AMF0_COMMAND_RELEASE_STREAM &&
                                 command_name != RTMP_AMF0_COMMAND_FC_PUBLISH &&
                                 command_name != RTMP_AMF0_COMMAND_UNPUBLISH))
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("amf0 decode FMLE start packet check command_name failed, ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0ReadNumber(manager, transaction_id)) != ERROR_SUCCESS)
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("amf0 decode FMLE start packet transaction_id failed, ret=%d", ret);
        return ret;
    }

    {
        AMF0Any* p = nullptr;
        if ((ret = AMF0ReadAny(manager, &p)) != ERROR_SUCCESS) {
            rs_freep(p);
            rs_error(
                "decode FMLE_start packet: amf0 read object failed. ret=%d",
                ret);
            return ret;
        }
        else {
            rs_freep(command_object);
            command_object = p;
        }
    }

    if ((ret = AMF0ReadString(manager, stream_name)) != ERROR_SUCCESS)
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("amf0 decode FMLE start packet stream_name failed, ret=%d", ret);
        return ret;
    }

    rs_trace("decode FMLE_start packet success, stream name: %s", stream_name.c_str());
    return ret;
}

int FMLEStartPacket::EncodePacket(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    if ((ret = AMF0WriteString(manager, command_name)) != ERROR_SUCCESS)
    {
        ret = ERROR_PROTOCOL_AMF0_ENCODE;
        rs_error("amf0 encode FMLE start packet command_name failed, ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0WriteNumber(manager, transaction_id)) != ERROR_SUCCESS)
    {
        ret = ERROR_PROTOCOL_AMF0_ENCODE;
        rs_error("amf0 encode FMLE start packet transaction_id failed, ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0WriteNull(manager)) != ERROR_SUCCESS)
    {

        ret = ERROR_PROTOCOL_AMF0_ENCODE;
        rs_error("amf0 encode FMLE start packet null failed, ret=%d", ret);
        return ret;
    }
    if ((ret = AMF0WriteString(manager, stream_name)) != ERROR_SUCCESS)
    {
        ret = ERROR_PROTOCOL_AMF0_ENCODE;
        rs_error("amf0 encode FMLE start packet stream_name failed, ret=%d", ret);
        return ret;
    }

    return ret;
}

int FMLEStartPacket::GetSize()
{
    int size = 0;
    size += AMF0_LEN_STR(command_name);
    size += AMF0_LEN_NUMBER;
    size += AMF0_LEN_ANY(command_object);
    size += AMF0_LEN_STR(stream_name);
    return size;
}

FMLEStartResPacket::FMLEStartResPacket(double trans_id) : transaction_id(trans_id),
                                                          command_name(RTMP_AMF0_COMMAND_RESULT),
                                                          stream_name("")
{
    command_object = AMF0Any::Null();
    args = AMF0Any::Undefined();
}

FMLEStartResPacket::~FMLEStartResPacket()
{
    rs_freep(command_object);
    rs_freep(args);
}

int FMLEStartResPacket::GetPreferCID()
{
    return RTMP_CID_OVER_CONNECTION;
}

int FMLEStartResPacket::GetMessageType()
{
    return RTMP_MSG_AMF0_COMMAND;
}

int FMLEStartResPacket::Decode(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    if ((ret = AMF0ReadString(manager, command_name)) != ERROR_SUCCESS)
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("amf0 decode FMLE start response packet command_name failed, ret=%d", ret);
        return ret;

    }

    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_RESULT)
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("amf0 decode FMLE start response packet command_name check failed, ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0ReadNumber(manager, transaction_id)) != ERROR_SUCCESS)
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("amf0 decode FMLE start response packet transaction_id failed, ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0ReadNull(manager)) != ERROR_SUCCESS)
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("amf0 decode FMLE start response packet null failed, ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0ReadUndefined(manager)) != ERROR_SUCCESS)
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("amf0 decode FMLE start response packet undefined failed, ret=%d", ret);
        return ret;
    }

    {
        AMF0Any* p = nullptr;
        if ((ret = AMF0ReadAny(manager, &p)) != ERROR_SUCCESS) {
            rs_freep(p);
            rs_error("decode FMLE_start response packet: amf0 read object "
                    "failed. ret=%d",
                    ret);
            return ret;
        }
        else {
            rs_freep(command_object);
            command_object = p;
        }
    }
    {
        AMF0Any* p = nullptr;
        if ((ret = AMF0ReadAny(manager, &p)) != ERROR_SUCCESS) {
            rs_freep(p);
            rs_error("decode FMLE_start response packet: amf0 read args "
                    "failed. ret=%d",
                    ret);
            return ret;
        }
        else {
            rs_freep(args);
            args = p;
        }
    }

    rs_trace("decode FMLE_start response packet success");
    return ret;
}

int FMLEStartResPacket::GetSize()
{
    int size = 0;
    size += AMF0_LEN_STR(command_name);
    size += AMF0_LEN_NUMBER;
    size += AMF0_LEN_NULL;
    size += AMF0_LEN_UNDEFINED;
    return size;
}

int FMLEStartResPacket::EncodePacket(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    if ((ret = AMF0WriteString(manager, command_name)) != ERROR_SUCCESS)
    {
        ret = ERROR_PROTOCOL_AMF0_ENCODE;
        rs_error("amf0 encode FMLE start response packet undefined failed, ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0WriteNumber(manager, transaction_id)) != ERROR_SUCCESS)
    {
        ret = ERROR_PROTOCOL_AMF0_ENCODE;
        rs_error("amf0 encode FMLE start response packet undefined failed, ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0WriteNull(manager)) != ERROR_SUCCESS)
    {
        ret = ERROR_PROTOCOL_AMF0_ENCODE;
        rs_error("amf0 encode FMLE start response packet undefined failed, ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0WriteUndefined(manager)) != ERROR_SUCCESS)
    {
        ret = ERROR_PROTOCOL_AMF0_ENCODE;
        rs_error("amf0 encode FMLE start response packet undefined failed, ret=%d", ret);
        return ret;
    }
    return ret;
}

CreateStreamPacket::CreateStreamPacket() : command_name(RTMP_AMF0_COMMAND_CREATE_STREAM),
                                           transaction_id(0)
{
    command_object = AMF0Any::Null();
}

CreateStreamPacket::~CreateStreamPacket()
{
    rs_freep(command_object);
}

int CreateStreamPacket::GetPreferCID()
{
    return RTMP_CID_OVER_CONNECTION;
}

int CreateStreamPacket::GetMessageType()
{
    return RTMP_MSG_AMF0_COMMAND;
}

int CreateStreamPacket::Decode(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;

    if ((ret = AMF0ReadString(manager, command_name)))
    {
        rs_error("amf0 decode createStream command_name failed, ret=%d", ret);
        return ret;
    }

    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_CREATE_STREAM)
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("amf0 decode createStream command_name failed, ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0ReadNumber(manager, transaction_id)) != ERROR_SUCCESS)
    {
        rs_error("amf0 decode createStream transaction_id failed, ret=%d", ret);
        return ret;
    }

    {
        AMF0Any* p = nullptr;
        if ((ret = AMF0ReadAny(manager, &p)) != ERROR_SUCCESS) {
            rs_freep(p);
            rs_error(
                "decode create_stream packet: amf0 read object failed. ret=%d",
                ret);
            return ret;
        }
        else {
            rs_freep(command_object);
            command_object = p;
        }
    }
    return ret;

}

int CreateStreamPacket::GetSize()
{
    int size = 0;
    size += AMF0_LEN_STR(command_name);
    size += AMF0_LEN_NUMBER;
    size += AMF0_LEN_NULL;
    return size;
}

int CreateStreamPacket::EncodePacket(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    return ret;
}

CreateStreamResPacket::CreateStreamResPacket(double trans_id, int sid) : command_name(RTMP_AMF0_COMMAND_RESULT),
                                                                         transaction_id(trans_id),
                                                                         stream_id(sid)
{
    command_object = AMF0Any::Null();
}

CreateStreamResPacket::~CreateStreamResPacket()
{
    rs_freep(command_object);
}

int CreateStreamResPacket::GetPreferCID()
{
    return RTMP_CID_OVER_CONNECTION;
}

int CreateStreamResPacket::GetMessageType()
{
    return RTMP_MSG_AMF0_COMMAND;
}

int CreateStreamResPacket::Decode(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    if ((ret = AMF0ReadString(manager, command_name)) != ERROR_SUCCESS){
        rs_error("decode create_stream response packet. amf0 read command failed. ret=%d", ret);
        return ret;
    }

    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_RESULT) {
        rs_error("decode create_stream response packet. amf0 read command failed. require=%s, actual=%s, ret=%d", RTMP_AMF0_COMMAND_RESULT,
        command_name.empty() ? "[EMPTY]":command_name.c_str(),ret);
        return ret;

    }

    if ((ret = AMF0ReadNumber(manager, transaction_id)) != ERROR_SUCCESS) {
        rs_error("decode create_stream response packet. amf0 read command failed. ret=%d", ret);
        return ret;
    }

    {
        AMF0Any* p = nullptr;
        if ((ret = AMF0ReadAny(manager, &p)) != ERROR_SUCCESS) {
            rs_freep(p);
            rs_error("decode create_stream response packet. amf0 read object failed. ret=%d", ret);
            return ret;
        } else {
            rs_freep(command_object);
            command_object = p;
        }
    }

    if ((ret = AMF0ReadNumber(manager, stream_id)) != ERROR_SUCCESS) {
        rs_error("decode create_stream response packet. amf0 read stream_id failed. ret=%d", ret);
        return ret;
    }

    rs_trace("decode create_stream response packet");

    return ret;
}

int CreateStreamResPacket::GetSize()
{
    int size = 0;
    size += AMF0_LEN_STR(command_name);
    size += AMF0_LEN_NUMBER;
    size += AMF0_LEN_ANY(command_object);
    size += AMF0_LEN_NUMBER;
    return size;
}

int CreateStreamResPacket::EncodePacket(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;

    if ((ret = AMF0WriteString(manager, command_name)) != ERROR_SUCCESS)
    {
        rs_error("amf0 write createStream reponse packet command_name failed,ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0WriteNumber(manager, transaction_id)) != ERROR_SUCCESS)
    {
        rs_error("amf0 write createStream reponse packet transaction_id failed,ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0WriteNull(manager)) != ERROR_SUCCESS)
    {
        rs_error("amf0 write createStream reponse packet null failed,ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0WriteNumber(manager, stream_id)) != ERROR_SUCCESS)
    {
        rs_error("amf0 write createStream reponse packet stream_id failed,ret=%d", ret);
        return ret;
    }

    return ret;

}

PublishPacket::PublishPacket() : command_name(RTMP_AMF0_COMMAND_PUBLISH),
                                 transaction_id(0),
                                 type("live")
{
    command_object = AMF0Any::Null();
}

PublishPacket::~PublishPacket()
{
    rs_freep(command_object);
}

int PublishPacket::GetPreferCID()
{
    return RTMP_CID_OVER_CONNECTION;
}

int PublishPacket::GetMessageType()
{
    return RTMP_MSG_AMF0_COMMAND;
}

int PublishPacket::Decode(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    if ((ret = AMF0ReadString(manager, command_name)) != ERROR_SUCCESS)
    {
        rs_error("amf0 decode publish message command_name failed,ret=%d", ret);
        return ret;
    }

    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_PUBLISH)
    {
        ret = ERROR_PROTOCOL_AMF0_ENCODE;
        rs_error("amf0 decode publish message command_name failed, ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0ReadNumber(manager, transaction_id)) != ERROR_SUCCESS)
    {
        rs_error("amf0 decode publish message transaction_id failed, ret=%d", ret);
        return ret;
    }

    {
        AMF0Any* p = nullptr;
        if ((ret = AMF0ReadAny(manager, &p)) != ERROR_SUCCESS) {
            rs_freep(p);
            rs_error("decode publish packet: amf0 read object failed. ret=%d",ret);
            return ret;
        }
        else {
            rs_freep(command_object);
            command_object = p;
        }
    }

    if ((ret = AMF0ReadString(manager, stream_name)) !=ERROR_SUCCESS)
    {
        rs_error("amf0 decode publish message stream_name failed, ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0ReadString(manager, type)) != ERROR_SUCCESS)
    {
        rs_error("amf0 decode publish message type failed, ret=%d", ret);
        return ret;
    }
    rs_trace("decode publish packet success");

    return ret;
}

int PublishPacket::GetSize()
{
    int size = 0;
    size += AMF0_LEN_STR(command_name);
    size += AMF0_LEN_NUMBER;
    size += AMF0_LEN_ANY(command_object);
    size += AMF0_LEN_STR(stream_name);
    size += AMF0_LEN_STR(type);
    return size;
}

int PublishPacket::EncodePacket(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    return ret;
}

OnStatusCallPacket::OnStatusCallPacket() : command_name(RTMP_AMF0_COMMAND_ON_STATUS),
                                           transaction_id(0)
{
    args = AMF0Any::Null();
    data = AMF0Any::Object();
}

OnStatusCallPacket::~OnStatusCallPacket()
{
    rs_freep(args);
    rs_freep(data);
}

int OnStatusCallPacket::GetPreferCID()
{
    return RTMP_CID_OVER_CONNECTION;
}

int OnStatusCallPacket::GetMessageType()
{
    return RTMP_MSG_AMF0_COMMAND;
}

int OnStatusCallPacket::Decode(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    return ret;
}
int OnStatusCallPacket::GetSize()
{
    int size = 0;
    size += AMF0_LEN_STR(command_name);
    size += AMF0_LEN_NUMBER;
    size += AMF0_LEN_ANY(args);
    size += AMF0_LEN_OBJECT(data);
    return size;
}

int OnStatusCallPacket::EncodePacket(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;

    if ((ret = AMF0WriteString(manager, command_name)) != ERROR_SUCCESS)
    {
        rs_error("amf0 encode onStatus packet commmand_name failed,ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0WriteNumber(manager, transaction_id)) != ERROR_SUCCESS)
    {
        rs_error("amf0 encode onStatus packet transaction_id failed,ret=%d", ret);
        return ret;
    }

    if ((ret = AMF0WriteNull(manager)) != ERROR_SUCCESS)
    {
        rs_error("amf0 encode onStatus packet null failed,ret=%d", ret);
        return ret;
    }

    if ((ret = data->Write(manager)) != ERROR_SUCCESS)
    {
        rs_error("amf0 encode onStatus packet data failed,ret=%d", ret);
        return ret;
    }

    return ret;
}

OnMetadataPacket::OnMetadataPacket()
{
    name = RTMP_AMF0_COMMAND_ON_METADATA;
    metadata = AMF0Any::Object();
}

OnMetadataPacket::~OnMetadataPacket()
{
    rs_freep(metadata);
}

int OnMetadataPacket::GetPreferCID()
{
    return RTMP_CID_OVER_CONNECTION2;
}

int OnMetadataPacket::GetMessageType()
{
    return RTMP_MSG_AMF0_DATA;
}

int OnMetadataPacket::GetSize()
{
    return AMF0_LEN_STR(name) + AMF0_LEN_OBJECT(metadata);
}

int OnMetadataPacket::EncodePacket(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    if ((ret == AMF0WriteString(manager, name)) != ERROR_SUCCESS)
    {
        rs_error("encode name failed, ret=%d", ret);
        return ret;
    }

    if ((ret = metadata->Write(manager)) != ERROR_SUCCESS)
    {
        rs_error("encode metadata failed, ret=%d", ret);
        return ret;
    }
    return ret;
}

int OnMetadataPacket::Decode(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;

    if ((ret = AMF0ReadString(manager, name)))
    {
        rs_error("decode on_metadata message command name failed. ret = %d", ret);
        return ret;
    }

    if (name == RTMP_AMF0_COMMAND_SET_DATAFRAME)
    {
        if ((ret = AMF0ReadString(manager, name)) != ERROR_SUCCESS)
        {
            rs_error("decode on_metadata message command name failed. ret = %d", ret);
            return ret;
        }
    }
    // TODO
    if (name != RTMP_AMF0_COMMAND_ON_METADATA)
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        rs_error("check on_metadata message command name failed.require=%s, actual=%s, ret=%d",
                RTMP_AMF0_COMMAND_ON_METADATA, name.c_str(), ret);
        return ret;
    }

    AMF0Any *any = nullptr;
    if ((ret = AMF0ReadAny(manager, &any)) != ERROR_SUCCESS)
    {
        rs_freep(any);
        rs_error("decode on_metadata message data failed, ret=%d", ret);
        return ret;
    }

    if (any->IsObject())
    {
        rs_freep(metadata);
        metadata = any->ToObject();
    } else
    {
        rs_auto_free(AMF0Any, any);
        if (any->IsEcmaArray())
        {
            AMF0EcmaArray *arr = any->ToEcmaArray();
            for (int i = 0; i < arr->Count(); i++)
            {
                metadata->Set(arr->KeyAt(i), arr->ValueAt(i)->Copy());
            }
        }
    }

    rs_trace("decode on_metadata packet success");

    return ret;
}


PlayPacket::PlayPacket()
{
    command_name = RTMP_AMF0_COMMAND_PLAY;
    transaction_id = 0;
    command_obj = AMF0Any::Null();
    stream_name = "";
    start = -2;
    duration = -1;
    reset = true;
}

PlayPacket::~PlayPacket()
{
    rs_freep(command_obj);
}

int PlayPacket::GetPreferCID()
{
    return RTMP_CID_OVER_CONNECTION;
}

int PlayPacket::GetMessageType()
{
    return RTMP_MSG_AMF0_COMMAND;
}

int PlayPacket::Decode(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;

    if ((ret = AMF0ReadString(manager, command_name)) != ERROR_SUCCESS)
    {
        rs_error("decode play packet: amf0 read command failed. ret=%d", ret);
        return ret;
    }

    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_PLAY)
    {
        ret = ERROR_PROTOCOL_AMF0_DECODE;
        rs_error("decode play packet: amf0 read command failed. require=%s, actual=%s, ret=%d",
                 RTMP_AMF0_COMMAND_PLAY,
                 (command_name.empty() ? "[EMPTY]" : command_name.c_str()), ret);
        return ret;
    }

    if ((ret = AMF0ReadNumber(manager, transaction_id)) != ERROR_SUCCESS)
    {
        rs_error("decode play packet: amf0 read transaction_id failed. ret=%d", ret);
        return ret;
    }
    {
        AMF0Any *p = nullptr;
        if ((ret = AMF0ReadAny(manager, &p)) != ERROR_SUCCESS)
        {
            rs_freep(p);
            rs_error("decode play packet: amf0 read object failed. ret=%d", ret);
            return ret;
        }
        else
        {
            rs_freep(command_obj);
            command_obj = p;
        }
    }

    if ((ret = AMF0ReadString(manager, stream_name)) != ERROR_SUCCESS)
    {
        rs_error("decode play packet: amf0 read stream_name failed. ret=%d", ret);
        return ret;
    }

    if (!manager->Empty() && (ret = AMF0ReadNumber(manager, start)) != ERROR_SUCCESS)
    {
        rs_error("decode play packet: amf0 read start_pos failed. ret=%d", ret);
        return ret;
    }

    if (!manager->Empty() && (ret = AMF0ReadNumber(manager, duration)) != ERROR_SUCCESS)
    {
        rs_error("decode play packet: amf0 read duration failed. ret=%d", ret);
        return ret;
    }

    if (!manager->Empty())
    {
        AMF0Any *p = nullptr;
        if ((ret = AMF0ReadAny(manager, &p)) != ERROR_SUCCESS)
        {
            rs_freep(p);
            rs_error("decode play packet: amf0 read reset marker failed. ret=%d", ret);
            return ret;
        }

        rs_auto_freea(AMF0Any, p);

        if (p)
        {
            if (p->IsBoolean())
            {
                reset = p->ToBoolean();
            }
            else if (p->IsNumber())
            {
                reset = (p->ToNumber() != 0);
            }
            else
            {
                ret = ERROR_PROTOCOL_AMF0_DECODE;
                rs_error("decode play packet: amf0 read reset marker failed. invalid type=%#x, requires number or boolean, ret=%d",
                         p->marker,
                         ret);
                return ret;
            }
        }
    }

    rs_trace("decode play packet success");
    return ret;
}

int PlayPacket::GetSize()
{
    int size = 0;
    size += AMF0_LEN_STR(command_name);
    size += AMF0_LEN_NUMBER;
    size += AMF0_LEN_ANY(command_obj);
    size += AMF0_LEN_STR(stream_name);

    if (start != -2 || duration != -1 || !reset)
    {
        size += AMF0_LEN_NUMBER;
    }
    if (duration != -1 || !reset)
    {
        size += AMF0_LEN_NUMBER;
    }
    if (!reset)
    {
        size += AMF0_LEN_BOOLEAN;
    }

    return size;
}

int PlayPacket::EncodePacket(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    return ret;
}

}
