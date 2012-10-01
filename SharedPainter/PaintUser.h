/*                                                                                                                                           
* Copyright (c) 2012, Eunhyuk Kim(gunoodaddy) 
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*   * Neither the name of Redis nor the names of its contributors may be used
*     to endorse or promote products derived from this software without
*     specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "PacketBuffer.h"

struct SPaintUserInfoData
{
	SPaintUserInfoData() : listenTcpPort(0), superPeerCandidate(false) { }

	std::string channel;
	std::string userId;
	std::string nickName;
	boost::uint16_t listenTcpPort;
	std::string viewIp;
	std::string localIp;
	bool superPeerCandidate;
};

class CPaintUser;

typedef std::vector<boost::shared_ptr<CPaintUser> > USER_LIST;

class CPaintUser
{
public:
	CPaintUser( bool myself ) : mySelfFlag_(myself) { }
	CPaintUser( void ) : mySelfFlag_(false) { }
	~CPaintUser( void ) { }

	// session id is only used for "always p2p mode"
	void setSessionId( int sessionId ) { sessionId_ = sessionId; }
	int sessionId( void ) { return sessionId_; }

	void setData( const struct SPaintUserInfoData &info ) { data_ = info; }
	void setNickName( const std::string & nick ) { data_.nickName = nick; }
	void setChannel( const std::string & channel ) { data_.channel = channel; }
	void setListenTcpPort( boost::uint16_t port ) { data_.listenTcpPort = port; }
    void setSuperPeerCandidate( bool enable = true ) { data_.superPeerCandidate = enable; }
	void setLocalIPAddress( const std::string &ip ) { data_.localIp = ip; }
	void setViewIPAddress( const std::string &ip ) { data_.viewIp = ip; }

	bool isMyself( void ) { return mySelfFlag_; }
	const struct SPaintUserInfoData &data( void ) { return data_; }
	bool isSuperPeerCandidate( void ) { return data_.superPeerCandidate; }   
	const std::string &localIPAddress( void ) { return data_.localIp; }
	const std::string &viewIPAddress( void ) { return data_.viewIp; }
	const std::string &channel( void ) { return data_.channel; }
	const std::string &userId( void ) { return data_.userId; }
	const std::string &nickName( void ) { return data_.nickName; }
	boost::uint16_t listenTcpPort( void ) { return data_.listenTcpPort; }

	std::string serialize( void ) {
		std::string body;
		int pos = 0;
		pos += CPacketBufferUtil::writeString8( body, pos, data_.channel );
		pos += CPacketBufferUtil::writeString8( body, pos, data_.userId );
		pos += CPacketBufferUtil::writeString8( body, pos, data_.nickName );
		pos += CPacketBufferUtil::writeString8( body, pos, data_.viewIp );
		pos += CPacketBufferUtil::writeString8( body, pos, data_.localIp );
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
			pos += CPacketBufferUtil::readString8( data, pos, data_.nickName );
			pos += CPacketBufferUtil::readString8( data, pos, data_.viewIp );
			pos += CPacketBufferUtil::readString8( data, pos, data_.localIp );
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
	bool mySelfFlag_;
	int sessionId_;
	SPaintUserInfoData data_;
};
