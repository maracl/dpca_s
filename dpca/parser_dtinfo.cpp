#include "Util.h"
#include "parser_dtinfo.h"
#include "rapidjson/Rjson.hpp"
#include "General_exception2.h"
#include "spdlog_helper.h"


bool  check_do_relate(const string& rsp_info,const int& width,const int& height)
{
	bool do_relate = false;
	try
	{
		auto parse_obj = [](const Value* obj, int rect[4], string& guid) 
		{
			const Value* body = Rjson::GetObject("Detect", "Body", obj);
			for (int i = 0; i < 4; ++i)
			{
				rect[i] = Rjson::GetInt("Rect", i, body);
			}
			Rjson::GetStringV(guid, "GUID", obj);
			int type = Rjson::GetInt("Type", obj);
			return type;
		};

		Document dc;
		Rjson::Parse(dc, rsp_info);
		const Value* image_results = nullptr;
		if (!Rjson::GetObjectV(&image_results, "image_results", 0, &dc))
		{
			return false;//如果算法包没有这个字段，返回空队列
		}
		string rsp_buf = Rjson::GetString("rsp_buf", image_results);
		//////////////////////////////////////////////////////////////////////////
		Document dc_obj;
		Rjson::Parse(dc_obj, rsp_buf);
		
		const Value* obj = nullptr;
		if (Rjson::GetObjectV(&obj, "ImageResults", 0, &dc_obj))
		{
			const Value* p_arry = Rjson::GetArray("Pedestrains", obj);
			for (int i = 0; i < p_arry->Size(); ++i)
			{
				const Value* obj = Rjson::GetObject(i, p_arry);
				int rect[4];
				string id;
				parse_obj(obj, rect, id);
				if((rect[2] >= 55) || (rect[1] + rect[3]/2 >= height/2))
				{
                   do_relate = true; 
				}
			}
			//////////////////////////////////////////////////////////////////////////
			//bike
			const Value* b_arry = Rjson::GetArray("Bikes", obj);
			for (int i = 0; i < b_arry->Size(); ++i)
			{
				const Value* obj = Rjson::GetObject(i, b_arry);
				int rect[4];
				string id;
				parse_obj(obj, rect, id);
				if((rect[2] >= 55) || (rect[1] + rect[3]/2 >= height/2))
				{
                   do_relate = true; 
				}
			}
			//机动车
			//const Value* vehicle_arr = Rjson::GetArray("Vehicles", obj);
			//for (int i = 0; i < vehicle_arr->Size(); ++i)
			//{
			//	const Value* obj = Rjson::GetObject(i, vehicle_arr);
			//	int rect[4];
			//	string id;
			//	parse_obj(obj, rect, id);
			//}
		}
	}
	catch (GeneralException2& e)
	{
		printf("parse_dt_info parse error,code: %d ,msg: %s",e.err_code(),e.err_str());
		return false;
	}
	catch (...)
	{
		printf("parse_dt_info parse error");
		return false;
	}
	return do_relate;
}
