#pragma once

#include "PaintUser.h"
#include "CommonPacketBuilder.h"

namespace SystemPacketBuilder
{
	class CJoinerUser
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
				struct SPaintUserInfoData userInfo;
				pos += CPacketBufferUtil::readString8( body, pos, userInfo.userId );

				boost::shared_ptr<CPaintUser> user = boost::shared_ptr<CPaintUser>(new CPaintUser);
				user->setData( userInfo );
				return user;

			}catch(...)
			{
			}
			return boost::shared_ptr<CPaintUser>();
		}
	};

	class CLeftUser
	{
	public:
		static std::string make( const std::string &userId )
		{
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, userId );

				return CommonPacketBuilder::makePacket( CODE_SYSTEM_LEFT, body );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, std::string &userId )
		{
			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::readString8( body, pos, userId );
				return true;

			}catch(...)
			{
			}
			return false;
		}
	};
};
