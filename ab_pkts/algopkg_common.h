#ifndef _ALGO_PKG_COMMON_H_
#define _ALGO_PKG_COMMON_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <opencv2/core/core_c.h>

#define ERRINFO_ERRMSG_MAXLEN 252

struct outputinfo_t;	//通用算法接口输出结构体,用来接收算法接口输出信息

/**
* @brief : 公共方法函数指针传递结构体
*/
typedef struct ap_methodset_t {
	/**
	* @brief : 发生文本消息回调
	* @param : output---算法输出对象
	* @param : msg---文本消息
	* @param : size---文本消息长度
	* @return: void
	*/
	void (*set_text_msg)(outputinfo_t* output, const char* msg, size_t size);
	
	/**
	* @brief : 发生二进制数据回调
	* @param : output---算法输出对象
	* @param : data---二进制数据
	* @param : size---二进制数据长度
	* @return: void
	*/
	void (*set_binary_msg)(outputinfo_t* output, const void* data, size_t size);

}ap_methodset_t;

/**
* @brief : 错误信息结构体,用于API出现错误时传出相关错误信息
*/
typedef struct errinfo_t{
	int errcode;						//错误码
	char errmsg[ERRINFO_ERRMSG_MAXLEN];	//错误信息
}errinfo_t;

/**
* @brief : 线程级gpu初始化输入参数,包含gpu-id、初始化分配资源数量等参数信息
*/
typedef struct threadgpuinfo_t {
	uint32_t _verflag;	//结构体版本与控制位
	int gpu_id;			//GPU id
	const char* param1;	//字符串形式的参数
}threadgpuinfo_t;

/**
* @brief : 用户传出的gputhread_instance实例
*/
typedef struct gputhreadinst_t {
	void* _ptr;
}gputhreadinst_t;

/**
* @brief : 通用图像信息结构体
*/
typedef struct imageinfo_t{
	uint32_t _verflag;	//结构体版本与控制位
	IplImage* p_img;	//图像数据(使用opencv图像格式)
	int64_t pts;		//pts
}imageinfo_t;

/**
* @brief : 通用算法接口输入结构体
*/
typedef struct inputinfo_t{
	uint32_t _verflag;		//结构体版本与控制位
	imageinfo_t** p_imgs;	//imageinfo数组指针
	size_t n_img;			//image个数
	const char*	param1;		//字符串形式参数,为json格式
}inputinfo_t;

/**
* @brief : session初始化参数结构体
*/
typedef struct sessinput_t{
	uint32_t _verflag;	//结构体版本与控制位
	const char* param1;	//字符串形式的参数
}sessinput_t;

/**
* @brief : 用户传出的session_instance实例
*/
typedef struct sessinst_t {
	void* _ptr;
}sessinst_t;
//thankyou666
#endif  //_ALGO_PKG_COMMON_H_
