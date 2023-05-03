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
    return true;
}

int Config::GetMRSleepMS(const std::string &vhost)
{
    return 350;
}

bool Config::GetRealTimeEnabled(const std::string &vhost)
{
    return false;
}

bool Config::GetReduceSequenceHeader(const std::string &vhost)
{
    return true;
}

bool Config::GetVhostIsEdge(const std::string &vhost)
{
    return true;
}

int Config::GetPublishFirstPktTimeout(const std::string &vhost)
{
    return 20000;
}

int Config::GetPublishNormalPktTimeout(const std::string &vhost)
{
    return 5000;

}

bool Config::GetTCPNoDelay(const std::string &vhost)
{
    return true;
}

int Config::GetDvrTimeJitter(const std::string &vhost)
{
    return 1;
}

std::string Config::GetDvrPath(const std::string &vhost)
{
    return "/tmp/flv_test";
}

bool Config::GetUTCTime()
{
    return false;
}

bool Config::GetDvrWaitKeyFrame(const std::string &vhost)
{
    return true;
}

std::string Config::GetDvrPlan(const std::string &vhost)
{
    return RS_CONFIG_NVR_PLAN_SEGMENT;
}

int Config::GetDvrDuration(const std::string &vhost)
{
    return 10;
}

bool Config::GetDvrEnabled(const std::string &vhost)
{
    return true;
}
