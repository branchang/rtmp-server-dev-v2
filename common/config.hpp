#ifndef RS_CONFIG_HPP
#define RS_CONFIG_HPP

#include <common/reload.hpp>

class Config
{
public:
    Config();
    virtual ~Config();

public:
    virtual void Subscribe(IReloadHandler *handler);
    virtual void UnSubScribe(IReloadHandler *handler);
    virtual int32_t Reload();

    virtual int32_t getChunkSize(const std::string &vhost);
    virtual bool GetATC(const std::string &vhost);
    virtual bool GetMREnabled(const std::string &vhost);
    virtual int GetMRSleepMS(const std::string &vhost);
    virtual bool GetRealTimeEnabled(const std::string &vhost);
    virtual bool GetReduceSequenceHeader(const std::string &vhost);

};

extern Config *_config;

#endif
