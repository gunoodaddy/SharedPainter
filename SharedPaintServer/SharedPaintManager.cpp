#include "SharedPaintManager.h"
#include "SharedPaintClient.h"
#include "SharedPaintProtocol.h"
#include "SharedPaintRoom.h"
#include "SystemPacketBuilder.h"

SharedPaintManager::SharedPaintManager( void ) {
}

void SharedPaintManager::joinRoom( boost::shared_ptr<SharedPaintClient> client, bool &firstFlag ) {

	boost::recursive_mutex::scoped_lock autolock(mutexData_);

	ROOM_MAP::iterator itR = roomMap_.find( client->user()->roomId() );	

	if( itR != roomMap_.end() ) {
		itR->second->addJoiner( client, firstFlag );
	} else {

		boost::shared_ptr<SharedPaintRoom> roomInfo(new SharedPaintRoom(client->user()->roomId()));
		roomInfo->addJoiner( client, firstFlag );

		// new room & new user
		roomMap_.insert( ROOM_MAP::value_type( client->user()->roomId(), roomInfo ) );
	}

	LOG_DEBUG("JOIN ROOM : roomid = %s, userid = %s, room count = %d, firstFlag = %d", 
		client->user()->roomId().c_str(), 
		client->user()->userId().c_str(), 
		totalUserCount(),
		firstFlag);
}

boost::shared_ptr<SharedPaintClient> SharedPaintManager::findUser( const std::string &roomid, const std::string &userId ) {
	
	boost::recursive_mutex::scoped_lock autolock(mutexData_);

	ROOM_MAP::iterator itR = roomMap_.find( roomid );	
	if( itR != roomMap_.end() ) {
		return itR->second->findUser( userId );
	}
	return boost::shared_ptr<SharedPaintClient>();
}

size_t SharedPaintManager::totalUserCount( void ) {
	boost::recursive_mutex::scoped_lock autolock(mutexData_);
	
	size_t totalCnt = 0;
	ROOM_MAP::iterator itR = roomMap_.begin();
	for( ; itR != roomMap_.end(); itR++ ) {
		totalCnt += itR->second->userCount();
	}

	return totalCnt;
}

void SharedPaintManager::leaveRoom( boost::shared_ptr<SharedPaintClient> client ) {
	boost::recursive_mutex::scoped_lock autolock(mutexData_);
	
	std::string roomid = client->user()->roomId();
	std::string userid = client->user()->userId();

	ROOM_MAP::iterator itR = roomMap_.find( roomid );

	if( itR != roomMap_.end() ) {
		itR->second->removeJoiner( client );
	}
	
	LOG_DEBUG("LEAVE ROOM : roomid = %s, userid = %s, room count = %d", 
		client->user()->roomId().c_str(), userid.c_str(), totalUserCount() );
}
	
std::string SharedPaintManager::serializeJoinerInfoPacket( const std::string &roomid ) {
	
	boost::recursive_mutex::scoped_lock autolock(mutexData_);

	ROOM_MAP::iterator itR = roomMap_.find( roomid );	
	if( itR != roomMap_.end() ) {
		return itR->second->serializeJoinerInfoPacket();
	}

	return "";
}
	
void SharedPaintManager::syncStart( const std::string &roomid, const std::string &tartgetId ) {
	boost::recursive_mutex::scoped_lock autolock(mutexData_);

	ROOM_MAP::iterator itR = roomMap_.find( roomid );	
	if( itR != roomMap_.end() ) {
		itR->second->syncStart( tartgetId );
	}
}

void SharedPaintManager::uniCast( const std::string &roomid, boost::shared_ptr<SharedPaintProtocol> prot ) {
	boost::recursive_mutex::scoped_lock autolock(mutexData_);
	
	ROOM_MAP::iterator itR = roomMap_.find( roomid );	

	if( itR != roomMap_.end() ) {
		itR->second->uniCast( prot );
	}
}

void SharedPaintManager::roomCast( const std::string &roomid, const std::string &fromid, boost::shared_ptr<SharedPaintProtocol> prot, bool sendMySelf ) {
	boost::recursive_mutex::scoped_lock autolock(mutexData_);
	
	ROOM_MAP::iterator itR = roomMap_.find( roomid );	

	if( itR != roomMap_.end() ) {
		itR->second->roomCast( fromid, prot, sendMySelf );
	}
}
	
void SharedPaintManager::setSuperPeerSession( boost::shared_ptr<SharedPaintClient> client ) {
	boost::recursive_mutex::scoped_lock autolock(mutexData_);
	
	ROOM_MAP::iterator itR = roomMap_.find( client->user()->roomId() );	

	if( itR != roomMap_.end() ) {
		itR->second->setSuperPeerSession( client );
	}
}

boost::shared_ptr<SharedPaintClient> SharedPaintManager::currentSuperPeerSession( const std::string &roomid ) {
	boost::recursive_mutex::scoped_lock autolock(mutexData_);
	
	ROOM_MAP::iterator itR = roomMap_.find( roomid );

	if( itR != roomMap_.end() ) {
		return itR->second->currentSuperPeerSession();
	}
	return boost::shared_ptr<SharedPaintClient>();
}
