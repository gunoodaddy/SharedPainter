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

#include "StdAfx.h"
#include "DefferedCaller.h"

boost::thread::id CDefferedCaller::mainThreadId_ = boost::this_thread::get_id();

CDefferedCaller::CDefferedCaller(void) : autoDelete_(false)
{
	qDebug() << "CDefferedCaller()" << this;
}

CDefferedCaller::~CDefferedCaller(void)
{
	qDebug() << "~CDefferedCaller()" << this << autoDelete_;
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


bool CDefferedCaller::performMainThreadAfterMilliseconds( FUNC_TYPE func, int msec )
{
	mutex_.lock();

	boost::shared_ptr<methodtimer_t> data(new methodtimer_t);
	data->remain_msec = msec;
	data->func = func;
	data->timer_start = false;

	deferredMethodsForTimer_.push_back(data);

	mutex_.unlock();

	QEvent *evt = new QEvent(QEvent::User);
	QApplication::postEvent(this, evt);
	return true;
}


void CDefferedCaller::timerEvent( void )
{
	mutex_.lock();

	qint64 now = QDateTime::currentMSecsSinceEpoch();
	for( TIMER_LIST::iterator it = deferredMethodsForTimer_.begin(); it != deferredMethodsForTimer_.end(); it++) 
	{
		boost::shared_ptr<methodtimer_t> data = (*it);
		if( (*it)->timer_start )
		{
			qint64 diffMsec = now - data->tick;
			assert( diffMsec >= 0 );

			data->remain_msec -= diffMsec;
			data->tick = now;
		}
	}

	TIMER_LIST methodsForTimer = deferredMethodsForTimer_;

	mutex_.unlock();

	for( TIMER_LIST::iterator it = methodsForTimer.begin(); it != methodsForTimer.end(); it++) 
	{
		if( (*it)->remain_msec < 10 )
		{
			(*it)->func();

			mutex_.lock();
			TIMER_LIST::iterator itF = std::find( deferredMethodsForTimer_.begin(), deferredMethodsForTimer_.end(), *it );
			if( itF != deferredMethodsForTimer_.end() )
				deferredMethodsForTimer_.erase( itF );
			mutex_.unlock();
		}
	}

	if( deferredMethodsForTimer_.size() <= 0 && autoDelete_ )
		delete this;
}

void CDefferedCaller::customEvent(QEvent* e)
{
	mutex_.lock();
	TIMER_LIST methodsForTimer = deferredMethodsForTimer_;
	FUNC_LIST methods = deferredMethods_;
	mutex_.unlock();

	// MUST be lock-free status..
	for( TIMER_LIST::iterator it = methodsForTimer.begin(); it != methodsForTimer.end(); it++) 
	{
		boost::shared_ptr<methodtimer_t> data = (*it);
		if( false == data->timer_start )
		{
			data->tick = QDateTime::currentMSecsSinceEpoch();
			QTimer::singleShot( data->remain_msec, this, SLOT(timerEvent()) );
			data->timer_start = true;
		}
	}

	for( FUNC_LIST::iterator it = methods.begin(); it != methods.end(); it++) 
	{
		(*it)();
	}

	if( methodsForTimer.size() != deferredMethodsForTimer_.size() )
	{
		CDefferedCaller::customEvent(e);	// recursive
		return;
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

	if( deferredMethodsForTimer_.size() <= 0 && autoDelete_ )
		delete this;
}
