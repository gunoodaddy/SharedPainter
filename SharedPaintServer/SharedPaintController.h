#pragma once

#include "SharedPaintProtocol.h"

using namespace coconut;
using namespace coconut::protocol;

class SharedPaintController : public ClientController {
public:
	class SharedPaintProtocolFactory : public protocol::BaseProtocolFactory {
	public:
		boost::shared_ptr<protocol::BaseProtocol> makeProtocol() {
			boost::shared_ptr<SharedPaintProtocol> prot(new SharedPaintProtocol);
			return prot;
		}
	};
private:
	void _onPreInitialized() {
		boost::shared_ptr<SharedPaintProtocolFactory> factory(new SharedPaintProtocolFactory);
		setProtocolFactory(factory);
		
		BaseController::_onPreInitialized();
	}

	void onReceivedProtocol(boost::shared_ptr<protocol::BaseProtocol> protocol);

protected:
	// SharedPaintController callback event
	virtual void onSharedPaintReceived(boost::shared_ptr<SharedPaintProtocol> prot) = 0;
};
