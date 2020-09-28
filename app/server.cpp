#include <app/server.hpp>
#include <common/log.hpp>
#include <common/error.hpp>
#include <common/st.hpp>
#include <common/utils.hpp>

#include <unistd.h>
#include <fcntl.h>


IServerListener::IServerListener(Server *server, ListenerType type): server_(server),type_(type), ip_(""), port_(-1)
{

}

IServerListener::~IServerListener()
{

}

ListenerType IServerListener::GetType()
{
    return type_;
}


RTMPStreamListener::RTMPStreamListener(Server *server, ListenerType type): IServerListener(server, type), listener_(nullptr)
{

}

RTMPStreamListener::~RTMPStreamListener()
{
    rs_freep(listener_);
}

int RTMPStreamListener::Listen(const std::string &ip, int port)
{
    int ret = ERROR_SUCCESS;

    ip_ = ip;
    port_ = port;

    rs_freep(listener_);
    listener_ = new TCPListener(this, ip, port);

    if ((ret = listener_->Listen()) != ERROR_SUCCESS)
    {
        rs_error("tcp listen failed, ep=[%s:%d], ret=%d", ip.c_str(), port, ret);
        return ret;
    }

    rs_info("RTMP streamer listen on [%s:%d]", ip.c_str(), port);

    return ret;
}


int RTMPStreamListener::OnTCPClient(st_netfd_t stfd)
{
    STCloseFd(stfd);
    return 0;
}


Server::Server()
{

}

Server::~Server()
{

}

int Server::InitializeST()
{
    int ret =ERROR_SUCCESS;
    if ((ret = STInit()) != ERROR_SUCCESS)
    {
        rs_error("STInit failed,ret=%d", ret);
        return ret;
    }

    _context->GenerateID();
    rs_trace("rtmp server main cid=%d, pid=%d", _context->GetID(), ::getpid());

    return ret;
}

int Server::Initilaize()
{

}

int Server::AcceptClient()
{

}

