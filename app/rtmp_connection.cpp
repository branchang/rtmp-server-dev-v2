#include <app/rtmp_connection.hpp>
#include <protocol/rtmp_stack.hpp>
#include <protocol/rtmp_consts.hpp>
#include <protocol/rtmp_source.hpp>
#include <common/error.hpp>
#include <common/config.hpp>
#include <common/log.hpp>
#include <common/utils.hpp>

#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>

RTMPConnection::RTMPConnection(Server *server, st_netfd_t stfd): Connection(server, stfd),
                                                                 server_(server),
                                                                 type_(rtmp::ConnType::UNKNOW)
{
    request_ = new rtmp::Request;
    response_ = new rtmp::Response;
    socket_ = new StSocket(stfd);
    rtmp_ = new RTMPServer(socket_);
    tcp_nodelay_ = false;
    mw_sleep_ = RTMP_MR_SLEEP_MS;
    wakeable_ = nullptr;
}

RTMPConnection::~RTMPConnection()
{
    rs_freep(response_);
    rs_freep(request_);
    rs_freep(rtmp_);
    rs_freep(socket_);
}

void RTMPConnection::Dispose()
{
    // if(wakeable_) {
    //     wakeable_->WakeUp();
    // }
}

int32_t RTMPConnection::DoCycle()
{
    int ret = ERROR_SUCCESS;
    rtmp_->SetRecvTimeout(RTMP_RECV_TIMEOUT_US);
    rtmp_->SetSendTimeout(RTMP_SEND_TIMEOUT_US);
    if ((ret = rtmp_->Handshake()) != ERROR_SUCCESS)
    {
        rs_error("rtmp handshake failed,ret=%d", ret);
        return ret;
    }
    // rtmp::CommonMessage *msg;
    // rtmp::Request req;
    // rtmp_->ConnectApp(&req);
    if ((ret = rtmp_->ConnectApp(request_)) != ERROR_SUCCESS)
    {
        rs_error("rtmp connect app failed,ret=%d", ret);
        return ret;
    }
    request_->ip = client_ip_;

    ServiceCycle();

    return ret;
}

int32_t RTMPConnection::StreamServiceCycle()
{
    int ret = ERROR_SUCCESS;
    rtmp::ConnType type;
    if ((ret = rtmp_->IdentifyClient(response_->stream_id, type, request_->stream, request_->duration)) != ERROR_SUCCESS)
    {
        rs_error("identify client failed,ret=%d", ret);
        return ret;
    }

    rtmp::DiscoveryTcUrl(request_->tc_url,
                         request_->schema,
                         request_->host,
                         request_->vhost,
                         request_->app,
                         request_->stream,
                         request_->port,
                         request_->param);
    request_->Strip();

    if (request_->schema.empty() || request_->vhost.empty() || request_->port.empty() || request_->app.empty())
    {
        ret = ERROR_RTMP_REQ_TCURL;
        rs_error("discovery tcUrl failed,tcUrl=%s,schema=%s,vhost=%s,port=%s,app=%s,ret=%d",
                 request_->tc_url.c_str(),
                 request_->schema.c_str(),
                 request_->vhost.c_str(),
                 request_->port.c_str(),
                 request_->app.c_str(),
                 ret);
        return ret;
    }

    if (request_->stream.empty())
    {
        ret= ERROR_RTMP_STREAM_NAME_EMPTY;
        rs_error("rtmp:empty stream name is not allowed,ret=%d",ret);
        return ret;
    }

    rs_trace("client identified, type=%d, stream_name=%s, duration=%.2f", type, request_->stream.c_str(), request_->duration);

    rtmp::Source *source = nullptr;
    if ((ret = rtmp::Source::FetchOrCreate(request_, server_, &source)) != ERROR_SUCCESS)
    {
        rs_error("FetchOrCreate failed.%d", ret);
        return ret;
    }
    type_ = type;
    switch (type)
    {
        case rtmp::ConnType::FMLE_PUBLISH:
            rs_info("FMLE start to publish stream %s", request_->stream.c_str());
            if ((ret = rtmp_->StartFmlePublish(response_->stream_id)) != ERROR_SUCCESS)
            {
                rs_error("start to publish stream failed,ret=%d", ret);
                return ret;
            }
            return Publishing(source);
        case rtmp::ConnType::PLAY:
            if ((ret = rtmp_->StartPlay(response_->stream_id)) != ERROR_SUCCESS)
            {
                rs_error("start to play stream failed. ret=%d", ret);
                return ret;
            }
            return Playing(source);
        // case rtmp::ConnType::UNKNOW:
        default:
            break;
    }

    return ret;
}

