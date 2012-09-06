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

void CDefferedCaller::performMainThread( deferredMethod_t func )
{
	mutex_.lock();

	deferredMethods_.push_back( func );

	mutex_.unlock();

	QEvent *evt = new QEvent(QEvent::User);
	QApplication::postEvent(this, evt);
}


void CDefferedCaller::customEvent(QEvent* e)
{
	mutex_.lock();

	for(size_t i = 0; i < deferredMethods_.size(); i++) 
	{
		deferredMethods_[i]();
	}

	deferredMethods_.clear();

	mutex_.unlock();
}
