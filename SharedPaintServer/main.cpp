#include "Coconut.h"
#include "SharedPaintClient.h"
#include "SharedPaintServer.h"

int main(int argc, char* argv[]) {
	
	coconut::IOServiceContainer ioServiceContainer(4);
	ioServiceContainer.initialize();

	coconut::logger::setLogLevel(coconut::logger::LEVEL_TRACE);
	coconut::setUseLittleEndianForNetwork( true );
	coconut::setEnableDebugMode();

	try {
		boost::shared_ptr<SharedPaintServer> serverController(new SharedPaintServer);

		coconut::NetworkHelper::listenTcp(&ioServiceContainer, LISTEN_PORT, serverController);

		LOG_INFO("tcpserver started..");
		ioServiceContainer.run();
	} catch(coconut::Exception &e) {
		LOG_FATAL("Exception emitted : %s\n", e.what());
	}
	return 0;
}

