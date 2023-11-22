#ifndef RS_SAMPLE_HPP
#define RS_SAMPLE_HPP

#include <common/core.hpp>

#define MAX_CODEC_SAMPLE 128

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
    // bool is_video;
    // AudioCodecType acodec;
    // FLVSampleRate flv_sample_rate;
    // int aac_sample_rate;
    // AudioSampleSize sound_size;
    // AudioSoundType sound_type;
    // AudioPakcetType aac_packet_type;
    int nb_sample_units;
    CodecSampleUnit sample_units[MAX_CODEC_SAMPLE];
    // int32_t cts;
    // bool has_idr;
    // bool has_sps_pps;
    // bool has_aud;
    // AVCNaluType first_nalu_type;
    // VideoFrameType frame_type;
    // VideoPacketType avc_packet_type;
};


#endif
