#pragma once

#include "PacketBuffer.h"

struct SPaintUserInfoData
{
	SPaintUserInfoData() : listenTcpPort(0), superPeerCandidate(false), syncComplete(false) { }
	std::string roomId;
	std::string userId;
	std::string nickName;
	std::string viewIp;
	std::string localIp;
	boost::uint16_t listenTcpPort;
	bool superPeerCandidate;
	bool syncComplete;
};

class CPaintUser
{
public:
	CPaintUser( void ) : sessionId_(0) { }
	~CPaintUser( void ) { }

	void setSessionId( int sessionId ) { sessionId_ = sessionId; }
	int sessionId( void ) { return sessionId_; }
	void setData( const struct SPaintUserInfoData &info ) { data_ = info; }
	void setNickName( const std::string & nick ) { data_.nickName = nick; }
	void setSuperPeerCandidate( bool enable = true ) { data_.superPeerCandidate = enable; }
	void setViewIPAddress( const std::string &ip ) { data_.viewIp = ip; }
	void setLocalIPAddress( const std::string &ip ) { data_.localIp = ip; }
	void setSyncComplete( bool flag = true ) { data_.syncComplete = flag; }

	bool isSuperPeerCandidate( void ) { return data_.superPeerCandidate; }
	const std::string &localIPAddress( void ) { return data_.localIp; }
	const std::string &viewIPAddress( void ) { return data_.viewIp; }

	const std::string &roomId( void ) { return data_.roomId; }
	const std::string &userId( void ) { return data_.userId; }
	const std::string &nickName( void ) { return data_.nickName; }
	boost::uint16_t listenTcpPort( void ) { return data_.listenTcpPort; }

	std::string serialize( void ) {
		std::string body;
		int pos = 0;
		pos += PacketBufferUtil::writeString8( body, pos, data_.roomId );
		pos += PacketBufferUtil::writeString8( body, pos, data_.userId );
		pos += PacketBufferUtil::writeString8( body, pos, data_.nickName );
		pos += PacketBufferUtil::writeString8( body, pos, data_.viewIp );
		pos += PacketBufferUtil::writeString8( body, pos, data_.localIp );
		pos += PacketBufferUtil::writeInt16( body, pos, data_.listenTcpPort, true );
		pos += PacketBufferUtil::writeInt8( body, pos, data_.superPeerCandidate ? 1 : 0 );
		return body;
	}

	bool deserialize( const std::string & data, int *readPos = NULL ) {
		try
		{
			boost::uint8_t f;
			int pos = readPos ? *readPos : 0;

			pos += PacketBufferUtil::readString8( data, pos, data_.roomId );
			pos += PacketBufferUtil::readString8( data, pos, data_.userId );
			pos += PacketBufferUtil::readString8( data, pos, data_.nickName );
			pos += PacketBufferUtil::readString8( data, pos, data_.viewIp );
			pos += PacketBufferUtil::readString8( data, pos, data_.localIp );
			pos += PacketBufferUtil::readInt16( data, pos, data_.listenTcpPort, true );
			pos += PacketBufferUtil::readInt8( data, pos, f );
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
