#ifndef RS_AVC_HPP
#define RS_AVC_HPP

#include <common/core.hpp>
#include <codec/codec.hpp>

namespace avc
{

enum class AVCProfile
{
    UNKNOW = 0,
    BASELINE = 66,
    CONSTRAINED_BASELINE = 578,
    MAIN = 77,
    EXTENDED = 88,
    HIGH = 110,
    HIGH_10 = 110,
    HIGH_10_INTRA = 2158,
    HIGH_422 = 122,
    HIGH_422_INTRA = 2170,
    HIGH_444 = 144,
    HIGH_444_PREDICTIVE = 244,
    HIGH_444_INTRA = 2192,
};

enum class AVCLevel
{
    UNKNOW = 0,
    LEVEL_1 = 10,
    LEVEL_11 = 11,
    LEVEL_12 = 12,
    LEVEL_13 = 13,
    LEVEL_2 = 20,
    LEVEL_21 = 21,
    LEVEL_22 = 22,
    LEVEL_3 = 30,
    LEVEL_31 = 31,
    LEVEL_32 = 32,
    LEVEL_4 = 40,
    LEVEL_41 = 41,
    LEVEL_42 = 42,
    LEVEL_5 = 50,
    LEVEL_51 = 51,
    LEVEL_52 = 52,
};

enum class AVCPayloadFormat
{
    GUESS = 0,
    ANNEXB = 1,
    IBMF = 2
};

enum class AVCNaluType
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

extern std::string profile_to_str(AVCProfile profile);
extern std::string level_to_str(AVCLevel level);
extern int avc_read_bit(BitBufferManager *manager, int8_t &v);
extern int avc_read_uev(BitBufferManager *manager, int32_t &v);

} // namesapace avc

class AVCCodec : public VCodec
{
public:
    AVCCodec();
    virtual ~AVCCodec();
public:
    virtual bool HasSequenceHeader() override;
    int AVCDemux(char *data, int size, CodecSample *sample);
	virtual int DecodeSequenceHeader(BufferManager *manager);
	virtual int DecodecNalu(BufferManager *manager, CodecSample *sample);

public:
    int avc_demux_sps();
    int avc_demux_sps_rbsp(char *rbsp, int nb_rbsp);
    int avc_demux_annexb_format(BufferManager *manager, CodecSample *sample);
    int avc_demux_ibmf_format(BufferManager *manager, CodecSample *sample);
    bool avc_start_with_annexb(BufferManager *manager, int *pnb_start_code);

public:
    int duration;
    int width;
    int height;
    int frame_rate;
    int video_codec_id;
    int video_data_rate;
    int audio_codec_id;
    int audio_data_rate;
    // profile_idc, H.264-AVC-ISO_IEC_14496-10.pdf, page 45
    avc::AVCProfile avc_profile;
    avc::AVCLevel avc_level;
    int8_t nalu_unit_length;
    uint16_t sps_length;
    char *sps;
    uint16_t pps_length;
    char *pps;
    avc::AVCPayloadFormat payload_format;
    // AACObjectType aac_obj_type;
    //samplingFrequencyIndex
    uint8_t aac_sample_rate;
    uint8_t aac_channels;
    int avc_extra_size;
    char *avc_extra_data;
    int aac_extra_size;
    char *aac_extra_data;
    bool avc_parse_sps;
};

#endif
