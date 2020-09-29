#include "ise_proxy.h"
#include "Util.h"
#include "StreamCmdApis.h"
#include "create_uuid.h"
#include "byte_buffer.h"
#include "rapidjson/Rjson.hpp"
#include "spdlog_helper.h"
#include "Base64.h"

//create fields
static const string person_db_fields = "sess_id String,rect String, obj_url String, bkg_url String, pts int64,timestamp int64,face_idx int64,face_cluster_id int64,"
"age int, sex int, bag int, bottom_color int, bottom_type int, hat int,knapsack int, orientation int,"
"hair int, umbrella int, upper_color int upper_type int, upper_texture int, body int, glasses int, mask int";
//push fields format
static const string person_fields = "sess_id,rect, obj_url, bkg_url, pts,timestamp,face_idx,face_cluster_id"
"age, sex, bag, bottom_color, bottom_type, hat,knapsack, orientation,"
"hair, umbrella, upper_color, upper_type, upper_texture, body, glasses, mask";
//static const string person_fields_format = "'%s','%s','%s','%s',%lld,%lld,%lld,%lld,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
static const string person_fields_format = "'{}','{}','{}','{}',{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{}";
//create fields
//static const string face_db_fields = "sess_id String,rect String, obj_url String, bkg_url String, pts int64,timestamp int64";
//push fields format
//static const string face_fields = "sess_id, rect, obj_url, bkg_url, pts, timestamp";
//static const string face_fields_format = "'{}','{}','{}','{}',{},{}";

//Face
//create fields
//static const string face_db_fields = "sess_id String,rect String, obj_url String, bkg_url String, pts int64,timestamp int64";
static const string face_db_fields = "Reserved String,FaceVID String,FaceID String,Url String,Pts BIGINT,SnapTime BIGINT,InterfaceType String,InterfaceVersion String,ManufacturerId Int,UUID String,VideoType Int,TaskId String,DeviceId String,DeviceName String,RelatePersonId String,Longitude String,Latitude String,StartTime BIGINT,EndTime BIGINT,BigImgUrl String,Width Int,Height Int,SmallImgUrl String,ObjPosition String,Angle Int,Direction Int,AgePerson Int,Beard Int,SkinColor Int,Glasses Int,EthicCode Int,GenderCode Int,Expression Int,Cover Int,Hat Int,Respirator Int,Eyebrow Int";//static const string face_fields_format = "'{}',    {},        {},              '{}',                  '{}',                      {},            '{}',        {},          '{}',       '{}',           '{}',           '{}',              {},             {},           '{}',            {},          {},     '{}',                  '{}',          {},           {},          {},          {},         {},       {},         {},           {},              {},           {},        {}";//push fields format
																																																																																																																						  //static const string face_fields = "sess_id, rect, obj_url, bkg_url, pts, timestamp";
static const string face_fields = "Reserved,FaceVID,FaceID,Url,Pts,SnapTime,InterfaceType,InterfaceVersion,ManufacturerId,UUID,VideoType,TaskId,DeviceId,DeviceName,RelatePersonId,Longitude,Latitude,StartTime,EndTime,BigImgUrl,Width,Height,SmallImgUrl,ObjPosition,Angle,Direction,AgePerson,Beard,SkinColor,Glasses,EthicCode,GenderCode,Expression,Cover,Hat,Respirator,Eyebrow";//static const string face_fields_format = "'{}','{}','{}','{}',{},{}";

static const string face_fields_format = "'{}',{},{},'{}','{}',{},'{}',{},'{}','{}','{}','{}',{},{},'{}',{},{},'{}','{}',{},{},{},{},{},{},{},{},{},{},{}";