int32_t RTMPConnection::Publishing(rtmp::Source *source)
{
    int ret = ERROR_SUCCESS;

    bool vhost_is_edge = _config->GetVhostIsEdge(request_->vhost);
    if ((ret = acquire_publish(source, false)) == ERROR_SUCCESS)
    {
        PublishRecvThread recv_thread(rtmp_, request_, st_netfd_fileno(client_stfd_), 0, this, source,type_!=rtmp::ConnType::FLASH_PUBLISH, vhost_is_edge);

        rs_info("start to publish stream %s sucess.", request_->stream.c_str());

        ret = do_publish(source, &recv_thread);

        recv_thread.Stop();
    }

    return ret;
}

int32_t RTMPConnection::DoPlaying(rtmp::Source* source, rtmp::Consumer* consumer, QueueRecvThread* recv_thread)
{
    int ret = ERROR_SUCCESS;
    rtmp::MessageArray msgs(RTMP_MR_MSGS);

    while (!disposed_)
    {
        if (expired_)
        {
            ret = ERROR_USER_DISCONNECT;
            rs_error("connection expired.ret=%d", ret);
            return ret;
        }
        while (!recv_thread->Empty())
        {
            rtmp::CommonMessage* msg = recv_thread->Pump();
            rs_freep(msg);
        }

        if ((ret = recv_thread->ErrorCode()) != ERROR_SUCCESS)
        {
            if(!IsClientGracefullyClose(ret) && !IsSystemControlError(ret))
            {
                rs_error("recv thread failed.ret=%d", ret);
            }
            return ret;
        }

        consumer->Wait(RTMP_MR_MIN_MSGS, mw_sleep_);

        int count = 0;
        if ((ret = consumer->DumpPackets(&msgs, count)) != ERROR_SUCCESS)
        {
            rs_error("get message from consumer failed.ret=%d", ret);
        }
        if (count <= 0)
        {
            rs_info("mw sleep %dms for no msg", mw_sleep_);
            st_usleep(mw_sleep_ * 1000);
            continue;
        }
        if ((ret = rtmp_->SendAndFreeMessages(msgs.msgs, count, response_->stream_id)) != ERROR_SUCCESS)
        {
            rs_error("send message to client failed.ret=%d", ret);
        }
        return ret;
    }
    return ret;
}

int32_t RTMPConnection::Playing(rtmp::Source* source)
{
    int ret = ERROR_SUCCESS;
    rtmp::Consumer* consumer = nullptr;
    if ((ret = source->CreateConsumer(this, consumer)) != ERROR_SUCCESS)
    {
        rs_error("create consumer failed.ret=%d", ret);
        return ret;
    }

    QueueRecvThread recv_thread(consumer, rtmp_, mw_sleep_);
    if ((ret =recv_thread.Start()) != ERROR_SUCCESS)
    {
        rs_error("start isolate recv thread failed. ret= %d", ret);
        return ret;
    }

    wakeable_ = consumer;
    ret = DoPlaying(source, consumer, &recv_thread);
    wakeable_ = nullptr;
    recv_thread.Stop();

    if (!recv_thread.Empty()) {
        rs_warn("drop received %d message", recv_thread.Size());
    }

    return 0;
}

