#ifndef _GPU_UTIL_EXT_H
#define _GPU_UTIL_EXT_H

#include "basic_utils.h"




template<class GpuInstType>
class GpuTaskPool_11b :no_copy {
public:
	typedef std::function<void(GpuInstType&, uint32_t)> InitType;
	typedef std::function<void(GpuInstType&)> TaskType;
	explicit GpuTaskPool_11b() {}
	~GpuTaskPool_11b() {}

	int init_pool(uint32_t cap, uint32_t gpuid, const InitType& inst_init_cb) {
		if (m_que) return 1;

		auto q = std::make_shared<tbb::concurrent_bounded_queue<TaskType> >();
		m_que = q;
		m_que->set_capacity(cap);

		std::thread([q, gpuid, inst_init_cb]() {
			GpuInstType _init_obj;
			inst_init_cb(_init_obj, gpuid);
			for (TaskType t;;) {
				q->pop(t);
				t(_init_obj);
			}
		}).detach();

		return 0;
	}

	bool is_init() const {
		return m_que.get() != nullptr;
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
		if (!m_que) return false;
		return m_que->try_push(tsk);
	}

	bool push_task_nb(const TaskType& tsk) {
		if (!m_que) return false;
		return m_que->try_push(tsk);
	}

private:
	std::shared_ptr<tbb::concurrent_bounded_queue<TaskType> > m_que;
};


template<class GpuInstType>
class GpuTaskPool_1Nb :no_copy {
public:
	typedef std::function<void(GpuInstType&, uint32_t)> InitType;
	typedef std::function<void(GpuInstType&, uint32_t thread_id)> TaskType;
	explicit GpuTaskPool_1Nb() {}
	~GpuTaskPool_1Nb() {}

	int init_pool(uint32_t cap, const std::vector<uint32_t>& gpu_lst, const InitType& inst_init_cb) {
		if (m_que) return 1;
		size_t ng = gpu_lst.size();
		if (ng == 0) return 10;
		auto q = std::make_shared<tbb::concurrent_bounded_queue<TaskType> >();
		m_que = q;
		m_que->set_capacity(cap);
		for (size_t i = 0; i < ng; i++) {
			uint32_t gpuid = gpu_lst[i];
			std::thread([q, gpuid, i, inst_init_cb]() {
				GpuInstType _init_obj;
				inst_init_cb(_init_obj, gpuid);
				for (TaskType t;;) {
					q->pop(t);
					t(_init_obj, (uint32_t)i);
				}
			}).detach();
		}
		return 0;
	}

	bool is_init() const {
		return m_que.get() != nullptr;
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
		if (!m_que) return false;
		return m_que->try_push(tsk);
	}

	bool push_task_nb(const TaskType& tsk) {
		if (!m_que) return false;
		return m_que->try_push(tsk);
	}

private:
	std::shared_ptr<tbb::concurrent_bounded_queue<TaskType> > m_que;
};






#endif