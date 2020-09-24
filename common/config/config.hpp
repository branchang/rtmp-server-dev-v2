#ifndef RS_CONFIG_HPP
#define RS_CONFIG_HPP

#include <common/config/reload.hpp>

class IConfig
{
public:
    IConfig();
    virtual ~IConfig();

public:
    virtual void Subscribe(IReloadHandler *handler);
    virtual void UnSubScribe(IReloadHandler *handler);
    virtual int Reload();

};

#endif
