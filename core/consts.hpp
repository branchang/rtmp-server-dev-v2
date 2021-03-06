#ifndef RS_CONSTS_HPP
#define RS_CONSTS_HPP

#define RS_CONSTS_RTMP_SEND_TIMEOUT_US (int64_t)(30 * 1000 * 1000LL)
#define RS_CONSTS_RTMP_RECV_TIMEOUT_US (int64_t)(30 * 1000 * 1000LL)
#define RS_CONSTS_RTMP_PROTOCOL_CHUNK_SIZE 128
#define RS_CONSTS_RTMP_MIN_CHUNK_SIZE 128
#define RS_CONSTS_RTMP_MAX_CHUNK_SIZE 65535

#define RS_CONSTS_CHUNK_STREAM_CHCAHE 16

// rtmp message header type
#define RTMP_FMT_TYPE0 0
#define RTMP_FMT_TYPE1 1
#define RTMP_FMT_TYPE2 2
#define RTMP_FMT_TYPE3 3

// rtmp cid
#define RTMP_CID_PROTOCOL_CONTROL 0x02
#define RTMP_CID_OVER_CONNECTION 0x03
#define RTMP_CID_OVER_CONNECTION2 0x04
#define RTMP_CID_VIDEO 0x06
#define RTMP_CID_AUDIO 0x07


// rtmp timestamp_delta when extended timestamp enabled
#define RTMP_EXTENDED_TIMESTAMPE 0xffffffff

#define SRS_CONSTS_RTMP_PROTOCOL_CHUNK_SIZE 128

//rtmp message type
#define RTMP_MSG_SET_CHUNK_SIZE 0x01
#define RTMP_MSG_ABORT 0x02
#define RTMP_MSG_ACK 0x03
#define RTMP_MSG_USER_CONTROL_MESSAGE 0x04
#define RTMP_MSG_WINDOW_ACK_SIZE 0x05
#define RTMP_MSG_SET_PERR_BANDWIDTH 0x06
#define RTMP_MSG_EDGE_AND_ORIGIN_SERVER_COMMAND 0x07
#define RTMP_MSG_AUDIO_MESSAGE 0x08
#define RTMP_MSG_VIDEO_MESSAGE 0x09
#define RTMP_MSG_AMF3_COMMAND 0x11
#define RTMP_MSG_AMF0_COMMAND 0x14
#define RTMP_MSG_AMF3_DATA 0x0f
#define RTMP_MSG_AMF0_DATA 0x12
#define RTMP_MSG_AMF3_SHARED_OBJ 0x10
#define RTMP_MSG_AMF0_SHARED_OBJ 0x13
#define RTMP_MSG_AGGREGATE 0x16

#endif
