#ifndef _BASIC_ERROR_H_
#define _BASIC_ERROR_H_


#define ERROR_OK		0 // 正确执行
#define ERROR_UNKOWN	1
#define ERROR_INVALID_PARAM 2

//================= vas =================
#define ERROR_VAS_CVT_NOT_INIT 20001 //cvt未初始化
#define ERROR_VAS_OPEN_INPUT_FAILED 20002 //FFmpeg打开文件失败
#define ERROR_VAS_FIND_STREAM_FAILED 20003 //FFmpeg查找流信息失败
#define ERROR_VAS_NO_VIDEO_STREAM 20004 //FFmpeg未找到视频流
#define ERROR_VAS_DECODER_NULL 20005 //FFmpeg解码器为空
#define ERROR_VAS_OPEN_DECODER_FAILED 20006 //FFmpeg打开解码器失败
#define ERROR_VAS_FRAME_WIDTH_ZERO 20007 //FFmpeg查找文件信息失败
#define ERROR_VAS_ADDRESS_UNREACHABLE 20008 //Rtsp地址探测失败
#define ERROR_VAS_OPTION_REPLY 20009 //Rtsp Option回复错误
#define ERROR_VAS_DESCRIBE_REPLY 20010 //Rtsp Describe回复错误
#define ERROR_VAS_SESSION_NULL 20011 //Rtsp MediaSession为空
#define ERROR_VAS_SDP_NO_MEDIA_LAYER 20012 //Rtsp Describe回复SDP没有媒体层
#define ERROR_VAS_NO_AVAILABLE_PORT 20013 //Rtsp 无可用端口
#define ERROR_VAS_SETUP_REPLY 20014 //Rtsp Setup回复错误
#define ERROR_VAS_PLAY_REPLY 20015 //Rtsp Play回复错误
#define ERROR_VAS_ALREADY_OPENED 20016 //RTSP已经打开
#define ERROR_VAS_VIDEO_TYPE_UNKNOWN 20017 //未知视频类型
#define ERROR_VAS_ZK_INIT_FAILED 20018 //Zookeeper连接初始化失败
#define ERROR_VAS_RTSP_START_FAILED 20019 //rtsp地址开启失败

//================= vss =================
#define ERROR_VSS_GPU_ID_INVALID 20101 //GPU ID无效
#define ERROR_VSS_ZK_INIT_FAILED 20102 //Zookeeper连接初始化失败
#define ERROR_VSS_SEEMMO_INIT_FAILED 20103 //Seemmo初始化失败
#define ERROR_VSS_SESSION_OVER_LIMIT 20104 //session数超过上限
#define ERROR_VSS_SESSION_INIT_FAILED 20105 //session初始化失败
#define ERROR_VSS_CHANNEL_ID_INVALID 20106 //channel id无效


//================= wmvs =================
#define ERROR_WMVS_XVID_ENCODE_FAILED 20301 //xvid编码失败
#define ERROR_WMVS_NO_MEMNODE 20302 //没有MemNode

//================= wmzt =================
#define ERROR_WMZT_FT_INIT_FAILED  20401 //FT_Init_FreeType失败
#define ERROR_WMZT_FT_NEW_FACE_FAILED  20402 //FT_New_Face失败
#define ERROR_WMZT_VSID_EMPTY  20403 //vsid未空
#define ERROR_WMZT_VSS_CHANNEL_INVALID  20404 //vss channel无效
#define ERROR_WMZT_IMG_DECODE  20405 //图像解码失败
#define ERROR_WMZT_SESSION_NOT_FOUND 20406 //session不存在
#define ERROR_WMZT_IMG_REC_TYPE_INVALID 20407 //识别类型不存在
#define ERROR_WMZT_IMG_BASE64_DECODE_FAILED 20408 //base64解码图片失败
#define ERROR_WMZT_SOCK_CONNECT_FAILED 20409 //socket 连接失败
#define ERROR_WMZT_SOCK_SEND_FAILED 20410 //socket 发送失败
#define ERROR_WMZT_SOCK_RECV_FAILED 20411 //socket 接收失败
#define ERROR_WMZT_SOCK_RECV_OVER_LIMIT 20412 //socket 接收数据量过大
#define ERROR_WMZT_SOCK_SET_OPT_FAILED 20413 //socket setsockopt失败
#define ERROR_WMZT_SOCK_INVALID 20414 //socket 无效
#define ERROR_WMZT_SOCK_BIND_FAILED 20415 //socket 绑定失败
#define ERROR_WMZT_SOCK_LISTEN_FAILED 20416 //socket 监听失败
#define ERROR_WMZT_SET_NET_FAILED 20417 //设置地址出错
#define ERROR_WMZT_OPERATION_PENDING 20418 //已有操作在进行中

