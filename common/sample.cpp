#include <common/sample.hpp>
#include <common/file.hpp>
#include <common/error.hpp>
#include <common/log.hpp>
#include <common/utils.hpp>

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
    // is_video = false;
    // acodec = AudioCodecType::UNKNOW;
    // aac_sample_rate = 0;
    // sound_size = AudioSampleSize::UNKNOW;
    // sound_type = AudioSoundType::UNKNOW;
    // aac_packet_type = AudioPakcetType::UNKNOW;
    nb_sample_units = 0;
    // cts  = 0;
    // has_idr = false;
    // has_sps_pps = false;
    // has_aud = false;
    // first_nalu_type = AVCNaluType::UNKNOW;


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

    // if (is_video)
    // {
    //     AVCNaluType nalu_type = (AVCNaluType)(bytes[0] & 0x1f);
    //     if (nalu_type == AVCNaluType::IDR)
    //     {
    //         has_idr = true;
    //     }
    //     else if (nalu_type == AVCNaluType::SPS || nalu_type == AVCNaluType::PPS)
    //     {
    //         has_sps_pps = true;
    //     }
    //     else if (nalu_type == AVCNaluType::ACCESS_UNIT_DELIMITER)
    //     {
    //         has_aud = true;
    //     }
    //     else
    //     {
    //         // ignore
    //     }

    //     if (first_nalu_type == AVCNaluType::UNKNOW)
    //     {
    //         first_nalu_type = nalu_type;
    //     }
    // }

    return ret;
}
