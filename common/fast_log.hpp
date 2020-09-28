#ifndef RS_FAST_LOG_HPP
#define RS_FAST_LOG_HPP
#include <common/core.hpp>
#include <common/log.hpp>
#include <common/reload.hpp>
#include <map>
#include <st.h>

// class FastLog::public ILog, IRel

class FastLog: public ILog, IReloadHandler
{
public:
    FastLog();
    virtual ~FastLog();

public:
    virtual void Verbose(const char *tag, int context_id, const char *fmt, ...);
    virtual void Info(const char *tag, int context_id, const char *fmt, ...);
    virtual void Trace(const char *tag, int context_id, const char *fmt, ...);
    virtual void Warn(const char *tag, int context_id, const char *fmt, ...);
    virtual void Error(const char *tag, int context_id, const char *fmt, ...);

public:
    virtual int OnReloadUTCTime();
    virtual int OnReloadLogTank();
    virtual int OnReloadLogLevel();
    virtual int OnReloadLogFile();

protected:
    virtual bool GenerateHeader(bool error, const char *tag, int context_id, const char* level_name, int *header_size);
    virtual void WriteLog(int &fd, char *str_log, int size, int level);

protected:
    int level_;
private:
    char *log_data_;
    int fd_;
    bool log_to_file_tank_;
    bool utc_;
};

class ThreadContext:public IThreadContext
{
public:
    ThreadContext();
    virtual ~ThreadContext();

public:
    virtual int GenerateID() override;
    virtual int GetID() override;
    virtual int SetID(int v) override;
    virtual void ClearID() override;
private:
    std::map<st_thread_t, int> cache_;
};

#endif
