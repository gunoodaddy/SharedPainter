#pragma once

#include "PacketBuffer.h"

namespace CommonPacketBuilder
{
	static std::string makePacket( boost::int16_t code, const std::string &body )
	{
		std::string buf;

		int pos = 0;
		pos += CPacketBufferUtil::writeInt8( buf, pos, NET_MAGIC_CODE );
		pos += CPacketBufferUtil::writeInt16( buf, pos, code, true );
		pos += CPacketBufferUtil::writeInt32( buf, pos, body.size(), true );
		buf += body;

		return buf;
	}
};