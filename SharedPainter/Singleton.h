/*                                                                                                                                           
* Copyright (c) 2012, Eunhyuk Kim(gunoodaddy) 
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*   * Neither the name of Redis nor the names of its contributors may be used
*     to endorse or promote products derived from this software without
*     specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#if ! defined(WIN32)
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
