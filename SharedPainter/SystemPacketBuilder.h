#pragma once

#include "PaintUser.h"
#include "CommonPacketBuilder.h"

namespace SystemPacketBuilder
{
	class CJoin
	{
	public:
		static std::string make( boost::shared_ptr<CPaintUser> user )
		{
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, user->userId() );

				return CommonPacketBuilder::makePacket( CODE_SYSTEM_JOIN, body );
			}catch(...)
			{
			}
			return "";
		}

		static boost::shared_ptr<CPaintUser> parse( const std::string &body )
		{
			int pos = 0;
			try
			{
				struct SUserInfoData userInfo;
				pos += CPacketBufferUtil::readString8( body, pos, userInfo.userId );

				boost::shared_ptr<CPaintUser> user = boost::shared_ptr<CPaintUser>(new CPaintUser);
				user->loadData( userInfo );
				return user;

			}catch(...)
			{
			}
			return boost::shared_ptr<CPaintUser>();
		}
	};
};
