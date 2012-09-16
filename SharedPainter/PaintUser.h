#pragma once

#include "PacketBuffer.h"

struct SPaintUserInfoData
{
	SPaintUserInfoData() : listenTcpPort(0), superPeerCandidate(true) { }

	std::string channel;
	std::string userId;
	boost::uint16_t listenTcpPort;
	std::string ipAddr;
	bool superPeerCandidate;
};

class CPaintUser;

typedef std::vector<boost::shared_ptr<CPaintUser> > USER_LIST;

class CPaintUser
{
public:
	CPaintUser( void ) { }
	~CPaintUser( void ) { }

	void setSessionId( int sessionId ) { sessionId_ = sessionId; }
	int sessionId( void ) { return sessionId_; }
	void setData( const struct SPaintUserInfoData &info ) { data_ = info; }
	void setChannel( const std::string & channel ) { data_.channel = channel; }
	void setListenTcpPort( boost::uint16_t port ) { data_.listenTcpPort = port; }
    void setSuperPeerCandidate( bool enable = true ) { data_.superPeerCandidate = enable; }

	bool isSuperPeerCandidate( void ) { return data_.superPeerCandidate; }   
	const std::string &ipAddress( void ) { return data_.ipAddr; }
	const std::string &channel( void ) { return data_.channel; }
	const std::string &userId( void ) { return data_.userId; }
	boost::uint16_t listenTcpPort( void ) { return data_.listenTcpPort; }

	std::string serialize( void ) {
		std::string body;
		int pos = 0;
		pos += CPacketBufferUtil::writeString8( body, pos, data_.channel );
		pos += CPacketBufferUtil::writeString8( body, pos, data_.userId );
		pos += CPacketBufferUtil::writeString8( body, pos, data_.ipAddr );
		pos += CPacketBufferUtil::writeInt16( body, pos, data_.listenTcpPort, true );
		pos += CPacketBufferUtil::writeInt8( body, pos, data_.superPeerCandidate ? 1 : 0 );
		return body;
	}        

	bool deserialize( const std::string & data, int *readPos = NULL ) {
		try  
		{    
			boost::uint8_t f;                                                                                                        
			int pos = readPos ? *readPos : 0;

			pos += CPacketBufferUtil::readString8( data, pos, data_.channel );
			pos += CPacketBufferUtil::readString8( data, pos, data_.userId );
			pos += CPacketBufferUtil::readString8( data, pos, data_.ipAddr );
			pos += CPacketBufferUtil::readInt16( data, pos, data_.listenTcpPort, true );
			pos += CPacketBufferUtil::readInt8( data, pos, f );
			data_.superPeerCandidate = (f == 1 ? true : false);

			if( readPos ) *readPos = pos;
		} catch(...)
		{    
			return false;
		}    
		return true;
	}    
private:
	int sessionId_;
	SPaintUserInfoData data_;
};