//================= cfg =================
#define ERROR_CFG_PARSE_FILE_FAILED 21601 //配置文件加载解析失败
#define ERROR_CFG_SEC_NOT_FOUND 21602 //配置节点未找到
#define ERROR_CFG_VALUE_TYPE 21603 //值类型错误
#define ERROR_CFG_OTHER_UNKNOWN 21604 //其他未知错误

#define ERROR_LOG_INIT 21611 //日志文件初始化错误

//================= vfs ================= 21701 21800
#define ERROR_VFS_GPU_ID_INVALID 21701 //gpu配置错误
#define ERROR_VFS_DETECT_NOT_FOUND 21702 //未检测到人脸
#define ERROR_VFS_DETECT_EMEND 21703 //检测和校正不匹配
//#define ERROR_VFS_JSON_CREATE 21704 //json 构造错误


//================= wmfe =================
#define ERROR_WMFE_ARGUMENT_IS_EMPTY 20501 // 参数为空
#define ERROR_WMFE_INVALID_PARAM	20502 // 参数无效
#define ERROR_WMFE_INIT_FAILED 20503 // 初始化失败
#define ERROR_WMFE_FONT_OPEN_FAILED 20504 // 字体打开失败
#define ERROR_WMFE_IMAGE_DECODE_FAILED 20505 // 图片解码失败
#define ERROR_WMFE_INVALID_VFS_CHANNEL 20506 // 非法vfs channel
#define ERROR_WMFE_SCORES_BELOW_THRESHOLD 20507 // 分数低于阈值 
#define ERROR_WMFE_FACE_RECOGNIZE_FAILED 20508 // 人脸识别失败 
#define ERROR_WMFE_VFS_FACE_ALARM_ERROE 20509 // VFS抓拍信息错误
#define ERROR_WMFE_IMAGE_SAVE_FAILED 20510 // 图片保存失败
#define ERROR_WMFE_SESSION_NOT_FOUND 20511 // 会话不存在
#define ERROR_WMFE_SESSION_IS_BUSY 20512 // 会话正忙
#define ERROR_WMFE_MSG_RECV_EXP 20513 // MSG_RECV抛出异常
#define ERROR_WMFE_SET_NET_FAILED 20514 // 设置地址出错
#define ERROR_WMFE_SOCK_INVALID 20515 //socket 无效
#define ERROR_WMFE_SOCK_BIND_FAILED 20516 // socket 绑定失败
#define ERROR_WMFE_SOCK_LISTEN_FAILED 20517 //socket 监听失败
#define ERROR_WMFE_EXCEPTION 20518 //底层异常错误
//================= sqldb =================
#define ERROR_SQLDB_ARGUMENT_IS_EMPTY 21001 // 参数为空
#define ERROR_SQLDB_INVALID_PARAM 21002 // 参数无效
#define ERROR_SQLDB_ERR_IDX 21003 // 取结果的列id超出范围
#define ERROR_SQLDB_LACK_OF_BIND_PARAM 21004 // 缺少bind parameter
#define ERROR_SQLDB_RECONNECT_ERROR 21005 // 重复连接
#define ERROR_SQLDB_ALLOCATE_MYSQL_STMT_FAILED 21006 // MYSQL_STMT句柄创建失败
#define ERROR_SQLDB_UNSUPPORTED_SQLDB_TYPE 21007 // 不支持的数据库类型
#define ERROR_SQLDB_EXE_ERROR 21008 // 数据库返回错误


//================= bytebuffer =================
#define ERROR_BYTEBUFFER_READ_STRSZ	21501		
#define ERROR_BYTEBUFFER_INVALID_STRSZ 21502		
#define ERROR_BYTEBUFFER_READ_STR 21503		
#define ERROR_BYTEBUFFER_INVALID_OP	21504	


