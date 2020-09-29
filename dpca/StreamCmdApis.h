#ifndef _STREAM_CMD_APIS_H
#define _STREAM_CMD_APIS_H

//#ifdef __GNUC__
//#define BYS_API  extern
//#else
//#ifdef BYS_EXPORTS
//#define BYS_API __declspec(dllexport)
//#else
//#define BYS_API __declspec(dllimport)
//#endif
//#endif

#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

using namespace std;

struct IdxPac
{
	int64_t idx;
	int64_t cluster_idx;
	int32_t gstat;
	IdxPac()
	{
		idx = cluster_idx = gstat = -1;
	}
};

class ImgRecord{
public:
	int64_t idx;
	float sim;
	vector<string> v_params;
	ImgRecord() :idx(-1), sim(-1) {}

	void swap(ImgRecord& r) {
		std::swap(idx, r.idx);
		std::swap(sim, r.sim);
		v_params.swap(r.v_params);
	}
};

class SimImgRecord{
public:
	int64_t cluster_idx;
	int64_t idx;
	float sim;
	vector<string> v_params;
	SimImgRecord() : cluster_idx(-1), idx(-1), sim(-1) {}

	void swap(SimImgRecord& r) 
	{
		std::swap(cluster_idx, r.cluster_idx);
		std::swap(idx, r.idx);
		std::swap(sim, r.sim);
		v_params.swap(r.v_params);
	}
};

class ImgRecordAndUrl {
public:
	int64_t idx;
	float sim;
	string targeturl;   //多图以图搜图，返回目标图片url
	vector<string> v_params;
	ImgRecordAndUrl() :idx(-1), sim(-1) {}
};
struct imgUrlAndData
{
	string imgUrl;
	string imgdata;
};

class sockaddr_ex;

//图搜服务API
class IseApi{
public:
	uint32_t conn_tmo;
	IseApi();
	~IseApi();
	
	/**
	* 配置连接地址
	* @param ip 目标特征搜索引擎地址
	* @param port 目标特征搜索引擎端口号
	*/
	int SetNetAddr(const char* ip, int port);

	/**
	* 创建数据库
	* @param dbname 数据库名
	* @param fieldlist 参数字段
	* @param dictionary 索引列，默认要将所有列制定为索引列，除非有确定不用做检索条件的列可不设为索引列
	*/
	void create_db(const string& dbname, const string& fieldlist, const string& dictionary);

	/**
	* 删除数据库
	* @param dbname 数据库名
	*/
	void delete_db(const string& dbname);

	/**
	* 创建指定库的聚类索引
	* @param dbname 数据库名
	* @param fieldlist 参数字段
	* @param dictionary 索引列，默认要将所有列制定为索引列，除非有确定不用做检索条件的列可不设为索引列
	* @param clusterType 聚类方式，0代表离线聚类，1代表实时聚类
	*/
	int add_db_cluster(const string& dbname, const string& fieldlist, const string& dictionary, int clusterType);

	/**
	* 删除指定库的聚类索引
	* @param dbname 数据库名
	*/
	void delete_db_cluster(const string& dbname);

	/**
	* 列举全部数据库
	* @return 返回的数据库名列表
	*/
	vector<string> enum_all_db(int type);

	/**
	* 获取指定库的状态位信息
	* @param dbname 数据库名
	* @return 相似度值返回的库状态位信息。如果失败则返回-1。成功返回一个>=0的flag二进制位组合值。目前flag位支持项目有：0x1：是否有聚类索引
	*/
	int64_t get_db_status_flag(const string& dbname);

	/**
	* 获取数据库记录数
	* @param dbname 数据库名
	* @return 返回的库记录数
	*/
	int64_t get_db_rec_ct(const string& dbname);

	/**
	* 获取库聚类特征记录数
	* @param dbname 数据库名
	* @return 返回的聚类特征记录数
	*/
	int64_t get_db_cluster_rec_ct(const string& dbname);

	/**
	* 插入图像
	* @param dbname 数据库名
	* @param ftype 特征类型
	* @param feat 图像特征
	* @param paralist 参数列表
	* @param paradata 参数值列表
	* @return 图像索引值
	*/
	IdxPac push_image(const string& dbname, int ftype, const string& feat,
		const string& paralist, const string& paradata, bool iscluster = false);

	/**
	* 插入图像
	* @param dbname 数据库名
	* @param ftype 特征类型
	* @param feat 图像特征
	* @param idx 图像索引值
	* @param paralist 参数列表
	* @param paradata 参数值列表
	* @return 图像索引值
	*/
	IdxPac push_image_idx(const string& dbname, int ftype, const string& feat, int64_t idx,
		const string& paralist, const string& paradata, bool iscluster = false);

