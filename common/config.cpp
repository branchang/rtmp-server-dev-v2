#include <common/config.hpp>
#include <common/error.hpp>


Config::Config()
{

}

Config::~Config()
{

}


int32_t Config::getChunkSize(const std::string &vhost)
{
    return 128;
}

void Config::Subscribe(IReloadHandler *handler)
{

}

void Config::UnSubScribe(IReloadHandler *handler)
{

}

int32_t Config::Reload()
{
    return ERROR_SUCCESS;
}

bool Config::GetATC(const std::string &vhost)
{
    return false;
}

bool Config::GetMREnabled(const std::string &vhost)
{
    return false;
}

int Config::GetMRSleepMS(const std::string &vhost)
{
    return 350;
}

bool Config::GetRealTimeEnabled(const std::string &host)
{
    return false;
}

