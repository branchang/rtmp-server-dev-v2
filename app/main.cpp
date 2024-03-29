#include <repo_version.h>
#include <stdio.h>
#include <common/log.hpp>
#include <common/thread.hpp>
#include <common/st.hpp>
#include <common/core.hpp>
#include <common/error.hpp>
#include <common/config.hpp>
#include <app/server.hpp>
#include <common/listener.hpp>

#include <signal.h>

ILog *_log = new FastLog;
IThreadContext *_context = new ThreadContext;
Server *_server = new Server();
Config *_config = new Config();

int32_t RunMaster()
{
    int32_t ret = ERROR_SUCCESS;
    if ((ret = _server->InitializeST()) != ERROR_SUCCESS)
    {
        return ret;
    }
    return ret;
}

void signal_handler(int signo)
{

}

int32_t main(int argc, char *argv[])
{
    rs_info("##############################");
    rs_info("repo_version:%s", REPO_VERSION);
    rs_info("repo_date:%s", REPO_DATE);
    rs_info("repo_hash:%s", REPO_HASH);
    rs_info("##############################");

    signal(SIGPIPE, signal_handler);
    RunMaster();

    RTMPStreamListener listener(_server, ListenerType::RTMP);
    listener.Listen("0.0.0.0", 1935);
    while(1)
        st_usleep(1000000000);
    return 0;
}