int32_t RTMPConnection::ServiceCycle()
{
    int ret = ERROR_SUCCESS;

    if ((ret = rtmp_->SetWindowAckSize((int)RTMP_DEFAULT_WINDOW_ACK_SIZE)) != ERROR_SUCCESS)
    {
        rs_error("set window ackowledgement size failed,ret=%d", ret);
        return ret;
    }

    if ((ret = rtmp_->SetPeerBandwidth((int)RTMP_DEFAULT_PEER_BAND_WIDTH, (int)rtmp::PeerBandwidthType::DYNAMIC)) != ERROR_SUCCESS)
    {
        rs_error("set peer bandwidth failed, ret=%d", ret);
        return ret;
    }

    std::string local_ip = Utils::GetLocalIp(st_netfd_fileno(client_stfd_));

    int chunk_size = _config->getChunkSize(request_->vhost);
    if ((ret = rtmp_->SetChunkSize(chunk_size)) != ERROR_SUCCESS)
    {
        rs_error("set chunk size failed, ret=%d", ret);
        return ret;
    }

    if ((ret = rtmp_->ResponseConnectApp(request_, local_ip)) != ERROR_SUCCESS)
    {
        rs_error("response connect app failed, ret=%d", ret);
        return ret;
    }

    while (!disposed_)
    {
        ret = StreamServiceCycle();
        if (ret == ERROR_SUCCESS)
        {
            continue;
        }
    }

    return ret;
}

void RTMPConnection::Resample()
{

}

int64_t RTMPConnection::GetSendBytesDelta()
{
    return 0;
}

int64_t RTMPConnection::GetRecvBytesDelta()
{
    return 0;
}

void RTMPConnection::CleanUp()
{

}

int RTMPConnection::process_publish_message(rtmp::Source *source, rtmp::CommonMessage *msg, bool is_edge)
{
    int ret = ERROR_SUCCESS;
    if (is_edge)
    {
        //TODO implement edge opt
    }

    if (msg->header.IsAudio())
    {
        if ((ret = source->OnAudio(msg)) != ERROR_SUCCESS)
        {
            rs_error("source process audio message failed. ret=%d", ret);
            return ret;
        }

    }

    if (msg->header.IsVideo())
    {
        if ((ret = source->OnVideo(msg)) != ERROR_SUCCESS)
        {
            rs_error("source process video message failed. ret=%d", ret);
            return ret;
        }
    }

    if (msg->header.IsAMF0Data() || msg->header.IsAMF3Data())
    {
        rtmp::Packet *packet = nullptr;
        if ((ret = rtmp_->DecodeMessage(msg, &packet)) != ERROR_SUCCESS)
        {
            rs_error("decode onMetaData message failed, ret=%d", ret);
            return ret;
        }

        rs_auto_free(rtmp::Packet, packet);
        if (dynamic_cast<rtmp::OnMetadataPacket *>(packet))
        {
            rtmp::OnMetadataPacket *pkt = dynamic_cast<rtmp::OnMetadataPacket *>(packet);
            if ((ret = source->OnMetadata(msg, pkt)) != ERROR_SUCCESS)
            {
                rs_error("source process on_metadata message failed,ret=%d", ret);
                return ret;
            }
            return ret;
        }
    }

    return ret;
}

int RTMPConnection::handle_publish_message(rtmp::Source *source, rtmp::CommonMessage *msg, bool is_fmle, bool is_edge)
{
    int ret = ERROR_SUCCESS;

    if (msg->header.IsAMF0Command() || msg->header.IsAMF3Command())
    {
        rtmp::Packet *packet = nullptr;
        if ((ret = rtmp_->DecodeMessage(msg, &packet)) != ERROR_SUCCESS)
        {
            rs_error("fmle decode unpublish message failed, ret=%d", ret);
            return ret;
        }
        rs_auto_free(rtmp::Packet, packet);

        if (!is_fmle)
        {
            rs_trace("refresh flash publish finished.");
            return ERROR_CONTROL_REPUBLISH;

        }

        if (dynamic_cast<rtmp::FMLEStartPacket *>(packet))
        {
            rtmp::FMLEStartPacket *pkt = dynamic_cast<rtmp::FMLEStartPacket *>(packet);
            if ((ret = rtmp_->FMLEUnPublish(response_->stream_id, pkt->transaction_id)) != ERROR_SUCCESS)
            {
                return ret;
            }
            return ERROR_CONTROL_REPUBLISH;
        }
        return ret;
    }

    if ((ret = process_publish_message(source, msg, is_edge)) != ERROR_SUCCESS)
    {
        rs_error("FMLE process publish message failed, ret=%d", ret);
        return ret;
    }

    return ret;
}


