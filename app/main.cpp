#include <repo_version.h>
#include <stdio.h>
#include <common/fast_log.hpp>
#include <common/thread.hpp>
#include <common/st.hpp>
#include <common/core.hpp>
#include <common/error.hpp>
#include <app/server.hpp>
#include <common/listener.hpp>


ILog *_log = new FastLog;
IThreadContext *_context = new ThreadContext;
Server *_server = new Server();

int32_t RunMaster()
{
    int32_t ret = ERROR_SUCCESS;
    if ((ret = _server->InitializeST()) != ERROR_SUCCESS)
    {
        return ret;
    }
    return ret;
}

int32_t main(int argc, char *argv[])
{
    rs_verbose("verbose info:%s", "12341321");
    rs_warn("warn info :%s", "12341321");
    rs_error("error info:%s", "12341321");
    rs_trace("trace info:%s", "12341321");
    rs_info("info info:%s", "12341321");

    RunMaster();

    RTMPStreamListener listener(_server, ListenerType::RTMP_STEAM);
    listener.Listen("0.0.0.0", 1935);
    st_usleep(1000000000);
    return 0;
}
