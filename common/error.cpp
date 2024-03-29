#include <common/error.hpp>

bool IsClientGracefullyClose(int32_t err_code)
{
    return err_code == ERROR_SOCKET_READ ||
           err_code == ERROR_SOCKET_READ_FULLY ||
           err_code == ERROR_SOCKET_WRITE ||
           err_code == ERROR_SOCKET_TIMEOUT;
}
bool IsSystemControlError(int err_code)
{
    return err_code == ERROR_CONTROL_REPUBLISH ||
            err_code == ERROR_CONTROL_RTMP_CLOSE;
}
