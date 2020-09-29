#ifndef _BASEIC_UTILS_H
#define _BASEIC_UTILS_H
#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "common_interface.h"
#include <fcntl.h>
#include <atomic>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#define _read read
#define _close close
#define _write write
#endif
#include <memory>
#include <vector>
#include <thread>
#include <tbb/concurrent_queue.h>
#include <functional>

class SimpleNotifier :no_copy{
	long _wk;
public:
	SimpleNotifier() :_wk(0) {}

	void wait();
        void wait(int seconds);
	void wait2(int millseconds);
	void wake();
};

void wait_ftxp(void* p);

void wake_ftxp(void* p);

class HiresTimer:no_copy{
#ifdef _WIN32
	__int64 iFreq, iStart, iEnd;
#else
	double dStart, dEnd;
#endif
public:
	HiresTimer();

	void start();
	void end();

	double getTime() const;
};


class FTaskPool : no_copy{

public:
	typedef std::function<void(int)> FuntorType;
	explicit FTaskPool(size_t cap);
	~FTaskPool();

	int init_pool(int nthread);

	void push_task(FuntorType&& tsk);

	bool push_task_nb(FuntorType&& tsk);

	void push_task(const FuntorType& tsk);

	bool push_task_nb(const FuntorType& tsk);

	void clear();

private:
	tbb::concurrent_bounded_queue<FuntorType > m_que;
};

class FTaskPool2 : no_copy{

public:
	typedef std::function<void()> FuntorType;
	explicit FTaskPool2(size_t cap) {
		m_que.set_capacity(cap);
	}
	~FTaskPool2() {
	}

	int init_pool(int nthread);

	void push_task(const FuntorType& tsk);

	bool push_task_nb(const FuntorType& tsk);

	void clear();

private:
	tbb::concurrent_bounded_queue<FuntorType > m_que;
};


class FTaskPool4 : no_copy{

private:
	std::atomic<int> _quitflg;
        std::vector < std::thread > _fun_vec;
public:
	typedef std::function<void()> FuntorType;
	explicit FTaskPool4(size_t cap) {
		m_que.set_capacity(cap);
                _quitflg = 0;
	}
	~FTaskPool4();

	int init_pool(int nthread);

	void push_task(const FuntorType& tsk);

	bool push_task_nb(const FuntorType& tsk);

	void clear();

private:
	tbb::concurrent_bounded_queue<FuntorType > m_que;
};





class FTaskPool3 : no_copy{
private:
	std::atomic<int> _quitflg;
public:
	typedef std::function<void()> FuntorType;
	explicit FTaskPool3() {_quitflg = 0;}
	~FTaskPool3();

	int init_pool(int cap, int nthread);

	void push_task(const FuntorType& tsk, uint32_t nt);

	bool push_task_nb(const FuntorType& tsk, uint32_t nt);

	void clear();

private:
	std::vector< std::shared_ptr<tbb::concurrent_bounded_queue<FuntorType> > > m_vqs;
};

template<class GpuInitType>
class GpuTaskPool_NN :no_copy {
public:
	typedef std::function<void(GpuInitType&, uint32_t thread_id)> TaskType;
	explicit GpuTaskPool_NN() {}
	~GpuTaskPool_NN() {}

	int init_pool(uint32_t cap, const std::vector<uint32_t>& gpu_lst) {
		if (!m_vqs.empty()) return 1;
		size_t ng = gpu_lst.size();
		if (ng == 0) return 10;
		m_vqs.reserve(ng);
		for (size_t i = 0; i < ng; i++) {
			uint32_t gpuid = gpu_lst[i];
			auto task_que = std::make_shared<tbb::concurrent_bounded_queue<TaskType> >();
			task_que->set_capacity(cap);
			m_vqs.push_back(task_que);
			std::thread([task_que, gpuid,i]() {
				GpuInitType _init_obj(gpuid);
				for (TaskType t;;) {
					task_que->pop(t);
					t(_init_obj, (uint32_t)i);
				}
			}).detach();
		}
		return 0;
	}

	void push_task(TaskType&& tsk, uint32_t nt) {
		if (m_vqs.empty()) return;
		m_vqs[nt % (uint32_t)m_vqs.size()]->push(tsk);
	}

	void push_task(const TaskType& tsk, uint32_t nt) {
		if (m_vqs.empty()) return;
		m_vqs[nt % (uint32_t)m_vqs.size()]->push(tsk);
	}

	bool push_task_nb(TaskType&& tsk, uint32_t nt) {
		if (m_vqs.empty()) return false;
		return m_vqs[nt % (uint32_t)m_vqs.size()]->try_push(tsk);
	}

	bool push_task_nb(const TaskType& tsk, uint32_t nt) {
		if (m_vqs.empty()) return false;
		return m_vqs[nt % (uint32_t)m_vqs.size()]->try_push(tsk);
	}
private:
	std::vector< std::shared_ptr<tbb::concurrent_bounded_queue<TaskType> > > m_vqs;
};


template<class GpuInitType>
class GpuTaskPool_1N :no_copy {
public:
	typedef std::function<void(GpuInitType&, uint32_t thread_id)> TaskType;
	explicit GpuTaskPool_1N() {}
	~GpuTaskPool_1N() {}
	
	int init_pool(uint32_t cap, const std::vector<uint32_t>& gpu_lst) {
		if (m_que) return 1;
		size_t ng = gpu_lst.size();
		if (ng == 0) return 10;
		auto q = std::make_shared<tbb::concurrent_bounded_queue<TaskType> >();
		m_que = q;
		m_que->set_capacity(cap);
		for (size_t i = 0; i < ng; i++) {
			uint32_t gpuid = gpu_lst[i];
			std::thread([q, gpuid, i]() {
				GpuInitType _init_obj(gpuid);
				for (TaskType t;;) {
					q->pop(t);
					t(_init_obj, (uint32_t)i);
				}
			}).detach();
		}
		return 0;
	}

	void push_task(TaskType&& tsk) {
		if (!m_que) return;
		m_que->push(tsk);
	}

	void push_task(const TaskType& tsk) {
		if (!m_que) return;
		m_que->push(tsk);
	}

	bool push_task_nb(TaskType&& tsk) {
		if(!m_que) return false;
		return m_que->try_push(tsk);
	}

	bool push_task_nb(const TaskType& tsk) {
		if (!m_que) return false;
		return m_que->try_push(tsk);
	}

private:
	std::shared_ptr<tbb::concurrent_bounded_queue<TaskType> > m_que;
};

/**
class ScalableTaskPool : no_copy{

public:
	typedef std::function<void(int)> FuntorType;
	explicit ScalableTaskPool();
	~ScalableTaskPool();

	int init_pool(uint32_t nt_min, uint32_t nt_max);

	void push_task(FuntorType&& tsk);

	bool push_task_nb(FuntorType&& tsk);

	void clear();

private:
	tbb::concurrent_bounded_queue<FuntorType > m_que;
};//*/

class AvgValueGenerator{
public:
	AvgValueGenerator() :sumv_(0), vct_(0) {}

	double new_value(double val) {
		sumv_ += val;
		++vct_;
		return get_avg();
	}

	double get_avg() const {
		return vct_ == 0 ? 0 : sumv_ / vct_;
	}
	void clear() {
		sumv_ = 0;	vct_ = 0;
	}
private:
	double sumv_;
	int64_t vct_;
};

#endif//_BASEIC_UTILS_H
