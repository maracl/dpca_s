#ifndef _BUSINESSPKG_COMMON_H_
#define _BUSINESSPKG_COMMON_H_

#include "algopkg_common.h"	//算法包头文件
#include <string>

struct bp_http_hdl_t;		//http方法session句柄
struct bp_algopkg_t;		//算法包应用实例句柄
struct bp_alogpkg_sess_t;	//算法包应用实例创建的会话session句柄
struct bp_vas_sess_t;		//vas的视频session
struct bp_pre_sess_t;       //pre预览session
struct bp_alg_sess_t;       //algo session


/**
* @brief : 算法包输出数据
*/
typedef struct {
	char* text;			//文本消息
	size_t text_len;	//文本消息长度
	char* binary;		//二进制消息
	size_t binary_len;	//二进制消息长度
#if 1
	imageinfo_t* image_info;  //图像信息
#endif
	int errc;			//错误码
	char* errmsg;		//错误消息
}ap_output_t;

/**
* @brief : 业务包输出数据
*/
typedef struct {
	char* text;			//文本消息,json格式
	size_t text_len;	//文本消息长度
}bp_output_t;

/**
* @brief : 视频数据帧信息
*/
typedef struct {
	imageinfo_t* image_info;  //图像信息
	char* binding_param;	  //绑定参数
	size_t param_size;		  //参数长度
}vi_frame_t;

/**
* @brief : 视频预览返回参数信息
*/
typedef struct {
	char* text;			//文本消息,json格式
	size_t text_len;	//文本消息长度
}pre_output_t;

/**
* @brief : http响应回调函数类型,函数对象
* @param : request---请求内容
* @param : req_size---请求内容长度
* @param : hdl---系统句柄,用于返回等
*/
typedef void (*http_callback_t)(const char* request, size_t req_size, bp_http_hdl_t* hdl);

