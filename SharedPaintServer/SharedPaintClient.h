#pragma once

#include <boost/thread/recursive_mutex.hpp>
#include "SharedPaintController.h"
#include "PaintUser.h"
#include "TcpTestClient.h"

using namespace coconut;


class SharedPaintClient : public SharedPaintController {
public:
	SharedPaintClient();
	~SharedPaintClient();

public:
	static void initialize(IOServiceContainer *container) {
		gIOServiceContainer_ = container;
		gProtocolFactory_ = boost::shared_ptr<SharedPaintController::SharedPaintProtocolFactory>(
									new SharedPaintController::SharedPaintProtocolFactory);
	}

	boost::shared_ptr<CPaintUser> user( void ) { return user_; }

	void setInvalidSessionFlag( void ) { invalidSessionFlag_ = true; }

protected:
	void onSharedPaintReceived(boost::shared_ptr<SharedPaintProtocol> prot);
	void onClosed( void );
	void onError(int error, const char *strerror);
	void onTimer(unsigned short id);

private:
	void checkIfSuperPeer( void );
	void _handle_CODE_SYSTEM_JOIN_SERVER(boost::shared_ptr<SharedPaintProtocol> prot);
	void _handle_CODE_SYSTEM_LEAVE(boost::shared_ptr<SharedPaintProtocol> prot);
	void _handle_CODE_SYSTEM_TCPACK(boost::shared_ptr<SharedPaintProtocol> prot);
	void _handle_CODE_SYSTEM_SYNC_REQUEST(boost::shared_ptr<SharedPaintProtocol> prot);

private:
	static boost::shared_ptr<SharedPaintController::SharedPaintProtocolFactory> gProtocolFactory_;
	static IOServiceContainer *gIOServiceContainer_;
	boost::shared_ptr<CPaintUser> user_;

	bool invalidSessionFlag_;
	boost::shared_ptr<TcpTestClient> testClient_;
};

