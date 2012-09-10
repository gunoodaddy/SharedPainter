#include "StdAfx.h"
#include "DefferedCaller.h"

boost::thread::id CDefferedCaller::mainThreadId_ = boost::this_thread::get_id();

CDefferedCaller::CDefferedCaller(void)
{
}

CDefferedCaller::~CDefferedCaller(void)
{
}

bool CDefferedCaller::isMainThread( void )
{
	if( mainThreadId_ == boost::this_thread::get_id() )
		return true;
	return false;
}

void CDefferedCaller::performMainThreadAlwaysDeffered( FUNC_TYPE func )
{
	mutex_.lock();

	deferredMethods_.push_back( func );

	mutex_.unlock();

	QEvent *evt = new QEvent(QEvent::User);
	QApplication::postEvent(this, evt);
}

void CDefferedCaller::performMainThread( FUNC_TYPE func )
{
	if( isMainThread() )
	{
		func();
		return;
	}

	performMainThreadAlwaysDeffered( func );
}


void CDefferedCaller::customEvent(QEvent* e)
{
	mutex_.lock();
	std::list<FUNC_TYPE> methods = deferredMethods_;
	mutex_.unlock();

	// MUST be lock-free status..
	for( std::list<FUNC_TYPE>::iterator it = methods.begin(); it != methods.end(); it++) 
	{
		(*it)();
	}

	mutex_.lock();
	if( methods.size() < deferredMethods_.size() )
	{
		int cnt = methods.size();
		while( --cnt >= 0 )
			deferredMethods_.pop_front();

		mutex_.unlock();

		CDefferedCaller::customEvent(e);	// recursive
		return;
	}

	deferredMethods_.clear();
	mutex_.unlock();
}
