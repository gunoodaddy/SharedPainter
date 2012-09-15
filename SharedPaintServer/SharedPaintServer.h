#include "Coconut.h"

#define LISTEN_PORT	10888

class SharedPaintServer : public coconut::ServerController {
	virtual boost::shared_ptr<coconut::ClientController> onAccept(boost::shared_ptr<coconut::TcpSocket> socket);
};
