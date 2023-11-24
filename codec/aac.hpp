#ifndef RS_AAC_HPP
#define RS_AAC_HPP

#include <common/core.hpp>
#include <codec/codec.hpp>

#include <string>

namespace aac
{
enum class ObjectType
{
    UNKNOW = 0,
    MAIN = 1,
    LC = 2,
    SSR = 3,
    //LC + SBR
    HE = 4,
    //LC + SBR + PS
    HEV2 = 5
};

}

class AACCodec : public ACodec
{
public:
    AACCodec();
    virtual ~AACCodec();

public:
    virtual bool HasSequenceHeader() override;
    virtual int DecodeSequenceHeader(BufferManager *manager) override;
    virtual int DecodeRawData(BufferManager *manager, CodecSample * sample) override;

public:
    int extradata_size;
    char *extradata;
    uint8_t channels;
    uint8_t sample_rate;
    aac::ObjectType object_type;
};
#endif