//================= ise =================
#define ERROR_ISE_OPEN_DB 21301
#define ERROR_ISE_CREATE_DB 21302
#define ERROR_ISE_DELETE_DB 21303
#define ERROR_ISE_ENUM_DB 21304
#define ERROR_ISE_GET_REC_COUNT 21305
#define ERROR_ISE_SELECT_REC_COUNT 21306
#define ERROR_ISE_PUSH_IMAGE_REC 21307
#define ERROR_ISE_RETRIEVE_IMAGE 21308
#define ERROR_ISE_DB_GET_RECORD 21309
#define ERROR_ISE_FEATURE_TYPE 21310
#define ERROR_ISE_FEATURE_SIZE 21311
#define ERROR_ISE_DB_ADD_RECORD 21312
#define ERROR_ISE_SELECT_IMAGE_REC 21313
#define ERROR_ISE_UPDATE_IMAGE_REC 21314
#define ERROR_ISE_UPDATE_IMAGE_REC_WS 21315
#define ERROR_ISE_DELETE_IMAGE_REC 21316
#define ERROR_ISE_DELETE_IMAGE_REC_WS 21317
#define ERROR_ISE_CHUNCK 21318
#define ERROR_ISE_DB_CREATE_DB 21319
#define ERROR_ISE_DB_OPEN_DB 21320
#define ERROR_ISE_DB_DELETE_DB 21321
#define ERROR_ISE_DB_UPDATE_RECORD 21322
#define ERROR_ISE_DB_DELETE_RECORD 21323
#define ERROR_ISE_BATCH 21324
#define ERROR_ISE_CHECK_DB_NAME 21325
#define ERROR_ISE_META 21326
#define ERROR_ISE_OPENMP 21327
#define ERROR_ISE_API_RECV_SIZE 21328
#define ERROR_ISE_API_SET_ISE_ADDR 21328
#define ERROR_ISE_API_SET_EXTR_ADDR 21329

//================= ise_face_ =================
#define ERROR_ISE_FACE_OPEN_DB 21901
#define ERROR_ISE_FACE_CREATE_DB 21902
#define ERROR_ISE_FACE_DELETE_DB 21903
#define ERROR_ISE_FACE_ENUM_DB 21904
#define ERROR_ISE_FACE_GET_REC_COUNT 21905
#define ERROR_ISE_FACE_SELECT_REC_COUNT 21906
#define ERROR_ISE_FACE_PUSH_IMAGE_REC 21907
#define ERROR_ISE_FACE_RETRIEVE_IMAGE 21908
#define ERROR_ISE_FACE_DB_GET_RECORD 21909
#define ERROR_ISE_FACE_FEATURE_TYPE 21910
#define ERROR_ISE_FACE_FEATURE_SIZE 21911
#define ERROR_ISE_FACE_DB_ADD_RECORD 21912
#define ERROR_ISE_FACE_SELECT_IMAGE_REC 21913
#define ERROR_ISE_FACE_UPDATE_IMAGE_REC 21914
#define ERROR_ISE_FACE_UPDATE_IMAGE_REC_WS 21915
#define ERROR_ISE_FACE_DELETE_IMAGE_REC 21916
#define ERROR_ISE_FACE_DELETE_IMAGE_REC_WS 21917
#define ERROR_ISE_FACE_CHUNCK 21918
#define ERROR_ISE_FACE_DB_CREATE_DB 21919
#define ERROR_ISE_FACE_DB_OPEN_DB 21920
#define ERROR_ISE_FACE_DB_DELETE_DB 21921
#define ERROR_ISE_FACE_DB_UPDATE_RECORD 21922
#define ERROR_ISE_FACE_DB_DELETE_RECORD 21923
#define ERROR_ISE_FACE_BATCH 21924
#define ERROR_ISE_FACE_CHECK_DB_NAME 21925
#define ERROR_ISE_FACE_META 21926
#define ERROR_ISE_FACE_OPENMP 21927
#define ERROR_ISE_FACE_INVALID_PARAM 21928
#define ERROR_ISE_FACE_MN_PROGRESS 21929
#define ERROR_ISE_FACE_MN 21930
#define ERROR_ISE_FACE_MN_SESSION_NOT_EXIST 21931
#define ERROR_ISE_FACE_MN_GET_RESULT 21932
#define ERROR_ISE_FACE_GET_FEATURE 21933
#define ERROR_ISE_FACE_MN_STOP 21934
#define ERROR_ISE_FACE_RETRIEVE_IMAGE_WANT_FEATURE 21935
#define ERROR_ISE_FACE_SELECT_IMAGE_REC_WANT_FEATURE 21936

const char* get_error_msg(int error_code);

#endif //!_BASIC_ERROR_H_
