#pragma once

#if ! defined(WIN32) // PORTING
#include <pthread.h>
#endif
#include <stdexcept>

class CS
{
#if defined(WIN32)
private:
	CRITICAL_SECTION m_lock;
public:
	CS() { ::InitializeCriticalSectionAndSpinCount( &m_lock, 4000 ); }
	~CS() { ::DeleteCriticalSection( &m_lock ); }
	void Lock() { ::EnterCriticalSection( &m_lock ); }
	void Unlock() { ::LeaveCriticalSection( &m_lock ); }
#else
private:
	pthread_mutex_t cs_;
public:
	CS()
	{
		pthread_mutexattr_t attr;
		if (pthread_mutexattr_init(&attr) != 0){}
		if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) {}
		if (pthread_mutex_init(&cs_, &attr) != 0) {}
		if (pthread_mutexattr_destroy(&attr) != 0) {}
	}
	~CS() { pthread_mutex_destroy(&cs_); }
	void Lock(void)	{ pthread_mutex_lock(&cs_); }	
	void Unlock(void) { pthread_mutex_unlock(&cs_); }
#endif
public:
	class Scoped
	{
		CS* m_pLock;
	public:
		Scoped(CS* pLock) : m_pLock(pLock) { m_pLock->Lock(); }
		~Scoped() { m_pLock->Unlock(); }
	};
};


template<typename T>
class CSingleton
{
private:
	static volatile T *instance_;
	static bool destroyed_;
	static CS m_lock;

public:
	inline static T* Instance(void)
	{
		if (instance_ == NULL)
		{
			MakeInstance();
		}

		return const_cast<T*>(instance_);
	}
	
	static void ForceToDelete(void)
	{
		delete instance_;
		instance_ = NULL;

		destroyed_ = true;
	}

private:
	CSingleton(void)
	{	
	}

	static void MakeInstance(void)
	{			
		CS::Scoped scoped(&m_lock);

		if (instance_ == NULL)
		{
			//if (destroyed_ == true)
			//{
			//	OnDeadReference();
			//}

			instance_ = new T;
			if (NULL == instance_)
			{
#if defined(ANDROID)
				ASSERT(false);
#else
				throw std::bad_alloc();
#endif
			}

//			atexit(ForceToDelete);
		}
	}

	static void OnDeadReference(void)
	{
#if defined(ANDROID)
		ASSERT(false);
#else
		throw std::logic_error("Dead reference detected.");
#endif
	}
};

template<typename T> volatile T *CSingleton<T>::instance_ = NULL;
template<typename T> bool CSingleton<T>::destroyed_ = false;
template<typename T> CS CSingleton<T>::m_lock;
