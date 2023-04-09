#ifndef RS_FLV_HPP
#define RS_FLV_HPP

#include <common/core.hpp>

namespace flv
{

enum class VideoCodecType
{
    SORENSON_H263 = 2,
    SCREEN_VIDEO = 3,
    ON2_VP6 = 4,
    ON3_VP6_WITH_ALPHA_CHANGEL = 5,
    SCREEN_VIDEO_VERSION2 = 6,
    AVC = 7
};

enum class VideoFrameType
{
    KEY_FRAME = 1,
    INTER_FRAME = 2,
    DISPOSABLE_INTER_FRAME = 3,
    GENERATED_KEY_FRAME = 4,
    VIDEO_INFO_FRAME = 5
};

enum class VIdeoPacketType
{
    SEQUENCE_HEADER = 0,
    NALU = 1,
    SEQUENCE_HEADER_EOF = 2
};

enum class AudioCodecType
{
    LINEAR_PCM_PLATFORM_ENDIAN = 0,
    ADPCM = 1,
    MP3 = 2,
    LINEAR_PCM_LITTE_ENDIAN = 3,
    NELLY_MOSER_16KHZ_MONO = 4,
    NELLY_MOSER_8kHZ_MONO = 5,
    NELLY_MOSER = 6,
    AAC = 10,
    SPEEX = 11,
};

enum class AudioPakcetType
{
    SEQUENCE_HEADER = 0,
    RAW_DATA = 1
};

class Codec
{
public:
    static bool IsVideoSeqenceHeader(char* data, int size);
    static bool IsAudioSeqenceHeader(char* data, int size);

private:
    static bool is_h264(char *data, int size);
    static bool is_aac(char *data, int size);

};

} // namespace flv
#endif