	/**
	* 按聚类插入图像
	* @param dbname 数据库名
	* @param cluster_idx 图像索引值
	* @param ftype 特征类型
	* @param feat 图像特征
	* @param paralist 参数列表
	* @param paradata 参数值列表
	* @return 图像索引值
	*/
	int64_t push_image_cluster(const string& dbname, int64_t cluster_idx, int ftype, const string& feat,
		const string& paralist, const string& paradata);

	/**
	* 添加聚类记录
	* @param dbname 数据库名
	* @param ftype 特征类型
	* @param feat 图像特征
	* @param paralist 参数列表
	* @param paradata 参数值列表
	* @return 图像索引值
	*/
	IdxPac add_cluster_rec(const string& dbname, int ftype, const string& feat, const string& paralist, const string& paradata, int newRec = 0);

	/**
	* 图像检索
	* @param dbname 数据库名
	* @param ftype 特征类型
	* @param feat 图像特征
	* @param wanted_param 希望返回的参数的列表
	* @param where_stmt where语句
	* @param min_sim 检索最小相似度限定
	* @param pRecs 检索到的图像数据
	* @param max_rec  最大返回结果数
	* @return 查询结果数
	*/
	int retrieve_image(const string& dbname, int ftype, const string& feat,
		const string& wanted_param, const string& where_stmt, float min_sim, ImgRecord* pRecs, uint32_t max_rec);

	/**
	* select方式查询记录数
	* @param dbname 数据库名
	* @param where_stmt where语句
	* @return 库记录数
	*/
	int64_t select_rec_ct(const string& dbname, const string& where_stmt);

	/**
	* select方式查询图像
	* @param dbname 数据库名
	* @param wanted_param 希望返回的参数的列表
	* @param where_stmt where语句	
	* @param pRecs 检索到的图像数据
	* @param max_rec 最大返回结果数
	* @return 查询结果数
	*/
	int select_img_rec(const string& dbname, const string& wanted_param, const string& where_stmt,
		ImgRecord* pRecs, uint32_t max_rec);

	/**
	* 根据图像索引获取图像信息
	* @param dbname 数据库名
	* @param idxes 图像索引，如传多个，需用逗号分隔
	* @param wanted_param 希望返回的参数的列表，此处对应的是入库图像的信息
	* @param pRecs 查询到的图像数据
	* @return 查询结果数
	*/
	int get_rec_by_idx(const string& dbname, string idxes, const string& wanted_param, ImgRecord* pRecs);

	/**
	* 根据聚类索引获取聚类信息
	* @param dbname 数据库名
	* @param cluster_idxes 聚类索引，如传多个，需用逗号分隔
	* @param wanted_param 希望返回的参数的列表，此处对应的是聚类的信息
	* @param pRecs 查询到的聚类数据
	* @return 查询结果数
	*/
	int get_cluster_rec_by_cluster_idx(const string& dbname, string cluster_idxes, const string& wanted_param, ImgRecord* pRecs);

	/**
	* 聚类特征检索
	* @param dbname 数据库名
	* @param ftype 特征类型
	* @param feat 图像特征
	* @param wanted_param 希望返回的参数的列表
	* @param where_stmt where语句
	* @param min_sim 检索最小相似度限定
	* @param pRecs 检索到的图像数据
	* @param max_rec  最大返回结果数
	* @return 查询结果数
	*/
	int retrieve_image_cluster(const string& dbname, int ftype, const string& feat,
		const string& wanted_param, const string& where_stmt, float min_sim, ImgRecord* pRecs, uint32_t max_rec);

	/**
	* select方式查询聚类特征记录数
	* @param dbname 数据库名
	* @param where_stmt where语句
	* @return 聚类特征记录数
	*/
	int64_t select_cluster_ct(const string& dbname, const string& where_stmt);

	/**
	* select方式查询聚类特征数据
	* @param dbname 数据库名
	* @param wanted_param 希望返回的参数的列表
	* @param where_stmt where语句
	* @param pRecs 检索到的图像数据
	* @param max_rec 最大返回结果数
	* @return 查询结果数
	*/
	int select_image_cluster(const string& dbname, const string& wanted_param, const string& where_stmt,
		ImgRecord* pRecs, uint32_t max_rec);

	/**
	* 获取聚类下属图像信息
	* @param dbname 数据库名
	* @param cluster_idx 聚类特征索引
	* @param wanted_param 希望返回的参数的列表，此处对应的是入库图像的信息
	* @param pRecs 查询到的图像数据
	* @return 查询结果数
	*/
	int get_image_rec_by_cluster_index(const string& dbname, int64_t cluster_idx, const string& wanted_param, vector<ImgRecord>& pRecs);

