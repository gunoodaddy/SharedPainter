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

	boost::shared_ptr<CNetPeerSession> newConnect( const std::string &ip, int port )
	{
		boost::shared_ptr<CNetPeerSession> session = boost::shared_ptr<CNetPeerSession>(new CNetPeerSession( io_service_, newSessionId() ));
		session->connect( ip, port );
		
		return session;
	}

	void close( void )
	{
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

		thread_ = boost::thread(boost::bind(&CNetServiceRunner::_threadMain, this));

		stopped_ = false;
		threadStarted_ = true;
	}

	void _stop_thread( void )
	{
		mutex_.lock();
	
		if( !threadStarted_ )
			return;

		mutex_.unlock();

		thread_.join();

		mutex_.lock();
		threadStarted_ = false;
		mutex_.unlock();
	}

	void _threadMain( void )
	{
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
