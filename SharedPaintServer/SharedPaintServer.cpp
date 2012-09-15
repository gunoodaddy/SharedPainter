#include "Coconut.h"
#include "SharedPaintServer.h"
#include "SharedPaintClient.h"

boost::shared_ptr<coconut::ClientController> SharedPaintServer::onAccept(boost::shared_ptr<coconut::TcpSocket> socket) {
	boost::shared_ptr<SharedPaintClient> newController(new SharedPaintClient()); 
	return newController;
}
