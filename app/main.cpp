#include <repo_version.h>
#include <stdio.h>
#include <common/log/fast_log.hpp>
#include <common/thread/thread.hpp>
#include <common/io/st.hpp>


ILog *_log = new FastLog;
IThreadContext *_context = new ThreadContext;

int main(int argc, char *argv[])
{
    rs_verbose("verbose info:%s", "12341321");
    rs_warn("warn info :%s", "12341321");
    rs_error("error info:%s", "12341321");
    rs_trace("trace info:%s", "12341321");
    rs_info("info info:%s", "12341321");

    STInit();
    internal::IThreadHandler handler;
    internal::Thread thread("test_thread", &handler, 0, false);
    thread.Start();
    st_usleep(10000000000);
    thread.Stop();
}
