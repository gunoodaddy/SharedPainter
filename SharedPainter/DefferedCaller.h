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

	CDefferedCaller(void);
	~CDefferedCaller(void);

	bool isMainThread( void );
	void performMainThreadAlwaysDeffered( FUNC_TYPE func );
	void performMainThread( FUNC_TYPE func );	// if on main thread now, just call directly!

private:
	void customEvent(QEvent* e);

private:
	std::list<FUNC_TYPE> deferredMethods_;
	boost::recursive_mutex mutex_;
	static boost::thread::id mainThreadId_;
};
