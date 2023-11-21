#include <protocol/flv.hpp>
#include <common/error.hpp>
#include <common/log.hpp>
#include <common/buffer.hpp>
#include <common/utils.hpp>
namespace flv
{

static int avc_read_uev(BitBufferManager *manager, int32_t &v)
{
    int ret = ERROR_SUCCESS;

    if (manager->Empty())
    {
        return ERROR_BIT_BUFFER_MANAGER_EMPTY;
    }

    int leading_zero_bits = -1;
    for(int8_t b = 0; !b && !manager->Empty(); leading_zero_bits++)
    {
        b = manager->ReadBit();
    }
    if (leading_zero_bits > 31)
    {
        return ERROR_BIT_BUFFER_MANAGER_EMPTY;
    }

    v = (1 << leading_zero_bits) - 1;
    for(int i = 0; i < leading_zero_bits; i++)
    {
        int32_t b = manager->ReadBit();
        v += b << (leading_zero_bits - i -1);
    }

    return ret;
}

static int avc_read_bit(BitBufferManager *manager, int8_t &v)
{
    int ret = ERROR_SUCCESS;

    if (manager->Empty())
    {
        return ERROR_BIT_BUFFER_MANAGER_EMPTY;
    }
    v = manager->ReadBit();
    return ret;
}

std::string ACodec2Str(AudioCodecType codec_type)
{
    switch (codec_type)
    {
        case AudioCodecType::AAC:
            return "AAC";
        case AudioCodecType::MP3:
            return "MP3";
        default:
            return "OTHER";
    }
}

std::string AACProfile2Str(AACObjectType object_type)
{
    switch(object_type)
    {
        case AACObjectType::MAIN:
            return "main";
        case AACObjectType::LC:
            return "LC";
        case AACObjectType::SSR:
            return "SSR";
        default:
            return "Other";
    }
}

CodecSampleUnit::CodecSampleUnit()
{
    size = 0;
    bytes = nullptr;
}

CodecSampleUnit::~CodecSampleUnit()
{
}

CodecSample::CodecSample()
{
    Clear();
}

CodecSample::~CodecSample()
{
}

void CodecSample::Clear()
{
    is_video = false;
    acodec = AudioCodecType::UNKNOW;
    aac_sample_rate = 0;
    sound_size = AudioSampleSize::UNKNOW;
    sound_type = AudioSoundType::UNKNOW;
    aac_packet_type = AudioPakcetType::UNKNOW;
    nb_sample_units = 0;
    cts  = 0;
    has_idr = false;
    has_sps_pps = false;
    has_aud = false;
    first_nalu_type = AVCNaluType::UNKNOW;


}

int CodecSample::AddSampleUnit(char *bytes, int size)
{
    int ret = ERROR_SUCCESS;

    if (nb_sample_units >= MAX_CODEC_SAMPLE)
    {
        ret = ERROR_SAMPLE_EXCEED;
        rs_error("codec sample exceed the max count:%d, ret=%d", MAX_CODEC_SAMPLE, ret);
        return ret;
    }

    CodecSampleUnit *sample_unit = &sample_units[nb_sample_units++];
    sample_unit->bytes = bytes;
    sample_unit->size = size;

    if (is_video)
    {
        AVCNaluType nalu_type = (AVCNaluType)(bytes[0] & 0x1f);
        if (nalu_type == AVCNaluType::IDR)
        {
            has_idr = true;
        }
        else if (nalu_type == AVCNaluType::SPS || nalu_type == AVCNaluType::PPS)
        {
            has_sps_pps = true;
        }
        else if (nalu_type == AVCNaluType::ACCESS_UNIT_DELIMITER)
        {
            has_aud = true;
        }
        else
        {
            // ignore
        }

        if (first_nalu_type == AVCNaluType::UNKNOW)
        {
            first_nalu_type = nalu_type;
        }
    }

    return ret;
}

Codec::Codec()
{
    audio_codec_id = 0;
    aac_extra_size = 0;
    aac_extra_data = 0;
    channels = 0;
    sample_rate = AAC_SAMPLE_RATE_UNSET;
    aac_object_type = AACObjectType::UNKNOW;
}

Codec::~Codec()
{
}

bool Codec::IsH264(char *data, int size)
{
    if (size < 1)
    {
        return false;
    }

    char codec_id = data[0] & 0x0f;
    // codec_id &= 0x0f;
    return codec_id == (char)VideoCodecType::AVC;
}

bool Codec::IsAAC(char *data, int size)
{
    if (size < 1)
    {
        return false;
    }

    char codec_id = data[0];
    codec_id = (codec_id >> 4) & 0x0f;
    return codec_id == (char)AudioCodecType::AAC;
}

bool Codec::IsKeyFrame(char *data, int size)
{
    if (size < 1)
    {
        return false;
    }
    char frame_type = data[0];
    frame_type = (frame_type >> 4) & 0x0F;
    return frame_type == (char)VideoFrameType::KEY_FRAME;
}

bool Codec::IsVideoSeqenceHeader(char *data, int size)
{
    if (!IsH264(data, size))
    {
        return false;
    }

    if (size < 2)
    {
        return false;
    }

    char frame_type = data[0];
    frame_type = (frame_type >> 4) & 0x0f;

    char packet_type = data[1];

    return frame_type == (char)VideoFrameType::KEY_FRAME &&
            packet_type == (char)VideoPacketType::SEQUENCE_HEADER;
}

bool Codec::IsAudioSeqenceHeader(char *data, int size)
{
    if (!IsAAC(data, size))
    {
        return false;
    }

    if (size < 2)
    {
        return false;
    }

    char packet_type = data[1];

    return packet_type == (char)AudioPakcetType::SEQUENCE_HEADER;

}

int Codec::aac_sequence_header_demux(char *data, int size)
{
    int ret = ERROR_SUCCESS;

    BufferManager manager;
    if (manager.Initialize(data, size) != ERROR_SUCCESS)
    {
        return ret;
    }

    if (!manager.Require(2))
    {
        ret = ERROR_DECODE_AAC_EXTRA_DATA_FAILED;
        rs_error("decode aac sequene header failed.ret=%d", ret);
        return ret;
    }

    uint8_t object_type = manager.Read1Bytes();
    uint8_t sampling_frequency_index = manager.Read1Bytes();

    channels = (sampling_frequency_index >> 3) & 0xf;
    sampling_frequency_index = ((object_type & 0x07) << 1) | ((sampling_frequency_index >> 7) & 0x01);
    object_type = sampling_frequency_index;
    aac_object_type = (AACObjectType)object_type;

    if (aac_object_type == AACObjectType::HE || aac_object_type == AACObjectType::HEV2)
    {
        ret = ERROR_ACODEC_NOT_SUPPORT;
        rs_error("not support HE/HE2 yet, ret=%d", ret);
        return ret;
    }
    else if (aac_object_type == AACObjectType::UNKNOW)
    {
        ret = ERROR_DECODE_AAC_EXTRA_DATA_FAILED;
        rs_error("decode aac sequence header failed, adts object=%d invalid,ret=%d", object_type, ret);
        return ret;
    }
    else
    {
        //TODO: support HE/HE2. see: ngx_rtmp_codec_parse_acc_header
    }

    return ret;
}

bool Codec::is_aac_codec_ok()
{
    return aac_extra_size > 0 && aac_extra_data;
}

int Codec::DemuxAudio(char *data, int size, CodecSample *sample)
{
    int ret = ERROR_SUCCESS;
    sample->is_video = false;
    if (!data || size <= 0)
    {
        rs_warn("no audio persent, ignore it");
        return ret;
    }

    BufferManager manager;
    if ((ret = manager.Initialize(data, size)) != ERROR_SUCCESS)
    {
        return ret;
    }

    if (!manager.Require(1))
    {
        ret = ERROR_DECODE_AAC_FAILED;
        rs_error("aac decode sound_format failed,ret=%d", ret);
        return ret;
    }

    int sound_format = manager.Read1Bytes();

    int sound_type = sound_format & 0x01;
    int sound_size = (sound_format >> 1) &0x01;
    int sound_rate = (sound_format >> 2) & 0x03;
    sound_format= (sound_format >> 4) & 0x0f;

    audio_codec_id = sound_format;

    sample->acodec = (AudioCodecType)audio_codec_id;
    sample->sound_type = (AudioSoundType)sound_type;
    sample->sound_size = (AudioSampleSize)sound_size;
    sample->flv_sample_rate = (FLVSampleRate)sound_rate;

    if (audio_codec_id == (int) AudioCodecType::MP3)
    {
        return ERROR_ACODEC_TPY_MP3;
    }

    if (audio_codec_id != (int)AudioCodecType::AAC)
    {

        ret = ERROR_DECODE_AAC_FAILED;
        rs_error("only support mp3/aac codec, actual=%d,ret=%d", audio_codec_id, ret);
        return ret;
    }

    if (!manager.Require(1))
    {
        ret = ERROR_DECODE_AAC_FAILED;
        rs_error("aac decode aac_packet_type failed, ret=%d", ret);
        return ret;
    }

    int8_t aac_packet_type = manager.Read1Bytes();
    sample->aac_packet_type = (AudioPakcetType)aac_packet_type;

    if (aac_packet_type == (int8_t)AudioPakcetType::SEQUENCE_HEADER)
    {
        aac_extra_size = manager.Size() - manager.Pos();
        if (aac_extra_size > 0)
        {
            rs_freepa(aac_extra_data);
            aac_extra_data = new char[aac_extra_size];
            memcpy(aac_extra_data, manager.Data() + manager.Pos(), aac_extra_size);

            if ((ret = aac_sequence_header_demux(aac_extra_data, aac_extra_size)) != ERROR_SUCCESS)
            {
                return ret;
            }
        }
    }
    else if (aac_packet_type == (int8_t)AudioPakcetType::RAW_DATA)
    {
        if (!is_aac_codec_ok())
        {
            rs_warn("aac ignore type=%d for no sequence header.ret=%d", aac_packet_type, ret);
            return ret;
        }
        if ((ret = sample->AddSampleUnit(manager.Data() + manager.Pos(), manager.Size() - manager.Pos())) != ERROR_SUCCESS)
        {
            rs_error("add aac sample failed.ret=%d", ret);
            return ret;
        }
    }
    {
        //ignore
    }

    if (sample_rate != AAC_SAMPLE_RATE_UNSET)
    {
        static int aac_sample_rates[] = {
            96000, 88200, 64000,
            48000, 44100, 32000,
            24000, 22050, 16000,
            12000, 11025, 8000,
            0,0,0,0};
        switch (aac_sample_rates[sample_rate])
        {
            case 11025:
                sample->flv_sample_rate = FLVSampleRate::SAMPLE_RATE_11025;
                break;
            case 22050:
                sample->flv_sample_rate = FLVSampleRate::SAMPLE_RATE_22050;
                break;
            case 44100:
                sample->flv_sample_rate = FLVSampleRate::SAMPLE_RATE_44100;
                break;
            default:
                break;
        }
        sample->aac_sample_rate = aac_sample_rates[sample_rate];
    }
    rs_info("aac decoded, type=%d, codec=%d, asize=%d, rate=%d, format=%d, size=%d",
            sound_type, audio_codec_id, sound_size, sound_rate, sound_format, size);
    return ret;
}

Encoder::Encoder()
{
    writer_ = nullptr;
    nb_tag_headers_ = 0;
    tag_headers_ = nullptr;
    nb_ppts_ = 0;
    ppts_ = 0;
    nb_iovss_cache_ = 0;
    iovss_cache_ = nullptr;
}

Encoder::~Encoder()
{
    rs_freepa(iovss_cache_);
    rs_freepa(ppts_);
    rs_freepa(tag_headers_);
}

int Encoder::Initialize(FileWriter *writer)
{
    int ret = ERROR_SUCCESS;
    if (!writer->IsOpen())
    {
        ret = ERROR_KERNEL_FLV_STREAM_CLOSED;
        rs_warn("stream is not open for encoder.ret=%d", ret);
        return ret;
    }
    writer_ = writer;
}

int Encoder::WriteFlvHeader()
{
    int ret = ERROR_SUCCESS;
    char flv_header[] = {
        'F',
        'L',
        'V',
        (char)0x01,
        (char)0x05,
        (char)0x00,
        (char)0x00,
        (char)0x00,
        (char)0x09};
    if ((ret = WriteFlvHeader(flv_header)) != ERROR_SUCCESS)
    {
        return ret;
    }

    return ret;
}

int Encoder::WriteFlvHeader(char flv_header[9])
{
    int ret = ERROR_SUCCESS;
    if ((ret = writer_->Write(flv_header, 9, nullptr)) != ERROR_SUCCESS)
    {
        rs_error("write flv header failed.ret=%d", ret);
        return ret;
    }

    char previous_tag_size[] = {(char)0x00, (char)0x00, (char)0x00, (char)0x00};
    if ((ret = writer_->Write(previous_tag_size, 4, nullptr)) != ERROR_SUCCESS)
    {
        return ret;
    }
    return ret;
}

int Encoder::write_tag_header_to_cache(char type, int size, int timestamp, char *cache)
{
    int ret = ERROR_SUCCESS;

    timestamp &= 0x7fffffff;

    BufferManager  manager;
    if (manager.Initialize(cache, 11) != ERROR_SUCCESS)
    {
        return ret;
    }

    manager.Write1Bytes(type);
    manager.Write3Bytes(size);
    manager.Write3Bytes(type);
    manager.Write1Bytes((timestamp>>24) & 0xff);
    manager.Write3Bytes(0x00);

    return ret;
}

int Encoder::write_previous_tag_size_to_cache(int size, char *cache)
{
    int ret = ERROR_SUCCESS;
    BufferManager manager;
    if ((ret = manager.Initialize(cache, size)) != ERROR_SUCCESS)
    {
        return ret;
    }

    manager.Write4Bytes(size);
    return ret;
}

int Encoder::write_tag(char *header, int header_size, char *tag, int tag_size)
{
    int ret = ERROR_SUCCESS;

    char pre_size[FLV_PREVIOUS_TAG_SIZE];
    if ((ret = write_previous_tag_size_to_cache(header_size + tag_size, pre_size)) != ERROR_SUCCESS)
    {
        return ret;
    }

    iovec iovs[3];
    iovs[0].iov_base = header;
    iovs[0].iov_len = header_size;
    iovs[1].iov_base = tag;
    iovs[1].iov_len = tag_size;
    iovs[2].iov_base = pre_size;
    iovs[2].iov_len = FLV_PREVIOUS_TAG_SIZE;

    if ((ret = writer_->Writev(iovs, 3, nullptr)) != ERROR_SUCCESS)
    {
        if (!IsClientGracefullyClose(ret))
        {
            rs_error("write flv tag failed.ret=%d", ret);
        }
        return ret;
    }
    return ret;
}

int Encoder::WriteMetadata(char *data, int size)
{
    int ret = ERROR_SUCCESS;

    char tag_header[FLV_TAG_HEADER_SIZE];
    if ((ret = write_tag_header_to_cache((char)TagType::SCRIPT, size, 0, tag_header)) != ERROR_SUCCESS)
    {
        return ret;
    }
    if ((ret = write_tag(tag_header, sizeof(tag_header), data, size)) != ERROR_SUCCESS)
    {
        if (!IsClientGracefullyClose(ret))
        {
            rs_error("write flv metadata tag failed.ret=%d", ret);
        }
        return ret;
    }
    return ret;
}

int Encoder::WriteAudio(int64_t timestamp, char *data, int size)
{
    int ret = ERROR_SUCCESS;

    char tag_header[FLV_TAG_HEADER_SIZE];
    if ((ret = write_tag_header_to_cache((char)TagType::AUDIO, size, timestamp, tag_header)) != ERROR_SUCCESS)
    {
        return ret;
    }
    if ((ret = write_tag(tag_header, sizeof(tag_header), data, size)) != ERROR_SUCCESS)
    {
        if (!IsClientGracefullyClose(ret))
        {
            rs_error("write flv audio tag failed.ret=%d", ret);
        }
        return ret;
    }
    return ret;
}

int Encoder::WriteVideo(int64_t timestamp, char *data, int size)
{
    int ret = ERROR_SUCCESS;

    char tag_header[FLV_TAG_HEADER_SIZE];
    if ((ret = write_tag_header_to_cache((char)TagType::VIDEO, size, timestamp, tag_header)) != ERROR_SUCCESS)
    {
        return ret;
    }
    if ((ret = write_tag(tag_header, sizeof(tag_header), data, size)) != ERROR_SUCCESS)
    {
        if (!IsClientGracefullyClose(ret))
        {
            rs_error("write flv video tag failed.ret=%d", ret);
        }
        return ret;
    }
    return ret;
}

int Encoder::SizeTag(int data_size)
{
    return FLV_TAG_HEADER_SIZE + data_size + FLV_PREVIOUS_TAG_SIZE;
}

int Encoder::WriteTags(rtmp::SharedPtrMessage **msgs, int count)
{
    int ret = ERROR_SUCCESS;

    int nb_iovss = 3 * count;
    iovec *iovss = iovss_cache_;

    if (nb_iovss_cache_ < nb_iovss)
    {
        rs_freep(iovss_cache_);
        nb_iovss_cache_ = nb_iovss;
        iovss_cache_ = iovss = new iovec[nb_iovss];
    }

    char *cache = tag_headers_;
    if (nb_tag_headers_ < count)
    {
        rs_freepa(tag_headers_);
        nb_tag_headers_ = count;
        tag_headers_ = cache = new char(FLV_TAG_HEADER_SIZE * count);
    }

    char *pts = ppts_;
    if (nb_ppts_ < count)
    {
        rs_freepa(ppts_);
        nb_ppts_ = count;
        pts = ppts_ = new char[FLV_PREVIOUS_TAG_SIZE * count];
    }

    iovec *iovs = iovss;
    for (int i = 0; i < count; i++)
    {
        rtmp::SharedPtrMessage *msg = msgs[i];
        if (msg->IsAudio())
        {
            if ((ret = write_tag_header_to_cache((char)TagType::AUDIO, msg->size, msg->timestamp, cache)) != ERROR_SUCCESS)
            {
                return ret;
            }
        }
        else if (msg->IsVideo())
        {
            if ((ret = write_tag_header_to_cache((char)TagType::VIDEO, msg->size, msg->timestamp, cache)) != ERROR_SUCCESS)
            {
                return ret;
            }
        }
        else
        {
            if ((ret = write_tag_header_to_cache((char)TagType::SCRIPT, msg->size, msg->timestamp, cache)) != ERROR_SUCCESS)
            {
                return ret;
            }
        }

        if ((ret = write_previous_tag_size_to_cache(FLV_TAG_HEADER_SIZE + msg->size, pts)) != ERROR_SUCCESS)
        {
            return ret;
        }

        iovs[0].iov_base = cache;
        iovs[0].iov_len = FLV_TAG_HEADER_SIZE;
        iovs[1].iov_base = msg->payload;
        iovs[1].iov_len = msg->size;
        iovs[2].iov_base = pts;
        iovs[2].iov_len = FLV_PREVIOUS_TAG_SIZE;

        cache += FLV_TAG_HEADER_SIZE;
        pts += FLV_PREVIOUS_TAG_SIZE;
        iovs += 3;
    }
    if ((ret = writer_->Writev(iovss, nb_iovss, nullptr)) != ERROR_SUCCESS)
    {
        if (!IsClientGracefullyClose(ret))
        {
            rs_error("write flv tags failed. ret=%d", ret);
        }
        return ret;
    }
    return ret;

}

AVInfo::AVInfo()
{
    duration = 0;
    width = 0;
    height = 0;
    frame_rate = 0;
    video_codec_id = 0;
    video_data_rate = 0;
    audio_codec_id = 0;
    audio_data_rate = 0;
    avc_profile = AVCProfile::UNKNOW;
    avc_level = AVCLevel::UNKNOW;
    nalu_unit_length = 0;
    sps_length = 0;
    sps = nullptr;
    pps_length = 0;
    pps = nullptr;
    payload_format = AVCPayloadFormat::GUESS;
    aac_obj_type = AACObjectType::UNKNOW;
    aac_sample_rate = AAC_SAMPLE_RATE_UNSET;
    aac_channels = 0;
    avc_extra_size = 0;
    avc_extra_data = nullptr;
    aac_extra_size = 0;
    aac_extra_data = nullptr;
    avc_parse_sps = true;

}

AVInfo::~AVInfo()
{
}

int AVInfo::avc_demux_sps_rbsp(char *rbsp, int nb_rbsp)
{
    //for sps ,7.3.2.1.1 Sequence parameter set data syntax
    int ret = ERROR_SUCCESS;

    if (!avc_parse_sps)
    {
        return ret;
    }

    BufferManager manager;
    if (manager.Initialize(rbsp, nb_rbsp) != ERROR_SUCCESS)
    {
        return ret;
    }

    if (!manager.Require(3))
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sps_rbsp failed.ret=%d", ret);
        ret = ERROR_DECODE_H264_FALIED;
        return ret;
    }

