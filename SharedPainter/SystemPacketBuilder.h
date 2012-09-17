#pragma once

#include "PaintUser.h"
#include "CommonPacketBuilder.h"

namespace SystemPacketBuilder
{
	class ChangeSuperPeer{
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

	class CJoinerServerUser
	{
	public:
		static std::string make( boost::shared_ptr<CPaintUser> user )
		{
			try
			{
				std::string body = user->serialize();

				return CommonPacketBuilder::makePacket( CODE_SYSTEM_JOIN_SERVER, body );
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

	class CJoinerSuperPeerUser
	{
	public:
		static std::string make( boost::shared_ptr<CPaintUser> user )
		{
			try
			{
				std::string body = user->serialize();

				return CommonPacketBuilder::makePacket( CODE_SYSTEM_JOIN_SUPERPEER, body );
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
			USER_LIST &list, 
			std::string & superPeerId )
		{
			int pos = 0;
			try
			{
				boost::uint16_t count = 0;
				pos += CPacketBufferUtil::readString8( body, pos, channel );
				pos += CPacketBufferUtil::readInt16( body, pos, count, false );

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
};
