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

#include "CommonPacketBuilder.h"

namespace ScreenSharePacketBuilder
{
	class CChangeShowStream
	{
	public:
		static std::string make( const std::string &from, bool senderFlag, bool startFlag )
		{
			int pos = 0;
			try
			{
				std::string body = "";
				pos += CPacketBufferUtil::writeInt8( body, pos, senderFlag ? 1 : 0 );
				pos += CPacketBufferUtil::writeInt8( body, pos, startFlag ? 1 : 0 );
				return CommonPacketBuilder::makePacket( CODE_SCREENSHARE_CHANGE_SHOW_STREAM, body, &from );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, bool &senderFlag, bool &startFlag )
		{
			try
			{
				int pos = 0;
				boost::uint8_t temp;
				pos += CPacketBufferUtil::readInt8( body, pos, temp );
				senderFlag = temp == 1 ? true : false;
				pos += CPacketBufferUtil::readInt8( body, pos, temp );
				startFlag = temp == 1 ? true : false;
				return true;

			}catch(...)
			{
			}
			return false;
		}
	};

	class CResShowStream
	{
	public:
		static std::string make( const std::string &from, bool accept, int listenPort )
		{
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeInt8( body, pos, accept ? 1 : 0 );
				pos += CPacketBufferUtil::writeInt16( body, pos, listenPort, true );

				return CommonPacketBuilder::makePacket( CODE_SCREENSHARE_RES_SHOW_STREAM, body, &from );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, bool &accept, int &listenPort )
		{
			try
			{
				int pos = 0;
				boost::uint16_t temp_port;
				boost::uint8_t temp;
				pos += CPacketBufferUtil::readInt8( body, pos, temp );
				pos += CPacketBufferUtil::readInt16( body, pos, temp_port, true );
				accept = (temp == 1) ? true : false;
				listenPort = temp_port;
				return true;

			}catch(...)
			{
			}
			return false;
		}
	};

	class CChangeRecordStatus
	{
	public:
		static std::string make( const std::string &from, bool status )
		{
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeInt8( body, pos, status ? 1 : 0 );

				return CommonPacketBuilder::makePacket( CODE_SCREENSHARE_CHANGE_RECORD_STATUS, body, &from );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, bool &status )
		{
			int pos = 0;
			try
			{
				boost::uint8_t temp;
				pos += CPacketBufferUtil::readInt8( body, pos, temp );
				status = (temp == 1) ? true : false;
				return true;

			}catch(...)
			{
			}
			return false;
		}
	};
};