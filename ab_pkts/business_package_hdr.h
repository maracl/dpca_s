#ifndef _BUSINESS_PACKAGE_H_
#define _BUSINESS_PACKAGE_H_

#include "businesspkg_common.h"

#ifdef __GNUC__
#define BSNPKG_API  extern
#else
#define BSNPKG_API __declspec(dllexport)
#endif

extern "C"{
/**
* @brief : 海信进程级初始化接口
* @param : config_info---初始化配置参数,为json格式
* @param : methods---方法注册回调结构体对象
* @param : errifno---初始化错误信息
* @return: 返回0表示成功
*/
#if 1
BSNPKG_API int process_init(const char* config_info, bp_methodset_t* methods, errinfo_t* errifno);

/**
* @brief : 海信进程级反初始化接口
* @return: void
*/
BSNPKG_API void process_deinit();
#endif
#if 1
/**
 * * @brief : 云从进程级初始化接口
 * * @param : config_info---初始化配置参数,为json格式
 * * @param : methods---方法注册回调结构体对象
 * * @param : errifno---初始化错误信息
 * * @return: 返回0表示成功
 * */
BSNPKG_API int process_init_ocean(const char* config_info, bp_methodset_t* methods, errinfo_t* errifno);

/**
 * * @brief : 云从进程级反初始化接口
 * * @return: void
 * */
BSNPKG_API void process_deinit_ocean();
#endif
#if 1
/**
 * * @brief : 云从进程级初始化接口
 * * @param : config_info---初始化配置参数,为json格式
 * * @param : methods---方法注册回调结构体对象
 * * @param : errifno---初始化错误信息
 * * @return: 返回0表示成功
 * */
BSNPKG_API int process_init_glint(const char* config_info, bp_methodset_t* methods, errinfo_t* errifno);

/**
 * * @brief : 云从进程级反初始化接口
 * * @return: void
 * */
BSNPKG_API void process_deinit_glint();
#endif
}

#endif	// _BUSINESS_PACKAGE_H_
