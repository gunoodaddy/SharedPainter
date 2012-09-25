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

namespace WindowPacketBuilder
{
	class CResizeWindowSplitter
	{
	public:
		static std::string make( const std::vector<int> &sizes, const std::string *target = NULL )
		{
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeInt16( body, pos, sizes.size(), true );
				for( size_t i = 0; i < sizes.size(); i++ ) 
				{
					pos += CPacketBufferUtil::writeInt16( body, pos, sizes[i], true );
				}

				return CommonPacketBuilder::makePacket( CODE_WINDOW_RESIZE_WND_SPLITTER, body, NULL, target );
			}catch(...)
			{
			}
			return "";
		}

		static bool parse( const std::string &body, std::vector<int> &sizes )
		{
			int pos = 0;
			try
			{
				boost::uint16_t cnt, size;
				pos += CPacketBufferUtil::readInt16( body, pos, cnt, true );
				for( size_t i = 0; i < cnt; i++ ) 
				{
					pos += CPacketBufferUtil::readInt16( body, pos, size, true );
					sizes.push_back(size);
				}
			}catch(...)
			{
				return false;
			}
			return true;
		}
	};

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
				boost::uint16_t w, h;
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
