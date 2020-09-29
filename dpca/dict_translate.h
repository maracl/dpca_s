#ifndef _DICT_TRANSLATE_H_
#define _DICT_TRANSLATE_H_

#include <string>

using namespace std;

typedef struct CarBrand {
	CarBrand(){ };
	CarBrand(const char* main, const char* sub, const char* year) : Main(main), Sub(sub), Year(year){}
	string Main;
	string Sub;
	string Year;
} CarBrand;

string color_translate(const string& code);

string plate_attribution_translate(const string& code, int &has_plate);

int color_plate_translate(const string& code);

int person_color_translate(const string& code);

int angle_translate(const string& code);

int car_angle_translate(const string& code);

string car_type_translate(const string code);

int car_brand_translate(const string& guid, CarBrand& brand);

int person_sex_translate(const string &code);//�Ա�
string plate_type_translate(const int code);//��������
int person_upstyle_translate(const string &code);//����
int person_downstyle_translate(const string &code);//����
int person_haire_translate(const string &code);//����
int direction_translate(int type, const string &code);//����ת��

int bike_type_translate(int type, int &bike_type, int &bike_class);
int bike_color_translate(const string& code);//�ǻ�������ɫ

int face_cap_translate(int code);
int face_ems_translate(int code);
int face_cover_translate(int code);
int face_gender_translate(int code);
int face_Respirator_translate(int code);
int face_Eyebrow_translate(int code);
int face_glass_translate(int code);

int boyun_car_class_tran(const int code);
int boyun_confidence_translc(const int code);
string boyun_plate_type_translate(const int code);

#endif // _DICT_TRANSLATE_H_
