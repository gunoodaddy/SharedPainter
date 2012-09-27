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

	if( autoDelete_ )
		delete this;
}
