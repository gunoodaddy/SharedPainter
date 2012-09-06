#pragma once

#include <boost/thread/recursive_mutex.hpp>
#include <boost/function.hpp>
#include <QObject>
#include <QCustomEvent>
#include "Singleton.h"

#define DefferdCallerPtr()		CSingleton<CDefferedCaller>::Instance()

class CDefferedCaller : public QObject
{
public:
	typedef boost::function< void () > deferredMethod_t;

	CDefferedCaller(void);
	~CDefferedCaller(void);

	bool isMainThread( void );
	void performMainThread( deferredMethod_t func );

private:
	void customEvent(QEvent* e);

private:
	std::vector<deferredMethod_t> deferredMethods_;
	boost::recursive_mutex mutex_;
	static boost::thread::id mainThreadId_;
};
