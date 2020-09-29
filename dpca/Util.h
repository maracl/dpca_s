/**
 * @file     :  Util.h
 * @brief    :  for version etc
 * Copyright (c) 2019 by BoyunVision All Rights Reserved *
 *
 * @author   :  miaocs
 * @version  :  v1.0.0
 * @date     :  2019/04/19
 * @history  :
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include "basic_error.h"

//========= version =========
#define STRINGIFY(x) DO_STRINGIFY(x)
#define DO_STRINGIFY(x) #x

#define MAJOR_VERSION 2		//主版本号
#define MINOR_VERSION 1		//次版本号
#define PATCH_VERSION 5		//修订版本号

#define VERSION_STRING STRINGIFY(MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

//========= log =========
#define START_LOG "startlog" //程序启动日志
#define MAIN_LOG "mainlog"	 //程序运行日志
#define ANAL_LOG "anallog"	 //分析日志(重点关注业务)
#define TRIFLE_LOG_BC_PT "httplog"	 //

#define LOG_PATH "logs"		 //日志目录

#define START_LOG_FILE "logs/wmvs_main.log"
#define MAIN_LOG_FILE "logs/wmvs_run.log"
#define ANAL_LOG_FILE "logs/wmvs_anal.log"
#define HTTP_LOG_FILE "logs/wmvs_http.log"

#define AP_CNN_FACE "AP_CNN_FACE"

#endif
