#ifndef RS_FLV_HPP
#define RS_FLV_HPP

#include <common/core.hpp>


#include <string>

#define MAX_CODEC_SAMPLE 128
#define AAC_SAMPLE_RATE_UNSET 15

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

enum AVCNaluType
{
    UNKNOW = 0,
    NON_IDR = 1,
    DATA_PARTITION_A = 2,
    DATA_PARTITION_B = 3,
    DATA_PARTITION_C = 4,
    IDR = 5,
    SEI = 6,
    SPS = 7,
    PPS = 8,
    ACCESS_UNIT_DELIMITER = 9,
    EOSEQUENCE = 10,
    EOSTREAM = 11,
    FILTER_DATA = 12,
    SPS_EXT = 13,
    PERFIX_NALU = 14,
    SUBSET_SPS = 15,
    LAYER_WITHOUT_PARTITION = 19,
    CODECD_SLICE_EXT = 20
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
    MP3_8KHZ = 14,
    DEVICE_SPECIFIC_SOUND = 15,
    UNKNOW =16
};


enum class FLVSampleRate
{
    SAMPLE_RATE_5512 = 0,
    SAMPLE_RATE_11025 = 1,
    SAMPLE_RATE_22050 = 2,
    SAMPLE_RATE_44100 = 3,
    UNKNOW = 4
};

enum class AudioSampleSize
{
    SAMPLE_SIZE_8BIT = 0,
    SAMPLE_SIZE_16BIT = 1,
    UNKNOW = 2
};

enum class AudioSoundType
{
    MONO = 0,
    STEREO = 1,
    UNKNOW = 2
};

enum class AudioPakcetType
{
    SEQUENCE_HEADER = 0,
    RAW_DATA = 1,
    UNKNOW = 2,
};

enum class AACObjectType
{
    UNKNOW = 0,
    MAIN = 1,
    LC = 2,
    SSR = 3,
    // LC + SBR 
    HE =4,
    // LC + SBR + PS
    HEV2 = 5
};

extern std::string ACodec2Str(AudioCodecType codec_type);
extern std::string AACProfile2Str(AACObjectType object_type);

class CodecSampleUnit
{
public:
    CodecSampleUnit();
    virtual ~CodecSampleUnit();
public:
    int size;
    char *bytes;
};

class CodecSample
{
public:
    CodecSample();
    virtual ~CodecSample();
public:
    virtual void Clear();
    virtual int AddSampleUnit(char *bytes, int size);
public:
    bool is_video;
    AudioCodecType acodec;
    FLVSampleRate flv_sample_rate;
    int aac_sample_rate;
    AudioSampleSize sound_size;
    AudioSoundType sound_type;
    AudioPakcetType aac_packet_type;
    int nb_sample_units;
    CodecSampleUnit sample_units[MAX_CODEC_SAMPLE];
    int32_t cts;
    bool has_idr;
    bool has_sps_pps;
    bool has_aud;
    AVCNaluType first_nalu_type;
};


class Codec
{
public:
    Codec();
    ~Codec();
public:
    static bool IsVideoSeqenceHeader(char* data, int size);
    static bool IsAudioSeqenceHeader(char* data, int size);
    int DemuxAudio(char *data, int size, CodecSample *sample);

private:
    static bool is_h264(char *data, int size);
    static bool is_aac(char *data, int size);
    int aac_sequence_header_demux(char *data, int size);
    bool is_aac_codec_ok();
public:
    int audio_codec_id;
    int aac_extra_size;
    char *aac_extra_data;
    uint8_t channels;
    uint8_t sample_rate;
    AACObjectType aac_object_type;
};

} // namespace flv
#endif