    uint8_t profile_idc = manager.Read1Bytes();
    if (!profile_idc)
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sps_rbsp failed.ret=%d", ret);
        return ret;
    }

    uint8_t flags = manager.Read1Bytes();
    if (flags & 0x03)
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sps_rbsp failed.ret=%d", ret);
        return ret;
    }

    uint8_t level_idc = manager.Read1Bytes();
    if (!level_idc)
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sps_rbsp failed.ret=%d", ret);
        return ret;
    }

    BitBufferManager bbm;
    if ((ret = bbm.Initialize(&manager)) != ERROR_SUCCESS)
    {
        return ret;
    }

    int32_t seq_parameter_set_id = -1;
    if ((ret = avc_read_uev(&bbm, seq_parameter_set_id)) != ERROR_SUCCESS)
    {
        return ret;
    }

    if (seq_parameter_set_id < 0)
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sps_rbsp failed.ret=%d", ret);
        return ret;
    }

    int32_t chroma_format_idc = -1;
    int8_t separate_colour_plane_flag = 0;

    if (profile_idc == 100 ||
        profile_idc == 110 ||
        profile_idc == 122 ||
        profile_idc == 144 ||
        profile_idc == 44 ||
        profile_idc == 83 ||
        profile_idc == 86 ||
        profile_idc == 118 ||
        profile_idc == 128)
    {
        if ((ret = avc_read_uev(&bbm ,chroma_format_idc)) != ERROR_SUCCESS)
        {
            return ret;
        }

        if (chroma_format_idc == 3)
        {
            if ((ret = avc_read_bit(&bbm, separate_colour_plane_flag)) != ERROR_SUCCESS)
            {
                return ret;
            }
        }

        int32_t bit_depth_luma_minus8 = -1;
        if ((ret = avc_read_uev(&bbm, bit_depth_luma_minus8)) != ERROR_SUCCESS)
        {
            return ret;
        }

        int8_t qpprime_y_zero_transform_bypass_flag = -1;
        if ((ret = avc_read_bit(&bbm, qpprime_y_zero_transform_bypass_flag)) != ERROR_SUCCESS)
        {
            return ret;
        }

        int8_t seq_scaling_matrix_present_flag = -1;
        if ((ret = avc_read_bit(&bbm, seq_scaling_matrix_present_flag)) != ERROR_SUCCESS)
        {
            return ret;
        }

        if (seq_scaling_matrix_present_flag)
        {
            int nb_scmpfs = ((chroma_format_idc != 3)? 8 : 12);
            for (int i = 0; i < nb_scmpfs; i++)
            {
                int8_t seq_scaling_matrix_present_flag_i = -1;
                if ((ret = avc_read_bit(&bbm, seq_scaling_matrix_present_flag_i)) != ERROR_SUCCESS)
                {
                    return ret;
                }
            }
        }
    }

    int32_t log2_max_frame_num_minus4 = -1;
    if ((ret = avc_read_uev(&bbm, log2_max_frame_num_minus4)) != ERROR_SUCCESS)
    {
        return ret;
    }

    int32_t pic_order_cnt_type = -1;
    if ((ret = avc_read_uev(&bbm, pic_order_cnt_type)) != ERROR_SUCCESS)
    {
        return ret;
    }

    if (pic_order_cnt_type == 0)
    {
        int32_t log2_max_pic_order_cnt_lsb_minus4 = -1;
        if ((ret = avc_read_uev(&bbm, log2_max_pic_order_cnt_lsb_minus4)) != ERROR_SUCCESS)
        {
            return ret;
        }
    }else if (pic_order_cnt_type == 1)
    {
        int8_t delta_pic_order_always_zero_flag = -1;
        if ((ret = avc_read_bit(&bbm, delta_pic_order_always_zero_flag)) != ERROR_SUCCESS)
        {
            return ret;
        }

        int32_t offset_for_non_ref_pic = -1;
        if ((ret = avc_read_uev(&bbm, offset_for_non_ref_pic)) != ERROR_SUCCESS)
        {
            return ret;
        }

        int32_t offset_for_top_to_bottom_field = -1;
        if ((ret = avc_read_uev(&bbm, offset_for_top_to_bottom_field)) != ERROR_SUCCESS)
        {
            return ret;
        }

        int32_t num_ref_frames_in_pic_order_cnt_cycle = -1;
        if ((ret = avc_read_uev(&bbm, num_ref_frames_in_pic_order_cnt_cycle)) != ERROR_SUCCESS)
        {
            return ret;
        }
        if (num_ref_frames_in_pic_order_cnt_cycle < 0)
        {
            ret = ERROR_DECODE_H264_FALIED;
            rs_error("decode sps_rbsp failed.ret=%d", ret);
            return ret;
        }

        for (int i = 0;i < num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            int32_t offset_for_ref_frame_i = -1;
            if ((ret = avc_read_uev(&bbm, offset_for_ref_frame_i)) != ERROR_SUCCESS)
            {
                return ret;
            }
        }
    }

    int32_t max_num_ref_frames = -1;
    if ((ret = avc_read_uev(&bbm, max_num_ref_frames)) != ERROR_SUCCESS)
    {
        return ret;
    }

    int8_t gaps_in_frame_num_value_allowed_flag = -1;
    if ((ret = avc_read_bit(&bbm, gaps_in_frame_num_value_allowed_flag)) != ERROR_SUCCESS)
    {
        return ret;
    }

    int32_t pic_width_in_mbs_minus1 = -1;
    if ((ret = avc_read_uev(&bbm, pic_width_in_mbs_minus1)) != ERROR_SUCCESS)
    {
        return ret;
    }

    int32_t pic_height_in_map_units_minus1 = -1;
    if ((ret = avc_read_uev(&bbm, pic_height_in_map_units_minus1)) != ERROR_SUCCESS)
    {
        return ret;
    }

    int8_t frame_mbs_only_flag = -1;
    int8_t mb_adaptive_frame_field_flag = -1;
    int8_t direct_8x8_inference_flag = -1;

    if ((ret = avc_read_bit(&bbm, frame_mbs_only_flag)) != ERROR_SUCCESS)
    {
        return ret;
    }

    if ((ret = avc_read_bit(&bbm, mb_adaptive_frame_field_flag)) != ERROR_SUCCESS)
    {
        return ret;
    }

    if ((ret = avc_read_bit(&bbm, direct_8x8_inference_flag)) != ERROR_SUCCESS)
    {
        return ret;
    }

    int8_t frame_cropping_flag;
    int32_t frame_crop_left_offset = 0;
    int32_t frame_crop_right_offset = 0;
    int32_t frame_crop_top_offset = 0;
    int32_t frame_crop_bottom_offset = 0;
    if ((ret = avc_read_bit(&bbm, frame_cropping_flag)) != ERROR_SUCCESS)
    {
        return ret;
    }
    if (frame_cropping_flag)
    {
        if ((ret = avc_read_uev(&bbm, frame_crop_left_offset)) != ERROR_SUCCESS)
        {
            return ret;
        }

        if ((ret = avc_read_uev(&bbm, frame_crop_right_offset)) != ERROR_SUCCESS)
        {
            return ret;
        }

        if ((ret = avc_read_uev(&bbm, frame_crop_top_offset)) != ERROR_SUCCESS)
        {
            return ret;
        }

        if ((ret = avc_read_uev(&bbm, frame_crop_bottom_offset)) != ERROR_SUCCESS)
        {
            return ret;
        }
    }

    width = 16  * (pic_width_in_mbs_minus1 + 1);
    height = 16 * (2 - frame_mbs_only_flag) * (pic_height_in_map_units_minus1 + 1);

    if (separate_colour_plane_flag || chroma_format_idc == 0)
    {
        frame_crop_bottom_offset *= (2 - frame_mbs_only_flag);
        frame_crop_top_offset *= (2 - frame_mbs_only_flag);
    }
    else if (!separate_colour_plane_flag && chroma_format_idc > 0)
    {
        // width multipliers for formats 1 (4:2:0) and 2 (4:2:2)
        if (chroma_format_idc == 1 || chroma_format_idc == 2)
        {
            frame_crop_left_offset *= 2;
            frame_crop_right_offset *= 2;
        }
        // height multipliers for formats 1 (4:2:0)
        if (chroma_format_idc == 1)
        {
            frame_crop_top_offset *= 2;
            frame_crop_bottom_offset *= 2;
        }
    }

    // Subtract the crop for each dimension.
    width -= (frame_crop_left_offset + frame_crop_right_offset);
    width -= (frame_crop_left_offset + frame_crop_bottom_offset);

    rs_trace("sps parse,width=%d, height=%d, profile=%d, level=%d, sps_id", width, height, profile_idc, level_idc, seq_parameter_set_id);

    return ret;
}

