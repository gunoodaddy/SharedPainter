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

	void joinRoom( boost::shared_ptr<SharedPaintClient> client, bool &firstFlag );

	void leaveRoom( boost::shared_ptr<SharedPaintClient> client );

	void uniCast( const std::string &roomid, boost::shared_ptr<SharedPaintProtocol> prot );

	void roomCast( const std::string &roomid, const std::string &fromid, boost::shared_ptr<SharedPaintProtocol> prot, bool sendMySelf = false );

	void setSuperPeerSession( boost::shared_ptr<SharedPaintClient> client );
	boost::shared_ptr<SharedPaintClient> currentSuperPeerSession( const std::string &roomid );

	std::string serializeJoinerInfoPacket( const std::string &roomid );

	void syncStart( const std::string &roomid, const std::string &tartgetId );

	boost::shared_ptr<SharedPaintClient> findUser( const std::string &roomid, const std::string &userId );

	size_t totalUserCount( void );

private:
	typedef std::map< std::string, boost::shared_ptr<SharedPaintRoom> > ROOM_MAP;
	ROOM_MAP roomMap_;

	boost::recursive_mutex mutexData_;
};
