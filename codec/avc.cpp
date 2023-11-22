#include <codec/avc.hpp>
#include <common/error.hpp>
#include <common/log.hpp>
#include <common/buffer.hpp>
#include <common/utils.hpp>

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

AVCCode::AVCCode()
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
    // aac_obj_type = AACObjectType::UNKNOW;
    // aac_sample_rate = AAC_SAMPLE_RATE_UNSET;
    // aac_channels = 0;
    avc_extra_size = 0;
    avc_extra_data = nullptr;
    aac_extra_size = 0;
    aac_extra_data = nullptr;
    avc_parse_sps = true;

}

AVCCode::~AVCCode()
{
}

int AVCCode::avc_demux_sps_rbsp(char *rbsp, int nb_rbsp)
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
int AVCCode::avc_demux_sps()
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

// int AVCCode::avc_demux_sps_pps(BufferManager *manager)
int AVCCode::DecodeSequenceHeader(BufferManager *manager)
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

bool AVCCode::avc_has_sequence_header()
{
    return avc_extra_size > 0 && avc_extra_data;
}

bool AVCCode::avc_start_with_annexb(BufferManager *manager, int *pnb_start_code)
{
    char *bytes = manager->Data() + manager->Pos();
    char *p = bytes;
    while(true)
    {
        if (!manager->Require(p - bytes + 3))
        {
            return false;
        }
        if (p[0] != (char)0x00 && p[1] != (char)0x00)
        {
            return false;
        }

        if (p[2] == (char)0x01)
        {
            if (pnb_start_code)
            {
                *pnb_start_code = (int)(p - bytes) + 3;
            }
            return true;
        }
        p++;
    }
    return false;
}

int AVCCode::avc_demux_annexb_format(BufferManager *manager, CodecSample *sample)
{
    int ret = ERROR_SUCCESS;

    if (!avc_start_with_annexb(manager, nullptr))
    {
        return ERROR_AVC_TRY_OTHERS;
    }

    while (!manager->Empty())
    {
        int nb_start_code = 0;
        if (!avc_start_with_annexb(manager, &nb_start_code))
        {
            return ret;
        }

        if (nb_start_code > 0)
        {
            manager->Skip(nb_start_code);
        }

        char *p = manager->Data() + manager->Pos();

        while (!manager->Empty())
        {
            if (avc_start_with_annexb(manager, nullptr))
            {
                break;
            }
            manager->Skip(1);
        }

        char *pp = manager->Data() + manager->Pos();
        if (pp - p <= 0)
        {
            continue;
        }
        if ((ret = sample->AddSampleUnit(p, pp - p)) != ERROR_SUCCESS)
        {
            rs_error("avc add video sample failed.ret=%d", ret);
            return ret;
        }
    }
    return ret;
}

int AVCCode::avc_demux_ibmf_format(BufferManager *manager, CodecSample *sample)
{
    int ret = ERROR_SUCCESS;

    int picture_length = manager->Size() - manager->Pos();

    for (int i = 0; i < picture_length; i++)
    {
        if (!manager->Require(nalu_unit_length + 1))
        {
            ret = ERROR_DECODE_H264_FALIED;
            rs_error("avc decode nalu size failed. ret=%d", ret);
            return ret;
        }

        int32_t length = 0;

        if (nalu_unit_length == 3)
        {
            length = manager->Read4Bytes();
        }
        else if (nalu_unit_length == 1)
        {
            length = manager->Read2Bytes();
        }
        else
        {
            length = manager->Read1Bytes();
        }

        if (length < 0)
        {
            ret = ERROR_DECODE_H264_FALIED;
            rs_error("maybe stream is annexb format. ret=%d", ret);
            return ret;
        }

        if (!manager->Require(length))
        {
            ret = ERROR_AVC_TRY_OTHERS;
            return ret;
        }

        if ((ret = sample->AddSampleUnit(manager->Data() + manager->Pos(), length)) != ERROR_SUCCESS)
        {
            rs_error("avc add video sample failed. ret=%d", ret);
        }

        manager->Skip(length);

        i += nalu_unit_length + 1 + length;
    }

    return ret;
}

int AVCCode::DecodecNalu(BufferManager *manager, CodecSample *sample)
{
    int ret = ERROR_SUCCESS;

    if (avc_has_sequence_header())
    {
        rs_warn("avc ignore type=%d for no sequence header", ret);
        return ret;
    }

    if (payload_format == AVCPayloadFormat::GUESS || payload_format == AVCPayloadFormat::ANNEXB)
    {
        if ((ret = avc_demux_annexb_format(manager, sample)) != ERROR_SUCCESS)
        {
            if (ret != ERROR_AVC_TRY_OTHERS)
            {
                rs_error("avc demux annexb failed.ret=%d", ret);
                return ret;
            }
            if ((ret = avc_demux_ibmf_format(manager, sample)) != ERROR_SUCCESS)
            {
                if (ret == ERROR_AVC_TRY_OTHERS)
                {
                    ret = ERROR_DECODE_H264_FALIED;
                }
                rs_error("avc decode nalu failed. ret=%d", ret);
                return ret;
            }
            else
            {
                payload_format = AVCPayloadFormat::IBMF;
            }
        }
        else
        {
            payload_format = AVCPayloadFormat::IBMF;
        }
    }
    else
    {
        if ((ret = avc_demux_ibmf_format(manager, sample)) != ERROR_SUCCESS)
        {
            if (ret != ERROR_AVC_TRY_OTHERS)
            {
                rs_error("avc demux ibmf failed. ret=%d", ret);
                return ret;
            }
            if ((ret = avc_demux_annexb_format(manager, sample)) != ERROR_SUCCESS)
            {
                if (ret == ERROR_AVC_TRY_OTHERS)
                {
                    ret = ERROR_DECODE_H264_FALIED;
                }
                rs_error("avc decode nalu failed. ret=%d", ret);
                return ret;
            }
            else
            {
                payload_format = AVCPayloadFormat::ANNEXB;
            }
        }
        else
        {
            payload_format = AVCPayloadFormat::IBMF;
        }
    }
    return ret;
}
