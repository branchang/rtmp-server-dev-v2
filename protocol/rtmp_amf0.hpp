#ifndef RS_RTMP_AMF0_HPP
#define RS_RTMP_AMF0_HPP

#include <common/core.hpp>
#include <common/buffer.hpp>

#include <string>

namespace rtmp
{
class AMF0Object;

class AMF0Any
{
public:
    AMF0Any();
    virtual ~AMF0Any();
public:
    static AMF0Object *Object();

};

class AMF0Object : public AMF0Any
{

};

extern int AMF0ReadString(BufferManager *manager, std::string &value);
// extern int AMF0ReadUTF8(BufferManager *manager, std::string &value);
extern int AMF0ReadNumber(BufferManager *manager, double &value);

} // namespace rtmp

#endif
