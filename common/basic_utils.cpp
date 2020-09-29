#include "basic_utils.h"
#ifdef _WIN32
#include <ltmit/fast_sync.h>
#else
#include <time.h>
#include "fast_sync_cls.hpp"
#endif
#include <thread>
#include <sys/syscall.h>

#ifdef _WIN32 //for windows implement

//#define gettid() syscall(SYS_gettid)

void SimpleNotifier::wait()
{
	WaitForKeyedEvent(NULL, &_wk, 0);
}

void SimpleNotifier::wait(int millseconds)
{
	//timeout in 100-nanosecond units
	LARGE_INTEGER timeout;
	timeout.QuadPart = millseconds * 10000;
	WaitForKeyedEvent(NULL, &_wk, 0, &timeout);
}

void SimpleNotifier::wake()
{
	ReleaseKeyedEvent(NULL, &_wk, 0);
}

void wait_ftxp(void* p)
{
	WaitForKeyedEvent(NULL, p, 0);
}

void wake_ftxp(void* p)
{
	ReleaseKeyedEvent(NULL, p, 0);
}

HiresTimer::HiresTimer()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&iFreq);
}

void HiresTimer::start()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&iStart);
}

void HiresTimer::end()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&iEnd);
}

double HiresTimer::getTime() const {
	return (iEnd - iStart) / (double)iFreq;
}
#else //linux implement

void SimpleNotifier::wake(){	if (__sync_fetch_and_add(&_wk, 1L) == 0) {		futex_wake(&_wk, 1111);	}
}
void SimpleNotifier::wait()
{
	futex_wait(&_wk, 0, NULL);
}

void SimpleNotifier::wait(int seconds)
{
	//timespec timeout = { 0, millseconds*1000*1000};
	timespec  tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
    timespec timeout = {seconds , tp.tv_nsec};
	futex_wait(&_wk, 0, &timeout);
}

void SimpleNotifier::wait2(int millseconds)
{
	timespec timeout = { 0, millseconds*1000*1000};
	//timespec  tp;
	//	//clock_gettime(CLOCK_MONOTONIC, &tp);
	//	    //timespec timeout = {millseconds , tp.tv_nsec};
	futex_wait(&_wk, 0, &timeout);
}

void wake_ftxp(void* p)
{
	if (__sync_fetch_and_add((int*)p, 1) == 0) {
		futex_wake(p, 1111);
	}
}

void wait_ftxp(void* p)
{	futex_wait(p, 0, NULL);
}

static double get_time(){
	timespec  tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return tp.tv_sec + tp.tv_nsec*1e-9;
}
HiresTimer::HiresTimer()
{
	dStart = dEnd = 0;
}
void HiresTimer::start()
{
	dStart = get_time();
}
void HiresTimer::end()
{
	dEnd = get_time();}
double HiresTimer::getTime() const {
	return dEnd - dStart;
}

#endif

FTaskPool::FTaskPool(size_t cap)
{
	m_que.set_capacity(cap);
}

FTaskPool::~FTaskPool()
{

}

void FTaskPool::clear()
{
	m_que.clear();
}

int FTaskPool::init_pool(int nthread)
{
	for (int i = 0; i < nthread; i++) {
		std::thread thr([&](int i){
			while (1) {
				FuntorType tt;
				m_que.pop(tt);
				tt(i);
			}
		}, i);
		thr.detach();
	}
	return 0;
}

void FTaskPool::push_task(FuntorType&& tsk)
{
	m_que.push(tsk);
}

bool FTaskPool::push_task_nb(FuntorType&& tsk)
{
	return m_que.try_push(tsk);
}

void FTaskPool::push_task(const FuntorType& tsk)
{
	m_que.push(tsk);
}

bool FTaskPool::push_task_nb(const FuntorType& tsk)
{
	return m_que.try_push(tsk);
}


void FTaskPool2::clear()
{
	m_que.clear();
}

int FTaskPool2::init_pool(int nthread)
{
	for (int i = 0; i < nthread; i++) {
		std::thread thr([&](int i){
			while (1) {
				FuntorType tt;
				m_que.pop(tt);
				tt();
			}
		}, i);
		thr.detach();
	}
	return 0;
}