	/**
	* 获取聚类特征下最相似的记录，可以传多个聚类特征记录
	* @param dbname 数据库名
	* @param cluster_idxes 聚类特征索引，如传多个需按逗号分隔
	* @param wanted_param 希望返回的参数的列表，此处对应的是入库图像的信息
	* @param pRecs 获取的图像数据
	*/
	void get_most_sim_rec_by_cluster_idxes(const string& dbname, const string& cluster_idxes, const string& wanted_param, vector<SimImgRecord>& pRecs);

	/**
	* 获取聚类下属图像信息
	* @param dbname 数据库名
	* @param cluster_idx 聚类特征索引
	* @return 获取到的图像记录数
	*/
	int32_t get_rec_ct_by_cluster_idx(const string& dbname, int64_t cluster_idx);

	/**
	* 根据条件删除记录
	* @param dbname 数据库名
	* @param index 索引值
	*/
	void delete_img_rec(const string& dbname, int64_t index);

	/**
	* 根据条件删除记录
	* @param dbname 数据库名
	* @param where_stmt where语句
	*/
	void delete_img_rec_ws(const string& dbname, const string& where_stmt);

	/**
	* 根据条件更新记录
	* @param dbname 数据库名
	* @param index 索引值
	* @param set_stmt set语句，语句格式参照sql
	*/
	void update_img_rec(const string& dbname, int64_t index, const string& set_stmt);

	/**
	* 根据条件更新记录
	* @param dbname 数据库名
	* @param where_stmt where语句
	* @param set_stmt set语句，语句格式参照sql
	*/
	void update_img_rec_ws(const string& dbname, const string& where_stmt, const string& set_stmt);

	/**
	* 根据索引删除聚类特征记录
	* @param dbname 数据库名
	* @param cluster_idx 聚类特征索引
	* @param del_rec 传0表示不删除下属图像，传1表示删除聚类时一并删除下属图像
	*/
	void delete_cluster_rec(const string& dbname, int64_t cluster_idx, int del_rec);

	/**
	* 根据条件删除聚类特征记录
	* @param dbname 数据库名
	* @param where_stmt where语句
	* @param del_rec 传0表示不删除下属图像，传1表示删除聚类时一并删除下属图像
	*/
	void delete_cluster_rec_ws(const string& dbname, const string& where_stmt, int del_rec);

	/**
	* 根据索引更新聚类关联信息
	* @param dbname 数据库名
	* @param cluster_idx 聚类特征索引值
	* @param set_stmt set语句，语句格式参照sql
	*/
	void update_cluster_data(const string& dbname, int64_t cluster_idx, const string& set_stmt);

	/**
	* 根据条件更新聚类关联信息
	* @param dbname 数据库名
	* @param cluster_idx 聚类特征索引值
	* @param set_stmt set语句，语句格式参照sql
	*/
	void update_cluster_data_ws(const string& dbname, const string& where_stmt, const string& set_stmt);

	/**
	* 更新图像的聚类关系，没有聚类关系将创建聚类关系
	* @param dbname 数据库名
	* @param idx 图像特征索引值
	* @param cluster_idx 聚类特征索引值
	*/
	void update_rec_cluster(const string& dbname, int64_t idx, int64_t cluster_idx);

	/**
	* 移除聚类特征下的记录
	* @param dbname 数据库名
	* @param cluster_idx1 聚类特征索引1
	* @param idxes 图像记录索引，如传多个需按逗号分隔
	*/
	void remove_rec_of_cluster(const string& dbname, int64_t cluster_idx, const string& idxes);

	/**
	* 手动将聚类记录2合并至记录1。合并后聚类记录2信息消失，所有下属图像归属到记录1上
	* @param dbname 数据库名
	* @param cluster_idx1 记录1的聚类索引（要合并的）
	* @param cluster_idx2 记录2的聚类索引（要被合并消失的）
	*/
	void join_cluster_rec(const string& dbname, int64_t cluster_idx1, int64_t cluster_idx2);

	/**
	* 获取聚类下属图像信息
	* @param dbname 数据库名
	* @param idx 图像索引值
	* @param cluster_idx 聚类特征索引
	* @param wanted_param 希望返回的参数的列表，此处对应的是入库图像的信息
	* @param pRecs 查询到的图像数据
	* @param max_rec 最大返回结果数，传0表示返回全部
	* @return 查询结果数
	*/
	int cluster_comp_by_image_rec(const string& dbname, int64_t idx, int64_t cluster_idx, const string& wanted_param, ImgRecord* pRecs, uint32_t max_rec);

