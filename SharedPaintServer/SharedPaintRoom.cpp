#include "Coconut.h"
#include "SharedPaintRoom.h"
#include "SharedPaintProtocol.h"
#include "SystemPacketBuilder.h"
#include "SharedPaintClient.h"

void SharedPaintRoom::addJoiner( boost::shared_ptr<SharedPaintClient> joiner, bool &firstFlag ) {
	CLIENT_MAP::iterator itC = clientMap_.find( joiner->user()->userId() );
	if( itC == clientMap_.end() ) {
		// new user
		clientMap_.insert( CLIENT_MAP::value_type( joiner->user()->userId(), joiner ) );
		joiner->user()->setSyncComplete();	// first joiner is alway sync-complete status
	} else { 

		if( joiner != itC->second ) {
			// already joined user. duplicate!
			itC->second->setInvalidSessionFlag();
			itC->second->tcpSocket()->close();
			itC->second = joiner;	// overwrite
		} 
	}

	firstFlag = clientMap_.size() == 1 ? true : false;
}


void SharedPaintRoom::removeJoiner( boost::shared_ptr<SharedPaintClient> joiner ) {

	std::string userid = joiner->user()->userId();
	CLIENT_MAP::iterator itC = clientMap_.find( userid );
	if( itC != clientMap_.end() ) {

		clientMap_.erase( itC );

		if( superPeerSession_ == joiner ) {
			superPeerSession_ = boost::shared_ptr<SharedPaintClient>();	// clear
			tossSuperPeerRightToCandidates();
		}
	
		// notification
		boost::shared_ptr<SharedPaintProtocol> prot = SystemPacketBuilder::LeftUser::make( roomId_, userid );
		roomCast( userid, prot, false );

		LOG_DEBUG("LEAVE ROOM : %s, %s, %d", roomId_.c_str(), userid.c_str(), clientMap_.size() );
	}
}


boost::shared_ptr<SharedPaintClient> SharedPaintRoom::findUser( const std::string &userId ) {
	
	CLIENT_MAP::iterator itC = clientMap_.find( userId );
	if( itC != clientMap_.end() ) {
		return itC->second;
	}
	return boost::shared_ptr<SharedPaintClient>();
}

void SharedPaintRoom::tossSuperPeerRightToCandidates( void ) {

	CLIENT_MAP::iterator itC = clientMap_.begin();
	for( ; itC != clientMap_.end(); itC++ ) {
		boost::shared_ptr<SharedPaintClient> client = itC->second;

		if( client->user()->isSuperPeerCandidate() ) {
			setSuperPeerSession( client );
			break;
		}
	}
}

std::string SharedPaintRoom::serializeJoinerInfoPacket( void ) {

	int pos = 0;
	std::string body;

	pos += PacketBufferUtil::writeInt16( body, pos, clientMap_.size(), false );

	std::string serializedUsers;
	CLIENT_MAP::iterator itC = clientMap_.begin();
	for( ; itC != clientMap_.end(); itC++ ) {
		serializedUsers += itC->second->user()->serialize();
	}

	return body + serializedUsers;
}

void SharedPaintRoom::syncStart( const std::string &tartgetId ) {

	std::string runner;

	// first try
	CLIENT_MAP::iterator itC = clientMap_.begin();
	for( int i = 0; itC != clientMap_.end(); itC++, i++ ) {
		if( i == lastSyncRunnerIndex_ )
			continue;
		if( tartgetId == itC->first)
			continue;
		runner = itC->first;
	}

	// second try
	if( runner.empty() ) {
		CLIENT_MAP::iterator itC = clientMap_.begin();
		for( ; itC != clientMap_.end(); itC++ ) {
			if( tartgetId == itC->first)
				continue;
			runner = itC->first;
		}
	} else {
		lastSyncRunnerIndex_ = (lastSyncRunnerIndex_ + 1) % clientMap_.size();
	}

	LOG_DEBUG("======================> SYNC REQUEST <================ : %s -> %s", runner.c_str(), tartgetId.c_str());

	if( runner.empty() )
		return;	// exceptional case..

	boost::shared_ptr<SharedPaintProtocol> prot = SystemPacketBuilder::RequestSync::make( roomId_, runner, tartgetId );
	uniCast( prot );
}

void SharedPaintRoom::uniCast( boost::shared_ptr<SharedPaintProtocol> prot ) {

	CLIENT_MAP::iterator itC = clientMap_.find( prot->header().toId() );
	if( itC != clientMap_.end() ) {
		LOG_DEBUG("======================> UNICAST <================ : %d -> %s", prot->code(),  prot->header().toId().c_str());
		itC->second->tcpSocket()->write( prot->basePtr(), prot->totalSize() );
	}
}

void SharedPaintRoom::roomCast( const std::string &fromid, boost::shared_ptr<SharedPaintProtocol> prot, bool sendMySelf ) {
	
	CLIENT_MAP::iterator itC = clientMap_.begin();
	for( ; itC != clientMap_.end(); itC++ ) {
		if( !sendMySelf && itC->first == fromid )
			continue;

		itC->second->tcpSocket()->write( prot->basePtr(), prot->totalSize() );
	}
}

void SharedPaintRoom::setSuperPeerSession( boost::shared_ptr<SharedPaintClient> client ) {

	if( client != superPeerSession_ ) { 

		if( superPeerSession_ ) { // already superpeer exists
			// nothing to do
		} else {
			LOG_DEBUG("setSuperPeerSession() ------------ CHANGED TO %s, %s", client->user()->userId().c_str(), client->tcpSocket()->peerAddress()->ip() );
			// notification
			boost::shared_ptr<SharedPaintProtocol> prot = SystemPacketBuilder::ChangeSuperPeer::make( client );
			roomCast( client->user()->userId(), prot, true );

			superPeerSession_ = client;
		}
	}
}

boost::shared_ptr<SharedPaintClient> SharedPaintRoom::currentSuperPeerSession() {
	return superPeerSession_;
}