void FTaskPool2::push_task(const FuntorType& tsk)
{
	m_que.push(tsk);
}

bool FTaskPool2::push_task_nb(const FuntorType& tsk)
{
	return m_que.try_push(tsk);
}

int FTaskPool3::init_pool(int cap, int nthread)
{
	if (!m_vqs.empty()) return 1;
	size_t ng = nthread;
	if (ng == 0) return 10;
	m_vqs.reserve(ng);
	for (size_t i = 0; i < ng; i++) {
		auto task_que = std::make_shared<tbb::concurrent_bounded_queue<FuntorType> >();
		task_que->set_capacity(cap);
		m_vqs.push_back(task_que);
		std::thread thr([task_que,this](int i){
			printf("the FtaskTool thread id is %d~~%d\n",(int)syscall(SYS_gettid),(int)i);
			while (1) {
				FuntorType tt;
				task_que->pop(tt);
				//printf("the FtaskTool thread the quit flag is %d~~%d\n",(int)_quitflg,i);
				if(_quitflg < 0) break;
				tt();
			}
			printf("end FtaskTool thread id is %d~~%d\n",(int)syscall(SYS_gettid),(int)i);
		}, i);
		thr.detach();

	}
	return 0;
}

void FTaskPool3::push_task(const FuntorType& tsk, uint32_t nt)
{
	if (m_vqs.empty()) return;
	m_vqs[nt % (uint32_t)m_vqs.size()]->push(tsk);
}

bool FTaskPool3::push_task_nb(const FuntorType& tsk, uint32_t nt)
{
	if (m_vqs.empty()) return false;
	return m_vqs[nt % (uint32_t)m_vqs.size()]->try_push(tsk);
}

void FTaskPool3::clear()
{
   for(size_t i = 0;i < (size_t)m_vqs.size(); i++)
   {
      m_vqs[i]->clear();
   }
}

FTaskPool3::~FTaskPool3()
{
  _quitflg.fetch_sub(1);
  for(size_t i = 0;i < (size_t)m_vqs.size(); i++)
  {
	  FuntorType tt;
      m_vqs[i]->push(tt);
	  printf("push FtaskTool thread the queue size is %d~~\n",(int)m_vqs.size());
  }

}

//////////////////////////////////////////////////////////////////////////
void FTaskPool4::clear()
{
	m_que.clear();
}

int FTaskPool4::init_pool(int nthread)
{
        _fun_vec.resize(nthread);
        for (int i = 0; i < _fun_vec.size(); i++)
        {
            _fun_vec.at(i) = std::thread([&](int i){
                 while(1){
                      FuntorType tt;
                      m_que.pop(tt);
		      if(_quitflg < 0) break;
                      tt();
                 }
            },i);
        } 
	//for (int i = 0; i < nthread; i++) {
	//	std::thread thr([&](int i){
	//		while (1) {
	//			FuntorType tt;
	//			m_que.pop(tt);
	//			tt();
	//		}
	//	}, i);
	//	thr.detach();
	//}
	return 0;
}

void FTaskPool4::push_task(const FuntorType& tsk)
{
	if(_quitflg < 0) return;
	m_que.push(tsk);
}

bool FTaskPool4::push_task_nb(const FuntorType& tsk)
{
	if(_quitflg < 0) return 0;
	return m_que.try_push(tsk);
}

FTaskPool4::~FTaskPool4()
{
  _quitflg.fetch_sub(1);
  for(int i = 0;i < _fun_vec.size(); i++)
  { 
     FuntorType tt;
     m_que.push(tt);
     //printf("push FtaskTool thread the queue size is %d~~\n",(int)m_vqs.size());
  }
  printf("begin wait thread exit!\n");
  for(int j = 0;j < _fun_vec.size(); j++) 
  {
     if(_fun_vec[j].joinable())
        _fun_vec[j].join(); 
     
     printf("end %d thread exit!\n",j);
  }
  printf("end wait thread exit!\n");
}