// static const char* vehicle_wp = "Reserved,VehiVID,VehiID,Url,Pts,SnapTime,InterfaceType,InterfaceVersion,ManufacturerId,UUID,VideoType,TaskId,"
// "DeviceId,DeviceName,Longitude,Latitude,StartTime,EndTime,BigImgUrl,Width,Height,SmallImgUrl,ObjPosition,Angle,Direction,"
// "VehicleClass,VehicleColor,VehicleBrand,VehicleSubBrand,VehicleStyles,HasPlate,PlateNo,PlateAttribution,PlateColor,PlateType,Confidence,"
// "PlatePosition,Sunvisor,SubSunVisor,SafetyBelt,SubSafetyBelt,Calling,Rack,Skylight,Spare,Spray,Danger,Ornament,Paper,Tag,TagNum,Crash,Drop,Card,HaveSub,SubCalling";
// 
// static const char* bike_wp = "Reserved,BikeVID,BikeID,Url,Pts,SnapTime,InterfaceType,InterfaceVersion,ManufacturerId,UUID,VideoType,TaskId,"
// "DeviceId,DeviceName,RelateFaceID,Longitude,Latitude,StartTime,EndTime,BigImgUrl,Width,Height,SmallImgUrl,ObjPosition,Angle,Direction,"
// "BikeType,HasPlate,IsSeating,SeatingCount,Hood,CoatColor,TrousersColor,GenderCode,AgePerson,UpperStyle,LowerStyle,"
// "Bag,Handbag,Glasses,Umbrella,UmbrellaColor,Respirator,HoldChild,Hat,CapColor,HaireStyle,BikeClass";
// 
// static const char* pedestrain_wp = "Reserved,PedesVID,PedesID,Url,Pts,SnapTime,InterfaceType,InterfaceVersion,ManufacturerId,UUID,VideoType,TaskId,"
// "DeviceId,DeviceName,RelateFaceID,Longitude,Latitude,StartTime,EndTime,BigImgUrl,Width,Height,SmallImgUrl,ObjPosition,Angle,Direction,"
// "CoatColor,TrousersColor,GenderCode,AgePerson,UpperStyle,LowerStyle,"
// "Bag,Handbag,Glasses,Umbrella,Respirator,HoldChild,Hat,CapColor,HaireStyle";


IseApi g_IseApi;
extern IseApi  pIseApi;

string get_db_name(const string& db, int type)
{
	if (db.empty())
		return "";
	if (type == eTypePerson)
		return "p_" + db;
	else if (type == eTypeFace)
		return "f_" + db;
	else
		return "";
}

string get_db_fields_format(int type)//在其他源文件里边调用全局参数
{
	string fields_format;
	switch (type)
	{
	case eTypePerson:
		fields_format = person_fields_format;
	break;
	case eTypeFace:
		fields_format = face_fields_format;
	break;
	default:
		break;
	}
	return fields_format;
}

void ise_init(const string& ip, int port)
{
	g_IseApi.SetNetAddr(ip.data(), port);
}

string ise_create_db()
{
	string db_name;
	try
	{
		//db_name = create_uuid(false);
		//return db_name;//测试
		//person
		//string person_db_name = get_db_name(db_name, eTypePerson);
		//g_IseApi.create_db(db_name, person_db_fields, "");//暂时先注释行人库
		//face
		string face_db_name = get_db_name(db_name, eTypeFace);
		g_IseApi.add_db_cluster(face_db_name, face_db_fields, "",1);
	}
	catch (GeneralException2& e)
	{
		LOG_ERROR(MAIN_LOG,"ise_create_db failed，code:{},msg:{}", e.err_code(), e.err_msg());
		db_name.clear();
	}
	return db_name;
}

void ise_del_db_data(const string& db_name)
{
	try
	{
#if 0
		return;
#endif
		if (db_name.empty())	return;
		//del person data
		string person_db_name = get_db_name(db_name, eTypePerson);
		//select and del img

		//del db
		g_IseApi.delete_db(person_db_name);
		//////////////////////////////////////////////////////////////////////////
		//del face data
		string face_db_name = get_db_name(db_name, eTypeFace);
		//select and del img
		g_IseApi.delete_db(face_db_name);
	}
	catch (GeneralException2& e)
	{
		LOG_ERROR(MAIN_LOG,"ise_del_db_data failed，code:{},msg:{}", e.err_code(), e.err_msg());
	}
}

