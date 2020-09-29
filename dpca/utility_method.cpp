#include "utility_method.h"
#include "rapidjson/Rjson.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <math.h>
#include "Util.h"
#include "system.h"
#include "create_uuid.h"
#include "dict_translate.h"
#include "byte_buffer.h"
#include "StreamCmdApis.h"
#include "spdlog_helper.h"
#include "ise_proxy.h"

extern std::string ise_ip;
extern int ise_port;
extern std::vector<string> ise_dbname;
extern string g_VidStr;

string litos_vs(int64_t v)
{
	string ss;
	format_string(ss, "%lld", v);
	return ss;
}
std::string auto_codeTouuid(int64_t v)
{
	string stmp = create_uuid(false);
	string ss;
	format_string(ss, "Bsd%lld", v);
	stmp = stmp.substr(0, stmp.length() - ss.length());
	stmp = stmp + ss;
	return stmp;
}

std::string auto_codeTovid(int64_t v)
{
	string stmp = g_VidStr;
	string ss;
	format_string(ss, "%lld", v);
	stmp = stmp.substr(0, stmp.length() - ss.length());
	stmp = stmp + ss;
	return stmp;
}
int img_resize(const cv::Mat& mat, cv::Mat& resImg, double& ratio)
{
	if (!mat.data)
		throw GeneralException2(-1, "mat is invalid");
	int w = mat.cols;
	int h = mat.rows;
	ratio = 1.000f;
	if (w > 1920 || h > 1080)
	{
		double ratCol = (double)w / 1920.000f;
		double ratRow = (double)h / 1080.000f;
		ratio = ratCol > ratRow ? ratCol : ratRow;
		int cols = (int)((double)w / ratio);
		int rows = (int)((double)h / ratio);
		cv::Size resImgSize = cv::Size(cols, rows);
		resImg = cv::Mat(resImgSize, mat.type());
		cv::resize(mat, resImg, resImgSize, CV_INTER_CUBIC);
	}
	else
	{
		resImg = mat;
	}
	return 0;
}


int img_from_data(const std::string& img_data, cv::Mat& res)
{
	if (img_data.empty())
		throw GeneralException2(-1, "data is empty");
	cv::Mat mat(cv::imdecode(cv::Mat(1, img_data.size(), CV_8UC3, (unsigned char*)img_data.data()), CV_LOAD_IMAGE_COLOR));
	if (!mat.data)
		throw GeneralException2(-1, "image decode error");
	res = mat;
	return 0;
}


recog_face_info_t parse_face(const Value* face_obj)
{
	recog_face_info_t info;
	info.age.code = Rjson::GetInt("age", "code", face_obj);
	info.age.name = Rjson::GetString("age", "name", face_obj);

	info.gender.code = Rjson::GetInt("gender", "code", face_obj);
	info.gender.name = Rjson::GetString("gender", "name", face_obj);

	info.nation.code = Rjson::GetInt("nation", "code", face_obj);
	info.nation.name = Rjson::GetString("nation", "name", face_obj);

	info.race.code = Rjson::GetInt("race", "code", face_obj);
	info.race.name = Rjson::GetString("race", "name", face_obj);

	info.emoticon.code = Rjson::GetInt("emoticon", "code", face_obj);
	info.emoticon.name = Rjson::GetString("emoticon", "name", face_obj);

	info.mustache.code = Rjson::GetInt("mustache", "code", face_obj);
	info.mustache.name = Rjson::GetString("mustache", "name", face_obj);

	info.cover.code = Rjson::GetInt("cover", "code", face_obj);
	info.cover.name = Rjson::GetString("cover", "name", face_obj);

	info.cap.code = Rjson::GetInt("cap", "code", face_obj);
	info.cap.name = Rjson::GetString("cap", "name", face_obj);

	return info;
}


