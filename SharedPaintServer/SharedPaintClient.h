#pragma once

#include <boost/thread/recursive_mutex.hpp>
#include "SharedPaintController.h"
#include "PaintUser.h"

using namespace coconut;


class SharedPaintClient : public SharedPaintController {
public:
	SharedPaintClient();
	~SharedPaintClient();

public:
	static void initialize(IOServiceContainer *container) {
		gProtocolFactory_ = boost::shared_ptr<SharedPaintController::SharedPaintProtocolFactory>(
									new SharedPaintController::SharedPaintProtocolFactory);
	}

	boost::shared_ptr<CPaintUser> user( void ) { return user_; }

	void setInvalidSessionFlag( void ) { invalidSessionFlag_ = true; }

protected:
	void onSharedPaintReceived(boost::shared_ptr<SharedPaintProtocol> prot);
	void onClosed( void );
	void onError(int error, const char *strerror);

private:
	void _handle_CODE_SYSTEM_JOIN(boost::shared_ptr<SharedPaintProtocol> prot);
	void _handle_CODE_SYSTEM_LEAVE(boost::shared_ptr<SharedPaintProtocol> prot);

private:
	static boost::shared_ptr<SharedPaintController::SharedPaintProtocolFactory> gProtocolFactory_;
	boost::shared_ptr<CPaintUser> user_;

	bool invalidSessionFlag_;
};