// TODO
int AVInfo::avc_demux_sps()
{
    int ret = ERROR_SUCCESS;
    if (!sps_length)
    {
        return ret;
    }

    BufferManager manager;
    if (manager.Initialize(sps, sps_length) != ERROR_SUCCESS)
    {
        return ret;
    }

    if (!manager.Require(1))
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sps failed. ret=%d", ret);
        return ret;
    }

    int8_t nutv = manager.Read1Bytes();

    AVCNaluType nalu_unit_type = AVCNaluType(nutv & 0x1f);
    if (nalu_unit_type != AVCNaluType::SPS)
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sps failed. ret=%d", ret);
        return ret;
    }

    int8_t *rbsp = new int8_t[sps_length];
    rs_auto_freea(int8_t, rbsp);

    int nb_rbsp = 0;
    while (!manager.Empty())
    {
        rbsp[nb_rbsp] = manager.Read1Bytes();
        if (nb_rbsp > 2 &&
            rbsp[nb_rbsp - 2] == 0x00 &&
            rbsp[nb_rbsp - 1] == 0x00 &&
            rbsp[nb_rbsp] == 0x03)
        {
            if (!manager.Empty())
            {
                if (!manager.Empty())
                {
                    rbsp[nb_rbsp] = manager.Read1Bytes();
                    nb_rbsp++;
                    continue;
                }
            }
            nb_rbsp++;
        }
    }

    return avc_demux_sps_rbsp((char *)rbsp, nb_rbsp);
}

