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

#include "PaintUser.h"
#include "CommonPacketBuilder.h"

namespace SystemPacketBuilder
{

	class CChatMessage
	{
	public:
		static std::string make( const std::string &id, const std::string &nick, const std::string &msg )
		{
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, id );
				pos += CPacketBufferUtil::writeString8( body, pos, nick );
				pos += CPacketBufferUtil::writeString8( body, pos, msg );

				return CommonPacketBuilder::makePacket( CODE_SYSTEM_CHAT_MESSAGE, body );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, std::string &id, std::string &nick, std::string &msg )
		{
			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::readString8( body, pos, id );
				pos += CPacketBufferUtil::readString8( body, pos, nick );
				pos += CPacketBufferUtil::readString8( body, pos, msg );
			}catch(...)
			{
				return false;
			}
			return true;
		}
	};

	class CVersionInfo {
	public:
		static std::string make( const std::string &version, const std::string &protVersion )
		{
			try
			{
				int pos = 0;
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, version );
				pos += CPacketBufferUtil::writeString8( body, pos, protVersion );

				return CommonPacketBuilder::makePacket( CODE_SYSTEM_VERSION_INFO, body );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, std::string &version, std::string &protVersion )
		{
			try
			{
				int pos = 0;
				pos += CPacketBufferUtil::readString8( body, pos, version );
				pos += CPacketBufferUtil::readString8( body, pos, protVersion );
				return true;

			}catch(...)
			{
			}
			return false;
		}
	};

	class CChangeNickName{
	public:
		static std::string make( const std::string &userid, const std::string & nickName )
		{
			try
			{
				int pos = 0;
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, userid );
				pos += CPacketBufferUtil::writeString8( body, pos, nickName );

				return CommonPacketBuilder::makePacket( CODE_SYSTEM_CHANGE_NICKNAME, body );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, std::string & userid, std::string & nickName ) {

			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::readString8( body, pos, userid );
				pos += CPacketBufferUtil::readString8( body, pos, nickName );

				return true;
			}catch(...)
			{
			}
			return false;
		}
	};

	class CChangeSuperPeer{
	public:
		static bool parse( const std::string &body, std::string & userid ) {

			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::readString8( body, pos, userid );

				return true;
			}catch(...)
			{
			}
			return false;
		}
	};

	class CJoinToServer
	{
	public:
		static std::string make( boost::shared_ptr<CPaintUser> user )
		{
			try
			{
				std::string body = user->serialize();

				return CommonPacketBuilder::makePacket( CODE_SYSTEM_JOIN_TO_SERVER, body );
			}catch(...)
			{
			}
			return "";
		}

		static boost::shared_ptr<CPaintUser> parse( const std::string &body )
		{
			try
			{
				boost::shared_ptr<CPaintUser> user(new CPaintUser);
				user->deserialize( body );			
				return user;

			}catch(...)
			{
			}
			return boost::shared_ptr<CPaintUser>();
		}
	};

	class CJoinerToSuperPeer
	{
	public:
		static std::string make( boost::shared_ptr<CPaintUser> user )
		{
			try
			{
				std::string body = user->serialize();

				return CommonPacketBuilder::makePacket( CODE_SYSTEM_JOIN_TO_SUPERPEER, body );
			}catch(...)
			{
			}
			return "";
		}

		static boost::shared_ptr<CPaintUser> parse( const std::string &body )
		{
			try
			{
				boost::shared_ptr<CPaintUser> user(new CPaintUser);
				user->deserialize( body );			
				return user;

			}catch(...)
			{
			}
			return boost::shared_ptr<CPaintUser>();
		}
	};

	class CSyncRequest
	{
	public:
		static std::string make( void )
		{
			try
			{
				return CommonPacketBuilder::makePacket( CODE_SYSTEM_SYNC_REQUEST, "" );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, std::string &channel, std::string &target )
		{
			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::readString8( body, pos, channel );
				pos += CPacketBufferUtil::readString8( body, pos, target );
				return true;

			}catch(...)
			{
			}
			return false;
		}
	};

	class CSyncStart
	{
	public:
		static std::string make( const std::string &channel, const std::string &fromId, const std::string &toId )
		{
			try
			{
				int pos = 0;
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, channel );

				return CommonPacketBuilder::makePacket( CODE_SYSTEM_SYNC_START, body, &fromId, &toId );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, std::string &channel )
		{
			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::readString8( body, pos, channel );
				return true;

			}catch(...)
			{
			}
			return false;
		}
	};

	class CSyncComplete
	{
	public:
		static std::string make( const std::string &targetId )
		{
			try
			{
				return CommonPacketBuilder::makePacket( CODE_SYSTEM_SYNC_COMPLETE, "", NULL, &targetId );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body )
		{
			try
			{
				// nothing to do
				return true;
			}catch(...)
			{
			}
			return false;
		}
	};

	class CTcpSyn
	{
	public:
		static bool parse( const std::string &body )
		{
			try
			{
				// nothing to do yet..
				return true;

			}catch(...)
			{
			}
			return false;
		}
	};

	
	class CTcpAck
	{
	public:
		static std::string make( void )
		{
			try
			{
				return CommonPacketBuilder::makePacket( CODE_SYSTEM_TCPACK, "" );

			}catch(...)
			{
			}
			return "";
		}
	};

	class CResponseJoin
	{
	public:
		static bool parse( const std::string &body, 
			std::string &channel, 
			bool &firstFlag,
			USER_LIST &list, 
			std::string & superPeerId )
		{
			int pos = 0;
			try
			{
				boost::uint8_t tempFirstFlag = 0;
				boost::uint16_t count = 0;
				pos += CPacketBufferUtil::readString8( body, pos, channel );
				pos += CPacketBufferUtil::readInt8( body, pos, tempFirstFlag );
				pos += CPacketBufferUtil::readInt16( body, pos, count, false );

				firstFlag = (tempFirstFlag == 1) ? true : false;

				for( int i = 0; i < count; i++ )
				{
					boost::shared_ptr<CPaintUser> user(new CPaintUser);
					user->deserialize( body, &pos );			
					list.push_back( user );
				}

				pos += CPacketBufferUtil::readString8( body, pos, superPeerId );
				return true;

			}catch(...)
			{
			}
			return false;
		}
	};

	class CLeftUser
	{
	public:
		static std::string make( const std::string &channel, const std::string &userId )
		{
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, channel );
				pos += CPacketBufferUtil::writeString8( body, pos, userId );

				return CommonPacketBuilder::makePacket( CODE_SYSTEM_LEFT, body );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, std::string &channel, std::string &userId )
		{
			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::readString8( body, pos, channel );
				pos += CPacketBufferUtil::readString8( body, pos, userId );
				return true;

			}catch(...)
			{
			}
			return false;
		}
	};

	class CHistoryUserList
	{
	public:
		static std::string make( USER_LIST list )
		{
			try
			{
				int pos = 0;
				std::string body;
				pos += CPacketBufferUtil::writeInt16( body, pos, (boost::uint16_t)list.size(), true );

				for( size_t i = 0; i < list.size(); i++ )
				{
					body += list[i]->serialize();
				}

				return CommonPacketBuilder::makePacket( CODE_SYSTEM_HISTORY_USER_LIST, body );
			}catch(...)
			{
			}
			return "";
		}

		static USER_LIST parse( const std::string &body )
		{
			int pos = 0;
			try
			{
				USER_LIST list;
				boost::uint16_t count = 0;
				pos += CPacketBufferUtil::readInt16( body, pos, count, true );

				for( int i = 0; i < count; i++ )
				{
					boost::shared_ptr<CPaintUser> user(new CPaintUser);
					user->deserialize( body, &pos );
					list.push_back( user );
				}
				return list;

			}catch(...)
			{
			}
			return USER_LIST();
		}
	};

};
