#ifndef _ALGORITHM_PACKAGE_H_
#define _ALGORITHM_PACKAGE_H_

#include "algopkg_common.h"

#ifdef __GNUC__
#define ALGOPKG_API  extern
#else
#define ALGOPKG_API __declspec(dllexport)
#endif

extern "C"{
	//-------------------- 全局（进程级）API --------------------
	/**
	* @brief : 进程级初始化接口
	* @param : config_info---初始化配置参数,为json格式
	* @param : methods---方法注册回调结构体对象
	* @param : errifno---初始化错误信息
	* @return: 返回0表示成功
	*/
	ALGOPKG_API int process_init(const char* config_info, ap_methodset_t* methods, errinfo_t* errifno);

	/**
	* @brief : 进程级反初始化接口
	* @return: void
	*/
	ALGOPKG_API void process_deinit();

	/**
	* @brief : 全局进程级命令控制(如传递参数、获取状态等)
	* @param : ctrl_info---控制参数
	* @param : output---算法输出对象
	* @param : errifno---错误信息
	* @return: 返回0表示成功
	*/
	ALGOPKG_API int process_ctrl(const char* ctrl_info, outputinfo_t *output, errinfo_t* errinfo);

	//-------------------- 线程级API --------------------
	/**
	* @brief : (算法)线程级初始化
	* @param : init_info---线程级gpu初始化输入参数
	* @param : inst---gputhread_instance实例
	* @param : errifno---初始化错误信息
	* @return: 返回0表示成功
	*/
	ALGOPKG_API int thread_init(threadgpuinfo_t* init_info, gputhreadinst_t* inst, errinfo_t* errinfo);

	/**
	* @brief : 线程级反初始化
	* @param : inst---gputhread_instance实例
	* @return: void
	*/
	ALGOPKG_API void thread_deinit(gputhreadinst_t* inst);


#ifdef _ALGOPKG_STATELESS
	//-------------------- 算法实例API（无状态）--------------------
	/**
	* @brief : 算法调用方法
	* @param : inst---gputhread_instance实例
	* @param : input---算法输入对象
	* @param : output---算法输出对象
	* @return: 返回0表示成功
	*/
	ALGOPKG_API int algorithm_process(gputhreadinst_t* inst, inputinfo_t* input, outputinfo_t* output);

#else
	//-------------------- 算法实例API（有状态）--------------------
	/**
	* @brief : session初始化方法
	* @param : init_info---session初始化参数对象
	* @param : sess_inst---session_instance实例
	* @param : errifno---初始化错误信息
	* @return: 返回0表示成功
	*/
	ALGOPKG_API int session_init(sessinput_t* init_info, sessinst_t* sess_inst, errinfo_t* errinfo);

	/**
	* @brief : session反初始化方法
	* @param : sess_inst---session_instance实例
	* @return: void
	*/
	ALGOPKG_API void session_deinit(sessinst_t* sess_inst);

	/**
	* @brief : session算法调用
	* @param : sess_inst---session_instance实例
	* @param : inst---gputhread_instance实例
	* @param : input---算法输入对象
	* @param : output---算法输出对象
	* @return: 返回0表示成功
	*/
	ALGOPKG_API int sess_algo_process(sessinst_t* sess_inst, gputhreadinst_t* inst, inputinfo_t* input, outputinfo_t* output);

	/**
	* @brief : session命令控制(如传递参数、获取状态等)
	* @param : sess_inst---session_instance实例
	* @param : ctrl_info---控制参数
	* @param : output---算法输出对象
	* @param : errifno---初始化错误信息
	* @return: 返回0表示成功
	*/
	ALGOPKG_API int sess_ctrl(sessinst_t* sess_inst, const char* ctrl_info, outputinfo_t *out, errinfo_t* errinfo);

#endif	// _ALGOPKG_STATELESS

}

#endif	// _ALGORITHM_PACKAGE_H_
