#pragma once

#include "CommonPacketBuilder.h"

namespace BroadCastPacketBuilder
{
	class CServerInfo
	{
	public:
		static std::string make( const std::string &addr, int port )
		{
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, addr );
				pos += CPacketBufferUtil::writeInt16( body, pos, port, true );

				return CommonPacketBuilder::makePacket( CODE_BROAD_SERVER_INFO, body );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, std::string &addr, int &port )
		{
			int pos = 0;
			try
			{
				boost::int16_t temp_port;
				pos += CPacketBufferUtil::readString8( body, pos, addr );
				pos += CPacketBufferUtil::readInt16( body, pos, temp_port, true );
				port = temp_port;
			}catch(...)
			{
				return false;
			}
			return true;
		}
	};
};