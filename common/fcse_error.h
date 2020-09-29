#ifndef _fcse_error_h_
#define _fcse_error_h_

#define ERROR_OK                    0       // 正确执行
#define ERROR_UNKOWN				-1		// 内部错误
#define ERROR_MEMORY				-2		// 分配内存失败
#define ERROR_INVALID_PARAM			-3		// 参数格式无效
#define ERROR_MISS_PARAM			-4		// 缺少参数
#define ERROR_SESSION_NOT_FOUND		-5		// 会话不存在
#define ERROR_FILE_OPEN_FAILED		-6		// 文件打开失败
#define ERROR_VIDEO_INFO_NOT_FOUND	-7		// 视频信息未找到
#define ERROR_DECODER_NOT_FOUND		-8		// 解码器未找到
#define ERROR_OPEN_DECODER_FAILED	-9		// 打开解码器失败
#define ERROR_RTSP_OPEN_FAILED		-10		// RTSP打开失败
#define ERROR_GPU_RESOURCE_BUSY		-11		// 显卡资源紧张
#define ERROR_ISE					-12		// ISE错误
#define ERROR_TASKID_EXIST			-13		// TaskID重复
#define ERROR_VAS_INIT				-14		// Vas Init Failed
#define ERROR_VSS_INIT				-15		// Vss Init Failed
#define ERROR_TRY_AGAIN				-16		// Try Again
#define ERROR_IMG_URL_INVALID		-17		// 无效的图片路径
#define ERROR_IMG_DOWNLOAD_FAILED	-18		// 图片下载失败

#endif /* !_fcse_error_h_ */