	/**
	* 获取聚类下属图像信息
	* @param dbname 数据库名
	* @param cluster_idx1 聚类特征索引1
	* @param cluster_idx2 聚类特征索引2
	* @param wanted_param 希望返回的参数的列表，此处对应的是入库图像的信息
	* @param pRecs 查询到的图像数据
	* @param max_rec 最大返回结果数，传0表示返回全部
	* @return 查询结果数
	*/
	void two_cluster_comp(const string& dbname, int64_t idx, int64_t cluster_idx, const string& wanted_param, 
							unordered_map<shared_ptr<ImgRecord>, vector<shared_ptr<ImgRecord>>>& pRecs, uint32_t max_rec);

	/**
	* 根据索引比对两个图像特征的相似度
	* @param dbname 数据库名
	* @param idx1 图像特征索引1
	* @param idx2 图像特征索引2
	* @return 相似度值
	*/
	float calc_image_feat_sim(const string& dbname, int64_t idx1, int64_t idx2);
	
	/**
	* 根据索引比对两个聚类特征的相似度
	* @param dbname 数据库名
	* @param cluster_idx1 聚类特征索引1
	* @param cluster_idx2 聚类特征索引2
	* @return 相似度值
	*/
	float calc_center_feat_sim(const string& dbname, int64_t cluster_idx1, int64_t cluster_idx2);

	/**
	* 图像M:N检索
	* @param session_id 图像M:N检索唯一会话id
	* @param src_dbname 源数据库名
	* @param dst_dbname 待检索数据库
	* @param wantParaSrc 希望返回的源数据库参数的列表
	* @param wantParaDst 希望返回的待检索数据库参数的列表
	* @param where_stmt where语句
	* @param min_sim 最小相似度
	* @param max_rec 最大返回结果数
	*/
	void multi_retrieve(const string& session_id, const string& src_dbname, const string& dst_dbname, const string& wantParaSrc, const string& wantParaDst, const string& whereStm, float min_sim, uint32_t max_rec);

	/**
	* 聚类记录M:N检索
	* @param session_id 图像M:N检索唯一会话id
	* @param src_dbname 源数据库名
	* @param dst_dbname 待检索数据库
	* @param wantParaSrc 希望返回的源数据库参数的列表
	* @param wantParaDst 希望返回的待检索数据库参数的列表
	* @param where_stmt where语句
	* @param min_sim 最小相似度
	*/
	void multi_cluster_retrieve(const string& session_id, const string& src_dbname, const string& dst_dbname, const string& wantParaSrc, const string& wantParaDst, const string& whereStm, float min_sim);

	/**
	* 查询库文件数量
	* @param session_id 图像M:N检索唯一会话id
	*/
	double get_multi_retrieve_progress(const string& session_id);

	/**
	* 查询库文件数量
	* @param session_id 图像M:N检索唯一会话id
	*/
	void multi_retrieve_stop(const string& session_id);

	/**
	* 获取图像M:N检索结果
	* @param session_id 图像M:N检索唯一会话id
	* @param pRecs 返回结果合集
	*/
	void get_multi_retrieve_result(const string& session_id, unordered_map<shared_ptr<ImgRecord>, vector<shared_ptr<ImgRecord>>>& pRecs);

	/**
	* 获取图像特征
	* @param dbname 数据库名
	* @param idx 图像索引值
	* @param feature 返回的图像特征
	*/
	void get_feature(const string& dbname, int64_t idx, string& feature);

	/**
	* 获取聚类特征
	* @param dbname 数据库名
	* @param cluster_idx 聚类特征索引值
	* @param feature 返回的聚类特征
	*/
	void get_cluster_feature(const string& dbname, int64_t cluster_idx, string& feature);

private:
	sockaddr_ex* addr_;
};

//特征提取服务API
class ExtrApi{
public:
	uint32_t conn_tmo;
	ExtrApi();
	~ExtrApi();

	/**
	* 配置连接地址
	* @param ip 提特征服务地址
	* @param port 提特征服务端口号
	*/
	int SetNetAddr(const char* ip, int port);

	/**
	* 提特征
	* @param ftype 图片类型，一般填1
	* @param imgdata 图像二进制数据
	* @return 图像特征数据
	*/
	string extract_feature(int ftype,const string& imgdata);

	/**
	* 灰度图像提特征
	* @param ftype 图片类型，一般填1
	* @param w 图像宽度
	* @param h 图像宽度
	* @param pitch 图像像素数据行间距
	* @param pixdata 图像灰度数据
	* @return 图像特征数据
	*/
	string extract_feature(int ftype, uint32_t w, uint32_t h, uint32_t pitch,
		const char* pixdata);
private:
	sockaddr_ex* addr_;
};



#endif
