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

#include <boost/thread/recursive_mutex.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <QObject>
#include <QCustomEvent>
#include "Singleton.h"

#define DefferdCallerPtr()		CSingleton<CDefferedCaller>::Instance()

class CDefferedCaller : public QObject
{
public:
	typedef boost::function< void () > FUNC_TYPE;

	static void singleShot( FUNC_TYPE func ) 
	{
		CDefferedCaller *caller = new CDefferedCaller;
		caller->setAutoDelete();
		caller->performMainThreadAlwaysDeffered( func );
	}

	CDefferedCaller(void);
	~CDefferedCaller(void);

	bool isMainThread( void );
	void setAutoDelete( void ) { autoDelete_ = true; }
	void performMainThreadAlwaysDeffered( FUNC_TYPE func );
	void performMainThread( FUNC_TYPE func );	// if on main thread now, just call directly!

private:
	void customEvent(QEvent* e);

private:
	bool autoDelete_;
	std::list<FUNC_TYPE> deferredMethods_;
	boost::recursive_mutex mutex_;
	static boost::thread::id mainThreadId_;
};
