#include <Coconut.h>
#include "SharedPaintController.h"

void SharedPaintController::onReceivedProtocol(boost::shared_ptr<protocol::BaseProtocol> protocol) {
	boost::shared_ptr<SharedPaintProtocol> gtProt = boost::static_pointer_cast<SharedPaintProtocol>(protocol);
	onSharedPaintReceived(gtProt);
}
