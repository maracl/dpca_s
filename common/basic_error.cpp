#include "basic_error.h"
#include <unordered_map>


static std::unordered_map<int, std::string>g_error_msg =
{
	//---<vas>--------------------------------------------------------------------
	{ ERROR_VAS_CVT_NOT_INIT, "Cvt Not Initialized" },
	{ ERROR_VAS_OPEN_INPUT_FAILED, "FFmpeg Open Input Failed" },
	{ ERROR_VAS_FIND_STREAM_FAILED, "FFmpeg Find Stream Failed" },
	{ ERROR_VAS_NO_VIDEO_STREAM, "FFmpeg No Video Stream" },
	{ ERROR_VAS_DECODER_NULL, "FFmpeg Decoder Null" },
	{ ERROR_VAS_OPEN_DECODER_FAILED, "FFmpeg Open Decoder Failed" },
	{ ERROR_VAS_FRAME_WIDTH_ZERO, "FFmpeg Frame Width Zero" },
	{ ERROR_VAS_ADDRESS_UNREACHABLE, "RTSP Ping Failed" },
	{ ERROR_VAS_OPTION_REPLY, "RTSP Option Reply Error" },
	{ ERROR_VAS_DESCRIBE_REPLY, "RTSP Describe Reply Error" },
	{ ERROR_VAS_SESSION_NULL, "RTSP Create MediaSession From SDP Failed" },
	{ ERROR_VAS_SDP_NO_MEDIA_LAYER, "RTSP SDP Has No Media Layer" },
	{ ERROR_VAS_NO_AVAILABLE_PORT, "RTSP No Idle Port" },
	{ ERROR_VAS_SETUP_REPLY, "RTSP Describe Setup Error" },
	{ ERROR_VAS_PLAY_REPLY, "RTSP Describe Play Error" },
	{ ERROR_VAS_ALREADY_OPENED, "RTSP Already Opened" },
	{ ERROR_VAS_VIDEO_TYPE_UNKNOWN, "Unknown Video Type" },
	{ ERROR_VAS_ZK_INIT_FAILED, "Vas Init Zookeeper Failed" },
	{ ERROR_VAS_RTSP_START_FAILED, "Vas rtsp start Failed" },
	//---<vss>--------------------------------------------------------------------
	{ ERROR_VSS_GPU_ID_INVALID, "GPU ID Invalid" },
	{ ERROR_VSS_ZK_INIT_FAILED, "VSS Init Zookeeper Failed" },
	{ ERROR_VSS_SEEMMO_INIT_FAILED, "Seemmo Init Failed" },
	{ ERROR_VSS_SESSION_OVER_LIMIT, "Session Count Over Limit" },
	{ ERROR_VSS_SESSION_INIT_FAILED, "Vss Session Init Failed" },
	{ ERROR_VSS_CHANNEL_ID_INVALID, "Channel Id Invalid" },
	//---<wmvs>--------------------------------------------------------------------
	{ ERROR_WMVS_XVID_ENCODE_FAILED, "XVID Encode Failed" },
	{ ERROR_WMVS_NO_MEMNODE, "No Memnode" },
	//---<wmzt>--------------------------------------------------------------------
	{ ERROR_WMZT_FT_INIT_FAILED, "FT_Init_FreeType Failed" },
	{ ERROR_WMZT_FT_NEW_FACE_FAILED, "FT_New_Face Failed" },
	{ ERROR_WMZT_VSID_EMPTY, "Vsid Empty" },
	{ ERROR_WMZT_VSS_CHANNEL_INVALID, "Vss Channel Id Invalid" },
	{ ERROR_WMZT_IMG_DECODE, "Imdecode Failed" },
	{ ERROR_WMZT_SESSION_NOT_FOUND, "Session Not Found" },
	{ ERROR_WMZT_IMG_REC_TYPE_INVALID, "Image Recognize Type Invalid" },
	{ ERROR_WMZT_IMG_BASE64_DECODE_FAILED, "Image Base64 Decode Failed" },
	{ ERROR_WMZT_SOCK_CONNECT_FAILED, "Sock Connect Failed" },
	{ ERROR_WMZT_SOCK_SEND_FAILED, "Sock Send Failed" },
	{ ERROR_WMZT_SOCK_RECV_FAILED, "Sock Recv Failed" },
	{ ERROR_WMZT_SOCK_RECV_OVER_LIMIT, "Sock Recv Over Limit" },
	{ ERROR_WMZT_SOCK_SET_OPT_FAILED, "Sock Setsockopt Failed" },
	{ ERROR_WMZT_SOCK_INVALID, "Sock Invalid" },
	{ ERROR_WMZT_SOCK_BIND_FAILED, "Sock Bind Failed" },
	{ ERROR_WMZT_SOCK_LISTEN_FAILED, "Sock Listen Failed" },
	{ ERROR_WMZT_SET_NET_FAILED, "Set Net Address Failed" },
	{ ERROR_WMZT_OPERATION_PENDING, "Operation Pending" },
	//---<cfg_ini>--------------------------------------------------------------------
	{ ERROR_CFG_PARSE_FILE_FAILED, "Load Config file or parse failed" },
	{ ERROR_CFG_SEC_NOT_FOUND, "Config node can not find" },
	{ ERROR_CFG_VALUE_TYPE, "Config value type error" },
	{ ERROR_CFG_OTHER_UNKNOWN, "Config parse get other error" },

	{ ERROR_LOG_INIT, "Log file init failed" },
	//---<vfs>--------------------------------------------------------------------
	{ ERROR_VFS_GPU_ID_INVALID, "Gpu_id config error"},
	{ ERROR_VFS_DETECT_NOT_FOUND, "detect not found object" },
	{ ERROR_VFS_DETECT_EMEND, "detect and emend unmatched" },
	//---<wmfe>--------------------------------------------------------------------
	{ ERROR_WMFE_ARGUMENT_IS_EMPTY, "argument is empty" },
	{ ERROR_WMFE_INVALID_PARAM, "Invalid Param" },
	{ ERROR_WMFE_INIT_FAILED, "init failed" },
	{ ERROR_WMFE_FONT_OPEN_FAILED, "font open failed" },
	{ ERROR_WMFE_IMAGE_DECODE_FAILED, "image decode failed" },
	{ ERROR_WMFE_INVALID_VFS_CHANNEL, "invalid vfs channel" },
	{ ERROR_WMFE_SCORES_BELOW_THRESHOLD, "score below threshold" },
	{ ERROR_WMFE_FACE_RECOGNIZE_FAILED, "face recognize failed" },
	{ ERROR_WMFE_VFS_FACE_ALARM_ERROE, "VFS face alarm error" },
	{ ERROR_WMFE_IMAGE_SAVE_FAILED, "image save failed" },
	{ ERROR_WMFE_SESSION_NOT_FOUND, "session not found" },
	{ ERROR_WMFE_SESSION_IS_BUSY, "session is busy" },
	{ ERROR_WMFE_SET_NET_FAILED, "set net failed" },
	{ ERROR_WMFE_SOCK_INVALID, "sock invalid" },
	{ ERROR_WMFE_SOCK_BIND_FAILED, "sock bind failed" },
	{ ERROR_WMFE_SOCK_LISTEN_FAILED, "sock listen failed" },
	{ ERROR_WMFE_EXCEPTION, "exception" },
	//---<sqldb>--------------------------------------------------------------------
	{ ERROR_SQLDB_ARGUMENT_IS_EMPTY, "argument is empty" },
	{ ERROR_SQLDB_INVALID_PARAM, "Invalid Param" },
	{ ERROR_SQLDB_ERR_IDX, "error idx" },
	{ ERROR_SQLDB_LACK_OF_BIND_PARAM, "lack of bind-parameter" },
	{ ERROR_SQLDB_RECONNECT_ERROR, "re-connect error" },
	{ ERROR_SQLDB_ALLOCATE_MYSQL_STMT_FAILED, "mysql allocate MYSQL_STMT failed" },
	{ ERROR_SQLDB_UNSUPPORTED_SQLDB_TYPE, "unsupported sqldb type" },
	{ ERROR_SQLDB_EXE_ERROR, "SQL exe error" },
	//---<bytebuffer>--------------------------------------------------------------------
	{ ERROR_BYTEBUFFER_READ_STRSZ, "ERROR_BYTEBUFFER_READ_STRSZ" },
	{ ERROR_BYTEBUFFER_INVALID_STRSZ, "ERROR_BYTEBUFFER_INVALID_STRSZ" },
	{ ERROR_BYTEBUFFER_READ_STR, "ERROR_BYTEBUFFER_READ_STR" },
	{ ERROR_BYTEBUFFER_INVALID_OP, "ERROR_BYTEBUFFER_INVALID_OP" }
};

const char* get_error_msg(int error_code)
{
	auto it = g_error_msg.find(error_code);
	if (it != g_error_msg.end())
	{
		return it->second.c_str();
	}
	else
	{
		return "";
	}
}