/**
* @brief : 方法注册回调结构体
*/
typedef struct {
	//------------------------------------ 对外Http接口注册 ------------------------------------
	/**
	* @brief : 注册http响应方法回调
	* @param : url_suffix---请求url,如/video_task/start
	* @param : cb---响应回调函数
	* @return: void
	*/
	void (*register_http_handler)(const char* url_suffix, http_callback_t cb);

	/**
	* @brief : http响应数据写回,配合http_callback_t使用
	* @param : resp_data---响应数据
	* @param : resp_size---响应数据长度
	* @return: void
	*/
	void (*http_write_resp)(bp_http_hdl_t* hdl, const char* resp_data, size_t resp_size);


	//------------------------------------ 视频源操作 ------------------------------------
	/**
	* @brief : 打开视频源
	* @param : open_param---视频源参数,为json格式
	* @return: 视频session
	*/
	bp_vas_sess_t* (*open_video_source)(const char* open_param);

	/**
	* @brief : 关闭视频源
	* @param : sess---视频session
	* @return: void
	*/
	void (*close_video_source)(bp_vas_sess_t* sess);

	/**
	* @brief : 获取视频帧
	* @param : sess---视频session
	* @param : pframe---视频帧
	* @return: 返回0表示成功
	*/
	int (*get_video_frame)(bp_vas_sess_t* sess, vi_frame_t* pframe);

	/**
	* @brief : session控制(如获取进度、控制跳帧等)
	* @param : sess---视频session
	* @param : param---控制参数,为json格式
	* @return: 具体业务输出数据
	*/
	bp_output_t (*video_session_ctrl)(bp_vas_sess_t* sess, const char* param);

	/**
	* @brief : 释放业务输出数据资源
	* @param : output---业务输出数据对象
	* @return: void
	*/
	void (*release_bp_output)(bp_output_t* output);

	/**
	* @brief : 释放视频帧
	* @param : pframe---视频帧
	* @return: void
	*/
	void (*release_video_frame)(vi_frame_t* pframe);

	void (*release_video_src)(bp_vas_sess_t* pb);

    void(*set_dynamic_show)(bp_vas_sess_t* pb, bp_pre_sess_t* st);

	void(*end_dynamic_show)(bp_vas_sess_t* pb);

    void(*begin_alg_send)(bp_vas_sess_t* pb,bp_alg_sess_t* alg);

	pre_output_t (*get_video_info)(bp_vas_sess_t* pb);
	//------------------------------------ 算法包操作 ------------------------------------
	/**
	* @brief : 使用算法包名称获取算法包实例
	* @param : name---算法包名称
	* @return: 算法包实例句柄
	*/
	bp_algopkg_t* (*get_algopkg)(const char* name,int nChan);

	/**
	* @brief : 释放算法包输出数据资源,所有返回ap_output_t的函数,
			   都应该在使用完毕后释放ap_output_t
	* @param : output---算法包输出数据对象
	* @return: void
	*/
	void(*algopkg_output_release)(ap_output_t* output);

	/**
	* @brief : 用于进行全局进程级命令控制(如传递参数、获取状态等)
			   对应了算法包中"process_ctrl"方法
	* @param : param---控制参数,为json格式
	* @return: 算法输出数据
	*/
	ap_output_t(*algopkg_global_ctrl)(bp_algopkg_t* ap_hdl, const char* param);

	/**
	* @brief : 算法调用,对应了算法包中"algorithm_process"方法
	* @param : input---算法输入数据
	* @return: 算法输出数据
	*/
	ap_output_t(*algopkg_alg_process)(bp_algopkg_t* ap_hdl, inputinfo_t* input);


	/**
	* @brief : 获取算法包新session,对应了算法包中"session_init"方法
	* @param : ap_hdl---算法包应用实例句柄
	* @param : param---session参数
	* @return: 算法session句柄,错误时返回NULL
	*/
	bp_alogpkg_sess_t* (*algopkg_new_session)(bp_algopkg_t* ap_hdl, const char* param, int& errorCode, int flag);

	/**
	* @brief : 释放算法包session,对应了算法包中"session_deinit"方法
	* @param : sess---算法session句柄
	* @return: void
	*/
	void (*algopkg_release_session)(bp_alogpkg_sess_t* sess);

	/**
	* @brief : 用于session级命令控制(如传递参数、获取状态等),对应了算法包中"sess_ctrl"方法
	* @param : sess---算法session句柄
	* @param : param---控制参数,为json格式
	* @return: 算法输出数据
	*/
	ap_output_t (*algopkg_sess_ctrl)(bp_alogpkg_sess_t* sess, const char* param);


	/**
	* @brief : 算法调用,对应了算法包中"sess_algo_process"方法
	* @param : sess---算法session句柄
	* @param : input---算法输入数据
	* @return: 算法输出数据
	*/
	ap_output_t (*algopkg_sess_alg_process)(bp_alogpkg_sess_t* sess, inputinfo_t* input);

	ap_output_t (*algopkg_res_process)(bp_algopkg_t* ap_hdl);

	//------------------------------------ 视频预览操作 ------------------------------------
	pre_output_t (*start_pre_sess)(const char* open_param);

	void (*release_preoutput)(pre_output_t* output);

	pre_output_t(*pause_pre_sess)(const char* open_param);

	pre_output_t(*resume_pre_sess)(const char* open_param);

	pre_output_t(*stop_pre_sess)(const char* open_param);

	pre_output_t(*ctrl_pre_sess)(const char* open_param);

	//------------------------------------ 动态演示相关操作 ------------------------------------

	bp_pre_sess_t* (*new_vst_sess)(const char* sessionid);

	void (*release_vstoutput)(bp_pre_sess_t* pb);

	int (*send_vst_sess)(bp_pre_sess_t* pb, inputinfo_t* input);

	void (*stop_vst_sess)(bp_pre_sess_t* pb);

	//------------------------------------ 算法包多连接相关操作 ------------------------------------
	
	bp_alg_sess_t* (*open_algo_source)(const char* open_param, const char* name, int& errorCode);
	
	void (*close_algo_source)(bp_alg_sess_t* sess,bool flag);
	
	void (*send_algo_sess_frame)(bp_alg_sess_t* sess, inputinfo_t * input);
	
	void (*send_algo_frame_nw)(bp_alg_sess_t* sess, inputinfo_t * input);

	ap_output_t(*send_algo_frame)(bp_alg_sess_t* sess, inputinfo_t * input);
	
    void (*release_algo_sess)(bp_alg_sess_t* sess);

	ap_output_t(*recv_algo_resp)(bp_alg_sess_t * sess);

	void (*set_algo_quit)(bp_alg_sess_t* sess);

	void (*set_final_quit)(bp_alg_sess_t* sess);

	void (*gcb_begin_relate)(bp_alg_sess_t* sess,bp_alg_sess_t* pb);

	void (*gcb_end_relate)(bp_alg_sess_t* sess);

	ap_output_t (*gcb_sess_relate_recv)(bp_alg_sess_t* sess,std::string& dtrsp);
        
    //视频转码
        	
    pre_output_t(*start_trans_sess)(const char* open_param);
	pre_output_t(*query_trans_sess)(const char* open_param);

}bp_methodset_t;

#endif	// _BUSINESSPKG_COMMON_H_
