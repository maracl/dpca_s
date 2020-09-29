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

#define MAJOR_VERSION 2		//���汾��
#define MINOR_VERSION 1		//�ΰ汾��
#define PATCH_VERSION 5		//�޶��汾��

#define VERSION_STRING STRINGIFY(MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

//========= log =========
#define START_LOG "startlog" //����������־
#define MAIN_LOG "mainlog"	 //����������־
#define ANAL_LOG "anallog"	 //������־(�ص��עҵ��)
#define TRIFLE_LOG_BC_PT "httplog"	 //

#define LOG_PATH "logs"		 //��־Ŀ¼

#define START_LOG_FILE "logs/wmvs_main.log"
#define MAIN_LOG_FILE "logs/wmvs_run.log"
#define ANAL_LOG_FILE "logs/wmvs_anal.log"
#define HTTP_LOG_FILE "logs/wmvs_http.log"

#define AP_CNN_FACE "AP_CNN_FACE"

#endif
