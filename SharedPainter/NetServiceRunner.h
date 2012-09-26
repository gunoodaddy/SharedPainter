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

#include <boost/thread.hpp>  
#include <boost/asio.hpp>

class CNetServiceRunner
{
public:

	CNetServiceRunner(void) : stopped_(true), threadStarted_(false), idle_timer_(io_service_)
	{
		startIdleTimer();
	}

	~CNetServiceRunner(void)
	{
	}

	boost::asio::io_service& io_service( void ) { return io_service_; }

	boost::shared_ptr<CNetPeerSession> newSession( void )
	{
		boost::shared_ptr<CNetPeerSession> session = boost::shared_ptr<CNetPeerSession>(new CNetPeerSession( io_service_, newSessionId() ));
		
		return session;
	}

	void waitForExit( void )
	{
		_stop_thread();
	}

	void close( void )
	{
		qDebug() << "CNetServiceRunner::close called";

		mutex_.lock();
		stopped_ = true;
		mutex_.unlock();

		io_service_.stop();
		idle_timer_.cancel();

		_stop_thread();
	}

	void startIdleTimer( void )
	{
		idle_timer_.expires_from_now(boost::posix_time::seconds(1));
		idle_timer_.async_wait( boost::bind(&CNetServiceRunner::_handle_idle_timer, this) );
		
		_start_thread();
	}
public:
	static int newSessionId( void )
	{
		static int sessionIdPool = 0;
		return sessionIdPool++;
	}

private:
	void _start_thread( void )
	{
		boost::recursive_mutex::scoped_lock autolock(mutex_);

		if( threadStarted_ )
			return;

		stopped_ = false;
		threadStarted_ = true;

		thread_ = boost::thread(boost::bind(&CNetServiceRunner::_threadMain, this));
	}

	void _stop_thread( void )
	{
		if( !threadStarted_ )
			return;

		thread_.join();

		mutex_.lock();
		threadStarted_ = false;
		mutex_.unlock();
	}

	void _threadMain( void )
	{
		if( stopped_ )
			return;

		qDebug() << "IO Thread Started";
	
		io_service_.run();

		qDebug() << "IO Thread Finished";

		mutex_.lock();
		threadStarted_ = false;
		mutex_.unlock();
	}

	void _handle_idle_timer()
	{
		if( stopped_ )
			return;

		startIdleTimer();
	}


private:
	volatile bool stopped_;
	volatile bool threadStarted_;

	boost::thread thread_;
	boost::asio::io_service io_service_;
	boost::asio::deadline_timer idle_timer_;
	boost::recursive_mutex mutex_;
};