IdxPac ise_push_data(const string& db_name, int type, const string& feat, const string& value_fields, bool isCluster)
{
	//string feattemp = feat;
	/*IdxPac _idxPac;
	try
	{
		//return _idxPac;//test 关闭
		if (db_name.empty() || feat.empty())	return _idxPac;
		string real_db_name = get_db_name(db_name, type);
		string db_fields = (type == eTypePerson) ? person_fields : face_fields;
		IdxPac _idxPac = g_IseApi.push_image(real_db_name, 1, feat, db_fields, value_fields);
	}
	catch (GeneralException2& e)
	{
		LOG_ERROR(MAIN_LOG,"ise_push_data failed，code:{},msg:{}", e.err_code(), e.err_msg());
	}
	return _idxPac;*/
	IdxPac _idxPac;
	try
	{
		//return _idxPac;//test 关闭
		if (db_name.empty() || feat.empty())
			return _idxPac;
		//测试 双网双平台
// 		string ply;
//  		string fet1_base64 = base64_encode(reinterpret_cast<const unsigned char*>(feat.data()), feat.size());
// 		LOG_ERROR(MAIN_LOG,"warning--printf the feature for base64---{}", fet1_base64);
// 		post_doubleplatc_iseinfo(value_fields, fet1_base64, db_name, ply);
		//(-) liuhao
		//string real_db_name = get_db_name(db_name, type);
		string real_db_name = db_name;
		string db_fields = (type == eTypePerson) ? person_fields : face_fields;
		//TODO
// 		if (feattemp != feat)
// 		{
// 			LOG_INFO(MAIN_LOG,"{}", "here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!not match!!");
// 		}
		IdxPac _idxPac = pIseApi.push_image(real_db_name, type, feat, db_fields, value_fields,isCluster);
		int64_t cdx = _idxPac.cluster_idx;
		LOG_INFO(MAIN_LOG,"here!!!the cdx is {}", cdx);
		if (cdx < 0)
		{
			return _idxPac;
		}
		std::thread([cdx, real_db_name]() {
			try
			{
				string want_param("RealName");
				string cluster_idx;
				format_string(cluster_idx, "%lld", cdx);
				vector<ImgRecord> vRecs(2);
				int rt = pIseApi.get_cluster_rec_by_cluster_idx(real_db_name, cluster_idx, want_param, vRecs.data());
				vRecs.resize(rt);
				for (int sj = 0; sj < vRecs.size(); sj++)
				{
					ImgRecord& sr = vRecs[sj];
					if (sr.v_params[0] == "")
					{
						string set_stmt("RealName = 1");
						pIseApi.update_cluster_data(real_db_name, cdx, set_stmt);
					}
				}
			}
			catch (GeneralException2& e)
			{
				LOG_ERROR(MAIN_LOG,"cluster update failed，code:{},msg:{}", e.err_code(), e.err_msg());
			}
			catch (...)
			{

			}
		}).detach();
		return _idxPac;
	}
	catch (GeneralException2& e)
	{
		LOG_ERROR(MAIN_LOG,"ise_push_db_data failed，code:{},msg:{},type:{}", e.err_code(), e.err_msg(), type);
	}
	return _idxPac;

}

//////////////////////////////////////////////////////////////////////////
ExtrApi g_ExtrApi;
void extr_init(const string& ip, int port)
{
	g_ExtrApi.SetNetAddr(ip.data(), port);
}

string extr_feat_person(uint32_t w, uint32_t h, uint32_t pitch, const char* pixdata)
{
	return g_ExtrApi.extract_feature(1, w, h, pitch, pixdata);
}
