#include <repo_version.h>
#include <stdio.h>
#include <common/log/fast_log.hpp>


ILog *_log = new FastLog;
IThreadContext *_context = new ThreadContext;

int main(int argc, char *argv[])
{
    printf("REPO_VERSION:%s\n", REPO_VERSION);
    printf("REPO_DATA:%s\n", REPO_DATE);
    printf("REPO_HASH:%s\n", REPO_HASH);
    rs_verbose("verbose info:%s", "12341321");
    rs_warn("warn info :%s", "12341321");
    rs_error("error info:%s", "12341321");
    rs_trace("trace info:%s", "12341321");
    rs_info("info info:%s", "12341321");
}
