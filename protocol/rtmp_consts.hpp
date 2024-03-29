#ifndef RS_CONSTS_HPP
#define RS_CONSTS_HPP

#define RTMP_ST_NO_TIMEOUT (int64_t)(-1LL)
#define RTMP_SEND_TIMEOUT_US (int64_t)(30 * 1000 * 1000LL)
#define RTMP_RECV_TIMEOUT_US (int64_t)(30 * 1000 * 1000LL)
//the timeout to wait for client control message
#define RTMP_PULSE_TIMEOUT_US (int64_t)(500 * 1000LL)
#define RTMP_DEFAULT_WINDOW_ACK_SIZE (2.5 * 1000 * 1000)
#define RTMP_DEFAULT_PEER_BAND_WIDTH (2.5 * 1000 * 1000)
//rtmp default port
#define RTMP_DEFAULT_PORT 1935
//rtmp default vhost
#define RTMP_DEFAULT_VHOST_PARAM "?vhost=__defaultVhost__"
#define RTMP_DEFAULT_VHOST "__defaultVhost__"

#define RTMP_CONSTS_RTMP_PROTOCOL_CHUNK_SIZE 128
#define RTMP_DEFALUT_CHUNK_SIZE 128
#define RTMP_CONSTS_RTMP_MIN_CHUNK_SIZE 128
#define RTMP_CONSTS_RTMP_MAX_CHUNK_SIZE 65535

#define RTMP_CHUNK_STREAM_CHCAHE 16

//rtmp timestamp_delta when extended timestamp enabled
#define RTMP_EXTENDED_TIMESTAMP 0xffffffff
//rtmp jitter duration
#define RTMP_MAX_JITTER_MS 250
#define RTMP_MAX_JITTER_MS_NEG -250
#define RTMP_DEFALUT_FRAME_TIME_MS 10
//the timeout to wait for client control message
#define RTMP_PULSE_TIMEOUT_US (int64_t)(500 * 1000LL)
//rtmp marge read small bytes
#define RTMP_MR_SMALL_BYTES 4096
//rtmp perf
#define RTMP_PERF_MW_MSGS 128
#define RTMP_MR_MSGS 128
#define RTMP_MR_MIN_MSGS 8
#define RTMP_MR_SLEEP_MS 350
#define RTMP_IOVS_MAX (RTMP_MR_MSGS * 2)
#define RTMP_C0C3_HEADERS_MAX (RTMP_MR_MSGS * 32)


// rtmp fmt0 header size(max base header)
#define RTMP_FMT0_HEADER_SIZE 16

// rtmp message header type
#define RTMP_FMT_TYPE0 0
#define RTMP_FMT_TYPE1 1
#define RTMP_FMT_TYPE2 2
#define RTMP_FMT_TYPE3 3

// rtmp cid
#define RTMP_CID_PROTOCOL_CONTROL 0x02
#define RTMP_CID_OVER_CONNECTION 0x03
#define RTMP_CID_OVER_CONNECTION2 0x04
#define RTMP_CID_OVER_STREAM 0x05
#define RTMP_CID_OVER_STREAM2 0x08
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

// amf0 command message
#define RTMP_AMF0_COMMAND_CONNECT "connect"
#define RTMP_AMF0_COMMAND_RESULT "_result"

#define RTMP_AMF0_COMMAND_RELEASE_STREAM "releaseStream"
#define RTMP_AMF0_COMMAND_FC_PUBLISH "FCPublish"
#define RTMP_AMF0_COMMAND_UNPUBLISH "FCUnpublish"
#define RTMP_AMF0_COMMAND_PUBLISH "publish"
#define RTMP_AMF0_COMMAND_CREATE_STREAM "createStream"
#define RTMP_AMF0_COMMAND_ON_STATUS "onStatus"
#define RTMP_AMF0_COMMAND_ERROR "error"
#define RTMP_AMF0_COMMAND_ON_FC_PUBLISH "onFCPublish"
#define RTMP_AMF0_COMMAND_ON_FC_UNPUBLISH "onFCUnpublish"
#define RTMP_AMF0_COMMAND_ON_METADATA "onMetaData"
#define RTMP_AMF0_COMMAND_SET_DATAFRAME "@setDataFrame"
#define RTMP_AMF0_COMMAND_PLAY "play"


//amf0 marker
#define RTMP_AMF0_NUMBER 0x00
#define RTMP_AMF0_BOOLEAN 0x01
#define RTMP_AMF0_STRING 0x02
#define RTMP_AMF0_OBJECT 0x03
#define RTMP_AMF0_MOVIE_CLIP 0x04
#define RTMP_AMF0_NULL 0x05
#define RTMP_AMF0_UNDEFINED 0x06
#define RTMP_AMF0_REFENERCE 0x07
#define RTMP_AMF0_ECMA_ARRAY 0x08
#define RTMP_AMF0_OBJECT_END 0x09
#define RTMP_AMF0_STRICT_ARRAY 0x0a
#define RTMP_AMF0_DATE 0x0b
#define RTMP_AMF0_LONG_STRING 0x0c
#define RTMP_AMF0_UNSUPPORTED 0x0d
#define RTMP_AMF0_RECORD_SET 0x0e
#define RTMP_AMF0_XML_DOCUMENT 0x0f
#define RTMP_AMF0_TYPED_OBJECT 0x10
// may be is amf3
#define RTMP_AMF0_AVM_PLUS_OBJECT 0x11
#define RTMP_AMF0_ORIGIN_STRICT_ARRAY 0x20
#define RTMP_AMF0_INVALID 0x3f

// amf0 elem size
#define AMF0_LEN_UTF8(a) (2+(a).length())
#define AMF0_LEN_STR(a) (1 + AMF0_LEN_UTF8(a))
#define AMF0_LEN_NUMBER (1 + 8)
#define AMF0_LEN_DATE (1 + 8 + 2)
#define AMF0_LEN_NULL (1)
#define AMF0_LEN_UNDEFINED (1)
#define AMF0_LEN_BOOLEAN (1 + 1)
#define AMF0_LEN_OBJECT(a) ((a)->TotalSize())
#define AMF0_LEN_OBJ_EOF (2 + 1)
#define AMF0_LEN_ECMA_ARR(a) ((a)->TotalSize())
#define AMF0_LEN_STRICT_ARR(a) ((a)->TotalSize())
#define AMF0_LEN_ANY(a) ((a)->TotalSize())


#endif