vector<obj_info_t> parse_face_dt_info(const string& rsp_info)
{
	vector<obj_info_t>vInfos;
	try
	{
		if (rsp_info.empty())
		{
			LOG_ERROR(MAIN_LOG,"{}", "------>face detect error, rsp is empty");
			return vInfos;
		}
		Document dc_f;
		Rjson::Parse(dc_f, rsp_info);
		int errCode = Rjson::GetInt("code", &dc_f);
		if (errCode != 0) return vInfos;

		const Value* obj_img = Rjson::GetObject("results", 0, &dc_f);
		const Value* obj_infos = Rjson::GetArray("obj_infos", obj_img);
		for (int k = 0; k < obj_infos->Size(); ++k)
		{
			const Value* obj = Rjson::GetObject(k, obj_infos);
			obj_info_t info;
			recog_face_info_t finfo;
			for (int m = 0; m < 4; ++m)
			{
				info.rect[m] = (int)Rjson::GetFloat("rect", m, obj);
			}
#if 0
			Cordinate origin = { 0 };
			origin.left_top_x = info.rect[0];
			origin.left_top_y = info.rect[1];
			origin.width = info.rect[2];
			origin.height = info.rect[3];
			Cordinate after = { 0 };
			CordinateConverse(pMat, &origin, after);
			info.rect[0] = after.left_top_x;
			info.rect[1] = after.left_top_y;
			info.rect[2] = after.width;
			info.rect[3] = after.height;
#endif
			//info.score = (int)(Rjson::GetFloat("score", obj) * 100);//liuhao 20191009 对返回的分数先*100，后转换为整数
			info.score = Rjson::GetFloat("score", obj);
// 			if (info.score < 80)
// 			{
// 				continue;
// 			}
			info.dtInfo = Rjson::ToString(obj);
			try {
				const Value* ob = NULL;
				Rjson::GetObjectV(&ob, "recognize", obj);
				finfo = parse_face(ob);
				info.faceRecog = finfo;
			}
			catch (GeneralException2& e)
			{
				LOG_ERROR(MAIN_LOG,"{}", "parse_ob_false");
			}
			info.ptype = 4;
			vInfos.push_back(info);
		}
	}
	catch (GeneralException2& e)
	{
		LOG_ERROR(MAIN_LOG,"parse_face_dt_info error，code:{},msg:{},rsp:{}", e.err_code(), e.err_msg(), rsp_info);
	}
	return vInfos;
}


