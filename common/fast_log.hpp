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
    virtual void Verbose(const char *tag, int32_t context_id, const char *fmt, ...);
    virtual void Info(const char *tag, int32_t context_id, const char *fmt, ...);
    virtual void Trace(const char *tag, int32_t context_id, const char *fmt, ...);
    virtual void Warn(const char *tag, int32_t context_id, const char *fmt, ...);
    virtual void Error(const char *tag, int32_t context_id, const char *fmt, ...);

public:
    virtual int32_t OnReloadUTCTime();
    virtual int32_t OnReloadLogTank();
    virtual int32_t OnReloadLogLevel();
    virtual int32_t OnReloadLogFile();

protected:
    virtual bool GenerateHeader(bool error, const char *tag, int context_id, const char* level_name, int32_t *header_size);
    virtual void WriteLog(int &fd, char *str_log, int32_t size, int32_t level);

protected:
    int32_t level_;
private:
    char *log_data_;
    int32_t fd_;
    bool log_to_file_tank_;
    bool utc_;
};

class ThreadContext:public IThreadContext
{
public:
    ThreadContext();
    virtual ~ThreadContext();

public:
    virtual int32_t GenerateID() override;
    virtual int32_t GetID() override;
    virtual int32_t SetID(int v) override;
    virtual void ClearID() override;
private:
    std::map<st_thread_t, int32_t> cache_;
};

#endif