int AVInfo::avc_demux_sequence_header(BufferManager *manager)
{
    int ret = ERROR_SUCCESS;
    avc_extra_size = manager->Size() - manager->Pos();

    if (avc_extra_size > 0)
    {
        rs_freepa(avc_extra_data);
        avc_extra_data = new char[avc_extra_size];
        memcpy(avc_extra_data, manager->Data() + manager->Pos(), avc_extra_size);
    }

    if (!manager->Require(6))
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sequence header failed.ret=%d", ret);
        return ret;
    }

    manager->Read1Bytes();
    avc_profile = (AVCProfile)manager->Read1Bytes();
    manager->Read1Bytes();
    avc_level = (AVCLevel)manager->Read1Bytes();

    int8_t length_size_minus_one = manager->Read1Bytes();
    length_size_minus_one &= 0x03;

    nalu_unit_length = length_size_minus_one;
    if (nalu_unit_length == 2)
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("sequence should never be 2.ret=%d", ret);
        return ret;
    }

    if (!manager->Require(1))
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sequence header failed.ret=%d", ret);
        return ret;
    }

    int8_t num_of_sps = manager->Read1Bytes();
    num_of_sps &= 0x1f;
    if (num_of_sps != 1)
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sequence header failed.ret=%d", ret);
        return ret;
    }

    if (!manager->Require(2))
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sequence header failed.ret=%d", ret);
        return ret;
    }

    sps_length = manager->Read2Bytes();
    if (!manager->Require(sps_length))
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sequence header failed.ret=%d", ret);
        return ret;
    }

    if (sps_length > 0)
    {
        rs_freepa(sps);
        sps = new char[sps_length];
        manager->ReadBytes(sps,sps_length);
    }

    if (!manager->Require(1))
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sequence header failed.ret=%d", ret);
        return ret;
    }

    int8_t num_of_pps = manager->Read1Bytes();
    num_of_pps &= 0x1f;

    if (num_of_pps != 1)
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sequence header failed.ret=%d", ret);
        return ret;
    }

    pps_length = manager->Read2Bytes();
    if (!manager->Require(pps_length))
    {
        ret = ERROR_DECODE_H264_FALIED;
        rs_error("decode sequence header failed.ret=%d", ret);
        return ret;
    }

    if (pps_length > 0)
    {
        rs_freepa(pps);
        pps = new char[pps_length];
        manager->ReadBytes(pps, pps_length);
    }

    return avc_demux_sps();
}


