#pragma once

#include "CommonPacketBuilder.h"

namespace UdpPacketBuilder
{
	class CServerInfo
	{
	public:
		static std::string make( const std::string &broadCastChannel, const std::string &addr, int port )
		{
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, broadCastChannel );
				pos += CPacketBufferUtil::writeString8( body, pos, addr );
				pos += CPacketBufferUtil::writeInt16( body, pos, port, true );

				return CommonPacketBuilder::makePacket( CODE_UDP_SERVER_INFO, body );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, std::string &broadCastChannel, std::string &addr, int &port )
		{
			int pos = 0;
			try
			{
				boost::uint16_t temp_port;
				pos += CPacketBufferUtil::readString8( body, pos, broadCastChannel );
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