std::string get_face_recong(obj_info_t& obj,const string& bind,const string& feat,string& clusterjson)
{
    //if(bind.empty()) return string("");
	//if(feat.empty()) return string("");
    LOG_INFO(MAIN_LOG, "push a lot cos {}!", bind);
    Document dc;
    Rjson::Parse(dc,bind);
    int64_t snapTime = Rjson::GetInt64("snaptime",&dc);
    dc.RemoveMember("snaptime");
    string deviceId  = Rjson::GetString("DeviceId",&dc);
    string taskId   = Rjson::GetString("TaskId",&dc);
    string uuid   = Rjson::GetString("UUID",&dc);
    int    direction = 99;
    if(dc.HasMember("Direction"))
    direction = Rjson::GetInt("Direction",&dc);
    //string longitude  = Rjson::GetString("longitude",0,&dc);
    //string latitude  = Rjson::GetString("latitude",0,&dc);
    string face_obj_url = Rjson::GetString("SmallImgUrl",&dc);
    string face_bkg_url = Rjson::GetString("BigImgUrl",&dc);
    //
	string face_rect = fmt::format("{}/{}/{}/{}", obj.rect[0], obj.rect[1], obj.rect[2], obj.rect[3]);
	string value_fields2;
	format_string(value_fields2,
		"%s,%s,%s,%s,%lld,%lld,%s,%s,%d,%s,%d,%s,%s,%s,%s,%s,"
		"%s,%lld,%lld,%s,%d,%d,%s,%s,%d,%d,%d,%d,"
		"%d,%d,%d,%d,%d,%d,%d,%d,%d",
		"",
		"",
		"",
		"",
		obj.pts,
		obj.pts,
		"",
		"",
		2,
		uuid.data(),
		6,
		taskId.data(),
		deviceId.data(),
		"",
		"",
		"",
		"",
		0,
		0,
		face_bkg_url.data(),
		1920,//
		1080,
		face_obj_url.data(),
		face_rect.data(),
		99,//angle?
		direction,//direction?
		obj.faceRecog.age.code,
		face_cap_translate(obj.faceRecog.mustache.code),
		obj.faceRecog.race.code,
		face_glass_translate(obj.faceRecog.cover.code),
		obj.faceRecog.nation.code,
		face_gender_translate(obj.faceRecog.gender.code),
		face_ems_translate(obj.faceRecog.emoticon.code),
		obj.faceRecog.cover.code,
		face_cap_translate(obj.faceRecog.cap.code),
		face_Respirator_translate(obj.faceRecog.cover.code),
		face_Eyebrow_translate(obj.faceRecog.cover.code));//zj 20191108 增加戴帽子转义

	LOG_INFO(MAIN_LOG,"push ise face value_fields:{}", value_fields2);
	string db_name = ise_dbname[4];
	IdxPac _idcPac;
	_idcPac = ise_push_data(db_name, eTypeFace, feat, value_fields2,true);
	int64_t cdx = _idcPac.cluster_idx;
	string scdx = auto_codeTovid(cdx);
	string sidx = auto_codeTouuid(_idcPac.idx);
#if 0
	Document json = Rjson::rWriteDC();
	Rjson::rAdd(json,"InterfaceType", StringRef("HI-F"));
	Rjson::rAdd(json,"InterfaceVersion", StringRef("1.0"));
	//Rjson::rAdd(json,"FaceID",uuid);
	//Rjson::rAdd(json,"ManufacturerId",);
	//Rjson::rAdd(json,"FaceID", sidx);
	//Rjson::rAdd(json,"areaCode",m_areaCode);
	Rjson::rAdd(json,"VideoType",6);
	Rjson::rAdd(json,"ObjectType", 4);//人脸？
	Rjson::rAdd(json,"TaskId", taskId);
	//Rjson::rAdd(json,"DeviceId", deviceId);
	//Rjson::rAdd(json,"Longitude", longitude);
	//Rjson::rAdd(json,"Latitude", latitude);
	//Rjson::rAdd(json,"StartTime", StartTime);
	//Rjson::rAdd(json,"EndTime", EndTime);
	Rjson::rAdd(json,"SnapTime", snapTime);
	//Rjson::rAdd(json,"RelateTime", real_pts);//多
	//Rjson::rAdd(json,"BigImgUrl", face_bkg_url);
	Rjson::rAdd(json,"Width", obj.rect[2]);//TODO
	Rjson::rAdd(json,"Height", obj.rect[3]);
	//Rjson::rAdd(json,"SmallImgUrl", face_obj_url);

	string sx, sy, sbx, sby;
	format_string(sx, "%d", obj.rect[0]);
	format_string(sy, "%d", obj.rect[1]);
	format_string(sbx, "%d", obj.rect[0] + obj.rect[2]);
	format_string(sby, "%d", obj.rect[1] + obj.rect[3]);
	//TODO
	Rjson::rAdd(json,"LeftTopX", sx);
	Rjson::rAdd(json,"LeftTopY", sy);
	Rjson::rAdd(json,"RightBtmX", sbx);
	Rjson::rAdd(json,"RightBtmY", sby);
	//Rjson::rAdd(json,"TargetId", "");
	//Rjson::rAdd(json,"Angle", 99);
	//Rjson::rAdd(json,"Direction", 99);
	//Rjson::rAdd(json,"Speed", 2);
	Rjson::rAdd(json,"TargetSize", 3);//目标大小
	//Rjson::rAdd(json,"IsRetransmission", 99);
	//Rjson::rAdd(json,"CoatColor", 99);
	//Rjson::rAdd(json,"TrousersColor", 99);
	Rjson::rAdd(json,"GenderCode", face_gender_translate(obj.faceRecog.gender.code));
	Rjson::rAdd(json,"AgePerson", obj.faceRecog.age.code);
	Rjson::rAdd(json,"Beard", face_cap_translate(obj.faceRecog.mustache.code));
	if (obj.faceRecog.nation.code == 4)
	{
		Rjson::rAdd(json,"EthicCode", 99);
	}
	else
	{
		Rjson::rAdd(json,"EthicCode", obj.faceRecog.nation.code);
	}
	Rjson::rAdd(json,"SkinColor", obj.faceRecog.race.code);
	Rjson::rAdd(json,"Expression", face_ems_translate(obj.faceRecog.emoticon.code));
	Rjson::rAdd(json,"Cover", obj.faceRecog.cover.code);
	int coverCode = obj.faceRecog.cover.code;
	if (coverCode == 2 || coverCode == 11 || coverCode == 13 || coverCode == 15)
	{
		Rjson::rAdd(json,"Glasses", 1);
	}
	else if (coverCode == 3 || coverCode == 12 || coverCode == 14 || coverCode == 16)
	{
		Rjson::rAdd(json,"Glasses", 2);
	}
	else if (coverCode == 1 || coverCode == 4 || coverCode == 5)
	{
		Rjson::rAdd(json,"Glasses", 3);
	}
	else
	{
		Rjson::rAdd(json,"Glasses", 99);
	}
	//Rjson::rAdd(json,"FaceVID", scdx);
	//Rjson::rAdd(json,"IDType",99);
	//Rjson::rAdd(json,"IDNumber",99);
	//Rjson::rAdd(json,"Name",99);
	//Rjson::rAdd(json,"RelatePersonId", bigRte.uuid);
	//Rjson::rAdd(json,"AreaCode",99);
	//Rjson::rAdd(json,"UpperStyle",99);
	//Rjson::rAdd(json,"LowerStyle",99);
	//Rjson::rAdd(json,"Bag",99);
	//Rjson::rAdd(json,"Handbag",99);
	//Rjson::rAdd(json,"Glasses", has_glasses);
	//Rjson::rAdd(json,"Umbrella",99);
	//Rjson::rAdd(json,"areaCode", m_areaCode);
	if (coverCode == 4 || coverCode == 11 || coverCode == 12 || coverCode == 15 || coverCode == 16)// zj  20191001 判断遮挡返回值戴口罩
	{
		Rjson::rAdd(json,"Respirator", 1);
	}
	else if (coverCode == 99 || coverCode == 0 || coverCode == -1)// zj 20191001 不戴口罩
	{
		Rjson::rAdd(json,"Respirator", 99);
	}
	else
	{
		Rjson::rAdd(json,"Respirator", 0); //zj 20191001 其他情况
	}
	//????
	//Rjson::rAdd(json,"HoldChild", 99);//抱小孩
	Rjson::rAdd(json,"Hat", face_cap_translate(obj.faceRecog.cap.code));
	//Rjson::rAdd(json,"CapColor", 99);
	//Rjson::rAdd(json,"HairStyle", 99);
	string response = Rjson::ToString(json);//json.json();
#else
        dc.AddMember("InterfaceType",StringRef("HI-F"),dc.GetAllocator());
        dc.AddMember("InterfaceVersion",StringRef("1.0"),dc.GetAllocator());
        //dc.AddMember("VideoType",6,dc.GetAllocator());
        dc.AddMember("ObjectType",4,dc.GetAllocator());
        dc.AddMember("FaceVID",scdx,dc.GetAllocator());
        dc.AddMember("FaceID",sidx,dc.GetAllocator());
        dc.AddMember("SnapTime",snapTime,dc.GetAllocator());
        dc.AddMember("Width",obj.rect[2],dc.GetAllocator());
        dc.AddMember("Height",obj.rect[3],dc.GetAllocator());
	string sx, sy, sbx, sby;
	format_string(sx, "%d", obj.rect[0]);
	format_string(sy, "%d", obj.rect[1]);
	format_string(sbx, "%d", obj.rect[0] + obj.rect[2]);
	format_string(sby, "%d", obj.rect[1] + obj.rect[3]);
	//TODO
        dc.AddMember("LeftTopX",sx,dc.GetAllocator());
        dc.AddMember("LeftTopY",sy,dc.GetAllocator());
        dc.AddMember("RightBtmX",sbx,dc.GetAllocator());
        dc.AddMember("RightBtmY",sby,dc.GetAllocator());
        dc.AddMember("TargetSize",3,dc.GetAllocator());
        dc.AddMember("GenderCode",face_gender_translate(obj.faceRecog.gender.code),dc.GetAllocator());
        dc.AddMember("AgePerson",obj.faceRecog.age.code,dc.GetAllocator());
        dc.AddMember("Beard",face_cap_translate(obj.faceRecog.mustache.code),dc.GetAllocator());
	if (obj.faceRecog.nation.code == 4)
	{
		dc.AddMember("EthicCode",99,dc.GetAllocator());
	}
	else
	{
		dc.AddMember("EthicCode",obj.faceRecog.nation.code,dc.GetAllocator());
	}
	dc.AddMember("SkinColor",obj.faceRecog.race.code,dc.GetAllocator());
	dc.AddMember("Expression",face_ems_translate(obj.faceRecog.emoticon.code),dc.GetAllocator());
	dc.AddMember("Cover",obj.faceRecog.cover.code,dc.GetAllocator());
	int coverCode = obj.faceRecog.cover.code;
	if (coverCode == 2 || coverCode == 11 || coverCode == 13 || coverCode == 15)
	{
		dc.AddMember("Glasses",1,dc.GetAllocator());
	}
	else if (coverCode == 3 || coverCode == 12 || coverCode == 14 || coverCode == 16)
	{
		dc.AddMember("Glasses",2,dc.GetAllocator());
	}
	else if (coverCode == 1 || coverCode == 4 || coverCode == 5)
	{
		dc.AddMember("Glasses",3,dc.GetAllocator());
	}
	else
	{
		dc.AddMember("Glasses",99,dc.GetAllocator());
	}
	if (coverCode == 4 || coverCode == 11 || coverCode == 12 || coverCode == 15 || coverCode == 16)// zj  20191001 判断遮挡返回值戴口罩
	{
		dc.AddMember("Respirator",1,dc.GetAllocator());
	}
	else if (coverCode == 99 || coverCode == 0 || coverCode == -1)// zj 20191001 不戴口罩
	{
		dc.AddMember("Respirator",99,dc.GetAllocator());
	}
	else
	{
		dc.AddMember("Respirator",0,dc.GetAllocator());
	}
	dc.AddMember("Hat",face_cap_translate(obj.faceRecog.cap.code),dc.GetAllocator());
	string response = Rjson::ToString(dc);//json.json();
        {
		Document json = Rjson::rWriteDC();
		Rjson::rAdd(json,"InterfaceType", StringRef("HI-FV"));
		Rjson::rAdd(json,"InterfaceVersion", StringRef("1.0"));
		Rjson::rAdd(json,"FaceVID",scdx);
                //Value data = Rjson::rArray();
                Value data(kArrayType);
                //data.PushBack(sidx,json.GetAllocator());
                Value idx(sidx.c_str(),json.GetAllocator());
                data.PushBack(idx,json.GetAllocator());
		Rjson::rAdd(json,"FaceIDs",data);
		Rjson::rAdd(json,"CreateTime",litos_vs(snapTime));
                clusterjson = Rjson::ToString(json);                 
        } 
#endif
	return response;
}
