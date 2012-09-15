#include "Coconut.h"
#include "SharedPaintRoom.h"
#include "SharedPaintProtocol.h"
#include "SystemPacketBuilder.h"
#include "SharedPaintClient.h"

void SharedPaintRoom::addJoiner( boost::shared_ptr<SharedPaintClient> joiner )
{
	CLIENT_MAP::iterator itC = clientMap_.find( joiner->user()->userId() );
	if( itC == clientMap_.end() ) {
		// new user
		clientMap_.insert( CLIENT_MAP::value_type( joiner->user()->userId(), joiner ) );
	} else { 
		// already joined user. duplicate!
		itC->second->setInvalidSessionFlag();
		itC->second->tcpSocket()->close();
		itC->second = joiner;	// overwrite
	}
}


void SharedPaintRoom::removeJoiner( const std::string &userid ) {

	CLIENT_MAP::iterator itC = clientMap_.find( userid );
	if( itC != clientMap_.end() ) {
		clientMap_.erase( itC );
	}
}

std::string SharedPaintRoom::generateJoinerInfoPacket( void ) {

	int pos = 0;
	std::string body;

	pos += PacketBufferUtil::writeInt16( body, pos, clientMap_.size(), false );

	CLIENT_MAP::iterator itC = clientMap_.begin();
	for( ; itC != clientMap_.end(); itC++ ) {
			
		pos += PacketBufferUtil::writeString8( body, pos, itC->second->user()->userId() );
	}

	return body;
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

	if( runner.empty() )
		return;	// exceptional case..

	boost::shared_ptr<SharedPaintProtocol> prot = SystemPacketBuilder::RequestSync::make( roomId_, runner, tartgetId );

	uniCast( prot );
}

void SharedPaintRoom::uniCast( boost::shared_ptr<SharedPaintProtocol> prot ) {

	CLIENT_MAP::iterator itC = clientMap_.find( prot->header().toId() );
	if( itC != clientMap_.end() ) {
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

