/* 
 * File:   thread_util.h
 * Author: ltmit
 *
 * Created on 2013年8月20日, 上午10:30
 */

#ifndef THREAD_UTIL_H
#define	THREAD_UTIL_H

typedef void (cm_thread_func)(void*);

#ifdef __cplusplus
//////////////////////////////////////////////////////
namespace __tut{

class tibase{
	tibase(const tibase&);
public:
	tibase(){}
	virtual ~tibase() {}
	virtual void run()=0;
};

template<class T,class F>
class threadinfo:public tibase{
	T para;
	F func;
public:
	threadinfo(T& p,F& f):para(p),func(f) {}
	virtual ~threadinfo() {}
	virtual void run() { func(para); }
};

template<class T1,class T2,class F>
class threadinfo2:public tibase{
	T1 para1;	T2 para2;
	F func;
public:
	threadinfo2(T1& p1,T2& p2,F& f):para1(p1),para2(p2),func(f) {}
	virtual ~threadinfo2() {}
	virtual void run() { func(para1,para2); }
};

template<class T1,class T2,class T3,class F>
class threadinfo3:public tibase{
	T1 para1;	T2 para2;	T3 para3;
	F func;
public:
	threadinfo3(T1& p1,T2& p2,T3& p3,F& f):para1(p1),para2(p2),para3(p3),func(f) {}
	virtual ~threadinfo3() {}
	virtual void run() { func(para1,para2,para3); }
};

}//////////////////////////////////////////////////
#endif
#ifdef __linux  ///linux system////

#include <pthread.h>
#include <stdlib.h>

#ifdef __cplusplus
namespace __tut{

inline static void* _unix_dummy_thread_func(void* para)
{
	tibase* ti((tibase*)para);
	ti->run();
	delete ti;
	return 0;
}

}///////////////////

template<typename T,class F>
inline static int create_thread_v2(F func,T& para) {
	typedef __tut::threadinfo<T,F> ti_type;
	ti_type* pti=new ti_type(para,func);
	pthread_t thr=0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	int rt=pthread_create(&thr,&attr,__tut::_unix_dummy_thread_func,pti);
	pthread_attr_destroy(&attr);
	if(rt) {
		delete pti;
		return -2;
	}
	return 0;
}

template<typename T1,typename T2,class F>
inline static int create_thread_v2(F func,T1& para1,T2& para2) {
	typedef __tut::threadinfo2<T1,T2,F> ti_type;
	ti_type* pti=new ti_type(para1,para2,func);
	pthread_t thr=0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	int rt=pthread_create(&thr,&attr,__tut::_unix_dummy_thread_func,pti);
	pthread_attr_destroy(&attr);
	if(rt) {
		delete pti;
		return -2;
	}
	return 0;
}

template<typename T1,typename T2,typename T3,class F>
inline static int create_thread_v2(F func,T1& para1,T2& para2,T3& para3) {
	typedef __tut::threadinfo3<T1,T2,T3,F> ti_type;
	ti_type* pti=new ti_type(para1,para2,para3,func);
	pthread_t thr=0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	int rt=pthread_create(&thr,&attr,__tut::_unix_dummy_thread_func,pti);
	pthread_attr_destroy(&attr);
	if(rt) {
		delete pti;
		return -2;
	}
	return 0;
}
#endif///__cplusplus

inline static void* __dummy_thread_func(void* para)
{
	register cm_thread_func* f=(cm_thread_func*)((void**)para)[0];
	register void* p=(void*) ((void**)para)[1];
	free(para);
	try{
		f(p);
	}catch(...){}

	return NULL;
}

inline static int create_thread(cm_thread_func* func,void* para)
{
	unsigned long* p=(unsigned long*)malloc(sizeof(void*)*2);
	if(p==NULL) return -1;
	p[0]=(unsigned long)func;	p[1]=(unsigned long)para;
	pthread_t thr=0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	int rt=pthread_create(&thr,&attr,__dummy_thread_func,p);
	pthread_attr_destroy(&attr);
	if(rt!=0) {
		free(p);
		return -2;
	}
	return 0;
}


#elif defined(_WIN32) ///windows system///
#include <ws2tcpip.h>
#include <process.h>

//////////////////////////////////////////////////////
namespace __tut{

inline static unsigned int __stdcall _win_dummy_thread_func(void* para)
{
	tibase* ti((tibase*)para);
	ti->run();
	delete ti;
	return 0;
}
}///////////////////
inline static UINT __stdcall __dummy_thread_func(void* para)
{
	register cm_thread_func* f=(cm_thread_func*)((void**)para)[0];
	register void* p=(void*) ((void**)para)[1];
	free(para);
	try{
		f(p);
	}catch(...){}	

	return 0;
}

inline static int create_thread(cm_thread_func* func,void* para)
{
	void** p=(void**)malloc(sizeof(void*)*2);
	if(p==NULL) return -1;
	p[0]=(void* )func;	p[1]=(void*)para;
	HANDLE h=(HANDLE)_beginthreadex(NULL,0,__dummy_thread_func,(void*)p,0,NULL);
	if(h==NULL) {
		free(p);
		return -2;
	}
	CloseHandle(h);
	return 0;
}


template<typename T,class F>
inline static int create_thread_v2(F func,T& para) {
	typedef __tut::threadinfo<T,F> ti_type;
	ti_type* pti=new ti_type(para,func);
	HANDLE h=(HANDLE)_beginthreadex(NULL,0,__tut::_win_dummy_thread_func,(void*)pti,0,NULL);
	if(h==NULL) {
		delete pti;
		return -2;
	}
	CloseHandle(h);
	return 0;
}

template<typename T1,typename T2,class F>
inline static int create_thread_v2(F func,T1& para1,T2& para2) {
	typedef __tut::threadinfo2<T1,T2,F> ti_type;
	ti_type* pti=new ti_type(para1,para2,func);
	HANDLE h=(HANDLE)_beginthreadex(NULL,0,__tut::_win_dummy_thread_func,(void*)pti,0,NULL);
	if(h==NULL) {
		delete pti;
		return -2;
	}
	CloseHandle(h);
	return 0;
}

template<typename T1,typename T2,typename T3,class F>
inline static int create_thread_v2(F func,T1& para1,T2& para2,T3& para3) {
	typedef __tut::threadinfo3<T1,T2,T3,F> ti_type;
	ti_type* pti=new ti_type(para1,para2,para3,func);
	HANDLE h=(HANDLE)_beginthreadex(NULL,0,__tut::_win_dummy_thread_func,(void*)pti,0,NULL);
	if(h==NULL) {
		delete pti;
		return -2;
	}
	CloseHandle(h);
	return 0;
}

#endif

#endif	/* THREAD_UTIL_H */

