#pragma once

#include "CommonPacketBuilder.h"

namespace WindowPacketBuilder
{
	class CResizeMainWindow
	{
	public:
		static std::string make( int width, int height, const std::string *target = NULL )
		{
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeInt16( body, pos, width, true );
				pos += CPacketBufferUtil::writeInt16( body, pos, height, true );

				return CommonPacketBuilder::makePacket( CODE_WINDOW_RESIZE_MAIN_WND, body, NULL, target );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, int &width, int &height )
		{
			int pos = 0;
			try
			{
				boost::int16_t w, h;
				pos += CPacketBufferUtil::readInt16( body, pos, w, true );
				pos += CPacketBufferUtil::readInt16( body, pos, h, true );
				width = w;
				height = h;
			}catch(...)
			{
				return false;
			}
			return true;
		}
	};
};
