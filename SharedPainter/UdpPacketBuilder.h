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

namespace UdpPacketBuilder
{
	class CServerInfo
	{
	public:
		static std::string make( const std::string &paintChannel, const std::string &addr, int port )
		{
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, paintChannel );
				pos += CPacketBufferUtil::writeString8( body, pos, addr );
				pos += CPacketBufferUtil::writeInt16( body, pos, port, true );

				return CommonPacketBuilder::makePacket( CODE_UDP_SERVER_INFO, body );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, std::string &paintChannel, std::string &addr, int &port )
		{
			int pos = 0;
			try
			{
				boost::uint16_t temp_port;
				pos += CPacketBufferUtil::readString8( body, pos, paintChannel );
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
