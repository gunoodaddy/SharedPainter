#pragma once

#include "CommonPacketBuilder.h"

namespace BroadCastPacketBuilder
{
	class CProbeServer
	{
	public:
		static std::string make( const std::string &broadCastChannel, const std::string &addr, int port  )
		{
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, broadCastChannel );
				pos += CPacketBufferUtil::writeString8( body, pos, addr );
				pos += CPacketBufferUtil::writeInt16( body, pos, port, true );

				return CommonPacketBuilder::makePacket( CODE_BROAD_PROBE_SERVER, body );
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

	class CTextMessage
	{
	public:
		static std::string make( const std::string &broadCastChannel, const std::string &myId, const std::string &message )
		{
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, broadCastChannel );
				pos += CPacketBufferUtil::writeString8( body, pos, myId );
				pos += CPacketBufferUtil::writeString8( body, pos, message );

				return CommonPacketBuilder::makePacket( CODE_BROAD_TEXT_MESSAGE, body );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, std::string &broadCastChannel, std::string &myId, std::string &message )
		{
			int pos = 0;
			try
			{
				boost::uint16_t temp_port;
				pos += CPacketBufferUtil::readString8( body, pos, broadCastChannel );
				pos += CPacketBufferUtil::readString8( body, pos, myId );
				pos += CPacketBufferUtil::readString8( body, pos, message );
			}catch(...)
			{
				return false;
			}
			return true;
		}
	};
};
