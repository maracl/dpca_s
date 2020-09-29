#ifndef _platform_sync_hpp_
#define _platform_sync_hpp_

#include "locker.h"
#include "event.h"
#include "sema.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
///
/// locker
///
//////////////////////////////////////////////////////////////////////////
class ThreadLocker
{
	ThreadLocker(const ThreadLocker&){}
	ThreadLocker& operator =(const ThreadLocker&){ return *this; }

public:
	ThreadLocker()
	{
		int r = locker_create(&m_locker);
		if(0 != r)
			throw r;
	}

	~ThreadLocker()
	{
		locker_destroy(&m_locker);
	}

	int Lock()
	{
		return locker_lock(&m_locker);
	}

	int Unlock()
	{
		return locker_unlock(&m_locker);
	}

	int Trylock()
	{
		return locker_trylock(&m_locker);
	}

private:
	locker_t m_locker;
};

class AutoThreadLocker
{
	AutoThreadLocker(const AutoThreadLocker& locker):m_locker(locker.m_locker){}
	AutoThreadLocker& operator =(const AutoThreadLocker&){ return *this; }

public:
	AutoThreadLocker(ThreadLocker& locker)
		:m_locker(locker)
	{
		m_locker.Lock();
	}

	~AutoThreadLocker()
	{
		m_locker.Unlock();
	}

private:
	ThreadLocker& m_locker;
};

//////////////////////////////////////////////////////////////////////////
///
/// event
///
//////////////////////////////////////////////////////////////////////////
class ThreadEvent
{
	ThreadEvent(const ThreadEvent&){}
	ThreadEvent& operator =(const ThreadEvent&){ return *this; }

public:
	ThreadEvent()
	{
		int r = event_create(&m_event);
		if(0 != r)
			throw r;
	}

	~ThreadEvent()
	{
		event_destroy(&m_event);
	}

	int Wait()
	{
		return event_wait(&m_event);
	}

	int TimeWait(int timeout)
	{
		return event_timewait(&m_event, timeout);
	}

	int Signal()
	{
		return event_signal(&m_event);
	}

	int Reset()
	{
		return event_reset(&m_event);
	}

private:
	event_t m_event;
};

//////////////////////////////////////////////////////////////////////////
///
/// semaphore
///
//////////////////////////////////////////////////////////////////////////
//class CSemaphore
//{
//	CSemaphore(const CSemaphore&){}
//	CSemaphore& operator =(const CSemaphore&){ return *this; }
//
//public:
//	CSemaphore(int initValue)
//	{
//		semaphore_create(&m_sema, NULL, initValue);
//	}
//
//	CSemaphore(const char* name)
//	{
//		int r = semaphore_open(&m_sema, name);
//		if(0 != r)
//		{
//			r = semaphore_create(&m_sema, name, 1);
//			if(0 != r)
//			{
//				r = semaphore_open(&m_sema, name);
//			}
//		}
//
//		if(0 != r)
//			throw r;
//	}
//
//	~CSemaphore()
//	{
//		semaphore_destroy(&m_sema);
//	}
//
//	int Post()
//	{
//		return semaphore_post(&m_sema);
//	}
//
//	int Wait()
//	{
//		return semaphore_wait(&m_sema);
//	}
//
//	int TimeWait(int timeout)
//	{
//		return semaphore_timewait(&m_sema, timeout);
//	}
//
//	int Trywait()
//	{
//		return semaphore_trywait(&m_sema);
//	}
//
//private:
//	semaphore_t m_sema;
//};
//
//class CAutoSemaphore
//{
//	CAutoSemaphore(const CAutoSemaphore& sema):m_sema(sema.m_sema){}
//	CAutoSemaphore& operator =(const CAutoSemaphore&){ return *this; }
//
//public:
//	CAutoSemaphore(CSemaphore& sema)
//		:m_sema(sema)
//	{
//		m_sema.Wait();
//	}
//
//	~CAutoSemaphore()
//	{
//		m_sema.Post();
//	}
//
//private:
//	CSemaphore& m_sema;
//};

#endif /* !_platform_sync_hpp_ */
