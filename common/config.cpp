#include <common/config.hpp>
#include <common/error.hpp>


IConfig::IConfig()
{

}

IConfig::~IConfig()
{

}

void IConfig::Subscribe(IReloadHandler *handler)
{

}

void IConfig::UnSubScribe(IReloadHandler *handler)
{

}

int IConfig::Reload()
{
    return ERROR_SUCCESS;
}

