#ifndef _UTILITY_METHOD_H_
#define _UTILITY_METHOD_H_
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <opencv2/core/core.hpp>
#include "rapidjson/Rjson.hpp"


using namespace std;

//属性信息
typedef struct att_info_t
{
	int code;

	void sex_code() {
		if (code == -1)
		{
			code = 0;
		}
	}
	bool setCode() { 
		if(code == 0|| code == -1)
		{
			code = 99;
			return true;
		}
		return false;
	}

	void hasOrNot() {
		code == 2 ? code = 0 : code = 1;
	}

	void bike_type_trans() {
		switch (code)
		{
		case 1:
			code = 3;
			break;
		case 2:
			code = 4;
			break;
		case 3:
			code = 2;
			break;
		default:
			break;
		}
	}

	void hairy_style_trans() {
		switch (code)
		{
		case 1:
			code = 13;
			break;
		case 2:
			code = 12;
			break;
		case 3:
			code = 6;
			break;
		default:
			break;
		}
	}

	void age_person_trans() {
		switch (code)
		{
		case  2:
			code = 3;
			break;
		case 3:
			code = 4;
			break;
		default:
			code = 99;
			break;
		}
	}

	void color_correct() {
		switch (code)
		{
		case 13://银
		{
			code = 14;
		}break;
		case 14://金
		{
			code = 15;
		}break;
		case 15://透明
		{
			code = 13;
		}break;
		case 16://黄绿
		{
			code = 99;
		}break;
		default:
			break;
		}
	}

	void angle_trans() {
			if (code == 3|| code == 4)
			{
				code = 2;
			} 
			else if (code == 2)
			{
				code = 3;
			}
	}

	void direction_correct() {
		switch (code)
		{
		case 1:
			code = 11;
			break;
		case 2:
			code = 10;
			break;
		case 3:
			code = 13;
			break;
		case 4:
			code = 12;
			break;
		default:
			code = 9;
			break;
		}
	}

	void translate_plate_color() {
		switch (code)
		{
		case 2:
			code = 0;
			break;
		case 6:
			code = 1;
			break;
		case 5:
			code = 2;
			break;
		case 1:
			code = 3;
			break;
		case 8:
			code = 4;
			break;
		case 16:
			code = 5;
			break;
		default:
			break;
		}
	}

	void translate_plate_type() {
		switch (code)
		{
		case 1:

		default:
			break;
		}
	}


	string name;
	int rect[4];
	att_info_t() {
		code = 99;
		name = "未知";
		rect[0] = rect[1] = rect[2] = rect[3] = 0;
	}
}att_info_t;

//布控报警
typedef struct recog_face_info_t
{
	string id;
	uint64_t pts;
	int score;
	int rect[4];
	att_info_t age;//年龄
	att_info_t gender;//性别
	att_info_t nation;//民族
	att_info_t race;//种族/肤色
	att_info_t emoticon;//表情
	att_info_t mustache;//胡子
	att_info_t cover;//遮挡
	att_info_t cap;//戴帽子

	int face_rect[4];//仅图片结构化关联人脸使用//no uesed here
	int face_score;//仅图片结构化关联人脸使用

	recog_face_info_t()
	{
		pts = -1;
		score = 0;
		rect[0] = rect[1] = rect[2] = rect[3] = 0;
		face_rect[0] = face_rect[1] = face_rect[2] = face_rect[3] = 0;
		face_score = 0;
	}
}recog_face_info_t, *pRecog_face_info_t;

//目标信息
typedef struct obj_info_t
{
	string id;
	int64_t pts;
	int rect[4];
	int ptype;//目标类型
	float score;
	int type;//标志是人骑车还是行人
	cv::Ptr<IplImage> pImg;
	int face_rect[4];//仅图片结构化关联人脸使用
	int face_score;//仅图片结构化关联人脸使用
	string recogInfo;//申瞐最优帧结构化信息json,方便校正提特征
	string dtInfo;//人脸检测信息json,方便校正提特征
	recog_face_info_t faceRecog;//仅用于最优帧人脸属性识别结果的存储
	obj_info_t()
	{
		pts = -1;
		id = "";
		score = 0;
		rect[0] = rect[1] = rect[2] = rect[3] = 0;
		ptype = 0;
		type = -1;
		face_score = 0;
	}
}obj_info_t, *pObj_info_t;


//may throw
int img_resize(const cv::Mat& mat, cv::Mat& res, double& ratio);
//may throw
int img_from_data(const std::string& img_data, cv::Mat& res);

vector<obj_info_t> parse_face_dt_info(const string& rsp_info);
recog_face_info_t parse_face(const Value* face_obj);
std::string get_face_recong(obj_info_t& obj,const string& bind,const string& feat,string& clusterjson);

#endif // _UTILITY_METHOD_H_
