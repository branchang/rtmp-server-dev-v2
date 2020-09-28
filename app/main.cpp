#include <repo_version.h>
#include <stdio.h>
#include <common/fast_log.hpp>
#include <common/thread.hpp>
#include <common/st.hpp>
#include <common/core.hpp>
#include <common/error.hpp>
#include <app/server.hpp>
#include <app/listener.hpp>


ILog *_log = new FastLog;
IThreadContext *_context = new ThreadContext;
Server *_server = new Server();

int RunMaster()
{
    int ret = ERROR_SUCCESS;
    if ((ret = _server->InitializeST()) != ERROR_SUCCESS)
    {
        return ret;
    }
}

int main(int argc, char *argv[])
{
    rs_verbose("verbose info:%s", "12341321");
    rs_warn("warn info :%s", "12341321");
    rs_error("error info:%s", "12341321");
    rs_trace("trace info:%s", "12341321");
    rs_info("info info:%s", "12341321");

    // STInit();
    // internal::IThreadHandler handler;
    // internal::Thread thread("test_thread", &handler, 0, false);
    // thread.Start();
    // st_usleep(10000000000);
    // thread.Stop();
    RunMaster();

    RTMPStreamListener listener(_server, ListenerType::RTMP_STEAM);
    listener.Listen("0.0.0.0", 80);
    st_usleep(1000000);
}
