#pragma once

#include "Coconut.h"
#include "Singleton.h"
#include "SharedPaintCodeDefine.h"
#include "PaintUser.h"

#define SharedPaintManagerPtr()		CSingleton<SharedPaintManager>::Instance()

using namespace coconut;

class SharedPaintRoom;
class SharedPaintClient;
class SharedPaintProtocol;

class SharedPaintManager 
{
public:
	SharedPaintManager( void );

	void joinRoom( boost::shared_ptr<SharedPaintClient> client );

	void leaveRoom( const std::string &roomid, const std::string &userid );

	void uniCast( const std::string &roomid, boost::shared_ptr<SharedPaintProtocol> prot );

	void roomCast( const std::string &roomid, const std::string &fromid, boost::shared_ptr<SharedPaintProtocol> prot, bool sendMySelf = false );

	std::string generateJoinerInfoPacket( const std::string &roomid );

	void syncStart( const std::string &roomid, const std::string &tartgetId );

	size_t totalUserCount( void );

private:
	typedef std::map< std::string, boost::shared_ptr<SharedPaintRoom> > ROOM_MAP;
	ROOM_MAP roomMap_;

	boost::recursive_mutex mutexData_;
};