void RTMPConnection::set_socket_option()
{
    bool nvalue = _config->GetTCPNoDelay(request_->vhost);
    if (nvalue != tcp_nodelay_)
    {
        tcp_nodelay_ = nvalue;

        int fd = st_netfd_fileno(client_stfd_);
        socklen_t nb_v = sizeof(int);
        int ov = 0;

        getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &ov, &nb_v);

        int nv = tcp_nodelay_;
        if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nv, nb_v) < 0)
        {
            rs_error("set socket TCP_NODELAY=%d failed", nv);
            return ;
        }

        getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nv, &nb_v);
        rs_trace("set socket TCP_NODELAY=%d success. %d=>%d", tcp_nodelay_, ov, nv);
    }
}

int RTMPConnection::acquire_publish(rtmp::Source *source, bool is_edge)
{
    int ret = ERROR_SUCCESS;

    if (!source->CanPublish(is_edge))
    {
        ret = ERROR_SYSTEM_STREAM_BUSY;
        rs_warn("stream %s is alread publishing.ret=%d", request_->GetStreamUrl().c_str());
        return ret;
    }

    if (is_edge)
    {
    }
    else
    {
        if ((ret = source->OnPublish()) != ERROR_SUCCESS)
        {
            rs_error("notify publish failed. ret=%d", ret);
            return ret;
        }
    }
    return ret;
}

int RTMPConnection::do_publish(rtmp::Source *source, PublishRecvThread *recv_thread)
{
    int ret = ERROR_SUCCESS;
    if ((ret = recv_thread->Start()) != ERROR_SUCCESS)
    {
        rs_error("start isolae recv thread failed. ret=%d", ret);
        return ret;
    }

    int recv_thread_cid = recv_thread->GetCID();
    //merge isolate recv thread log
    recv_thread->SetCID(_context->GetID());

    publish_first_pkt_timeout_ = _config->GetPublishFirstPktTimeout(request_->vhost);
    publish_normal_pkt_timeout_ = _config->GetPublishNormalPktTimeout(request_->vhost);

    set_socket_option();

    bool mr = _config->GetMREnabled(request_->vhost);
    int mr_sleep = _config->GetMRSleepMS(request_->vhost);

    rs_trace("start publish mr=%d/%d, first_pkt_timeout=%d, normal_pkt_timeout=%d, rtcid=%d", mr, mr_sleep, publish_first_pkt_timeout_, publish_normal_pkt_timeout_, recv_thread_cid);

    int64_t nb_msgs = 0;

    while (!disposed_)
    {
        if (expired_)
        {
            ret = ERROR_USER_DISCONNECT;
            rs_error("connection expired. ret=%d", ret);
            return ret;
        }

        if (nb_msgs == 0)
        {
            recv_thread->Wait(publish_first_pkt_timeout_);
        }
        else
        {
            recv_thread->Wait(publish_normal_pkt_timeout_);
        }

        if ((ret = recv_thread->ErrorCode()) != ERROR_SUCCESS)
        {
            if (!IsSystemControlError(ret) && !IsClientGracefullyClose(ret))
            {
                rs_error("recv thread failed. ret=%d", ret);
                return ret;
            }

        }

        if (recv_thread->GetMsgNum() <= nb_msgs)
        {
            ret = ERROR_SOCKET_TIMEOUT;
            rs_warn("publish timeout %dms, nb_msgs=%lld, ret=%d", nb_msgs?publish_normal_pkt_timeout_: publish_first_pkt_timeout_, nb_msgs, ret);
            break;
        }

        nb_msgs = recv_thread->GetMsgNum();
    }
    return ret;
}