int AVInfo::AVCDemux(char *data, int size, CodecSample *sample)
{
    int ret = ERROR_SUCCESS;

    sample->is_video = true;

    if (!data || size <= 0)
    {
        return ret;
    }

    BufferManager manager;
    if ((ret = manager.Initialize(data, size)) != ERROR_SUCCESS)
    {
        return ret;
    }

    if (!manager.Require(1))
    {
        ret = ERROR_DECODE_FLV_FAILED;
        rs_error("decode frame_type failed. ret=%d", ret);
        return ret;
    }

    int8_t frame_type = manager.Read1Bytes();
    int8_t codec_id = frame_type & 0x0f;
    frame_type = (frame_type >> 4) & 0x0f;

    sample->frame_type = (VideoFrameType)frame_type;

    if (sample->frame_type == VideoFrameType::VIDEO_INFO_FRAME)
    {
        rs_warn("ignore the info frame");
        return ret;
    }

    if (codec_id != (int8_t)VideoCodecType::AVC);
    {
        ret = ERROR_DECODE_FLV_FAILED;
        rs_error("only support h264/avc codec. actual=%d, ret=%d", codec_id, ret);
        return ret;
    }

    video_codec_id = codec_id;
    if (!manager.Require(4))
    {
        ret = ERROR_DECODE_FLV_FAILED;
        rs_error("decode avc_packet_type failed. ret=%d", ret);
        return ret;
    }

    int8_t  acv_packet_type = manager.Read1Bytes();
    int32_t composition_time = manager.Read3Bytes();

    sample->cts = composition_time;
    sample->avc_packet_type = (VideoPacketType)acv_packet_type;

    if (acv_packet_type == (int8_t)VideoPacketType::SEQUENCE_HEADER)
    {
        if ((ret = avc_demux_sequence_header(&manager)))
        {
            return ret;
        }
    }
    else if (acv_packet_type == (int8_t)VideoPacketType::NALU)
    {

    }
    else
    {
        // ignore
    }

    return ret;
}


} // flv
