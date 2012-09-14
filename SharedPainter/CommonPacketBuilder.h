#pragma once

#include "PacketBuffer.h"


namespace CommonPacketBuilder
{
	static std::string makePacket( boost::int16_t code, const std::string &body, const std::string *fromId = NULL, const std::string *toId = NULL )
	{
		static std::string emptyString;
		std::string buf;

		int pos = 0;
		pos += CPacketBufferUtil::writeInt16( buf, pos, NET_MAGIC_CODE, true );
		pos += CPacketBufferUtil::writeInt16( buf, pos, code, true );
		pos += CPacketBufferUtil::writeString8( buf, pos, fromId ? *fromId : emptyString );
		pos += CPacketBufferUtil::writeString8( buf, pos, toId ? *toId : emptyString );
		pos += CPacketBufferUtil::writeInt32( buf, pos, body.size(), true );
		buf += body;

		return buf;
	}
};
