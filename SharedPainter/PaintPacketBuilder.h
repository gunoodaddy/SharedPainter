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
#include "PaintItem.h"
#include "PaintItemFactory.h"

namespace PaintPacketBuilder
{
	class CSetBackgroundGridLine
	{
	public:
		static std::string make( int size, const std::string *target = NULL )
		{		
			std::string data;
			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::writeInt16( data, pos, size, true );

				return CommonPacketBuilder::makePacket( CODE_PAINT_SET_BG_GRID_LINE, data, NULL, target );
			}catch(...)
			{
			}

			return "";
		}

		static bool parse( const std::string &body, int &size )
		{		
			int pos = 0;
			try
			{
				boost::uint16_t s;
				pos += CPacketBufferUtil::readInt16( body, pos, s, true );

				size = s;
				return true;
			}catch(...)
			{
			}
			return false;
		}
	};


	class CSetBackgroundColor
	{
	public:
		static std::string make( int r, int g, int b, int a, const std::string *target = NULL )
		{		
			std::string data;
			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::writeInt16( data, pos, r, true );
				pos += CPacketBufferUtil::writeInt16( data, pos, g, true );
				pos += CPacketBufferUtil::writeInt16( data, pos, b, true );
				pos += CPacketBufferUtil::writeInt16( data, pos, a, true );

				return CommonPacketBuilder::makePacket( CODE_PAINT_SET_BG_COLOR, data, NULL, target );
			}catch(...)
			{
			}

			return "";
		}

		static bool parse( const std::string &body, int &r, int &g, int &b, int &a )
		{		
			int pos = 0;
			try
			{
				boost::uint16_t rr, gg, bb, aa;
				pos += CPacketBufferUtil::readInt16( body, pos, rr, true );
				pos += CPacketBufferUtil::readInt16( body, pos, gg, true );
				pos += CPacketBufferUtil::readInt16( body, pos, bb, true );
				pos += CPacketBufferUtil::readInt16( body, pos, aa, true );

				r = rr;
				g = gg;
				b = bb;
				a = aa;
				return true;
			}catch(...)
			{
			}
			return false;
		}
	};

	class CSetBackgroundImage
	{
	public:
		static std::string make( boost::shared_ptr<CBackgroundImageItem> item, const std::string *target = NULL )
		{		
			std::string data = item->serialize();

			try
			{
				return CommonPacketBuilder::makePacket( CODE_PAINT_SET_BG_IMAGE, data, NULL, target );
			}catch(...)
			{

			}

			return "";
		}

		static boost::shared_ptr<CBackgroundImageItem> parse( const std::string &body )
		{		
			boost::shared_ptr< CBackgroundImageItem > item;

			try
			{
				item = boost::shared_ptr< CBackgroundImageItem >( new CBackgroundImageItem );
				if( !item->deserialize( body ) )
				{
					return boost::shared_ptr<CBackgroundImageItem>();
				}
			}catch(...)
			{
				return boost::shared_ptr<CBackgroundImageItem>();
			}

			return item;
		}
	};

	class CClearBackground
	{
	public:
		static std::string make( void )
		{		
			try
			{
				std::string body;
				return CommonPacketBuilder::makePacket( CODE_PAINT_CLEAR_BG, body );
			}catch(...)
			{

			}

			return "";
		}

		static bool parse( const std::string &body )
		{		
			try
			{
				// NOTHING BODY
				return true;
			}catch(...)
			{
				return false;
			}
			return true;
		}
	};

	class CCreateItem
	{
	public:
		static std::string make( boost::shared_ptr<CPaintItem> item, const std::string *target = NULL )
		{		
			std::string body;
			std::string data = item->serialize();

			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::writeInt16( body, pos, item->type(), true );
				body += data;

				return CommonPacketBuilder::makePacket( CODE_PAINT_CREATE_ITEM, body, NULL, target );
			}catch(...)
			{
				
			}

			return "";
		}

		static boost::shared_ptr<CPaintItem> parse( const std::string &body )
		{		
			boost::shared_ptr< CPaintItem > item;

			try
			{
				int pos = 0;
				boost::uint16_t temptype;
				pos += CPacketBufferUtil::readInt16( body, pos, temptype, true );

				PaintItemType type = (PaintItemType)temptype;

				item = CPaintItemFactory::createItem( type );

				std::string itemData( (const char *)body.c_str() + pos, body.size() - pos );
				if( !item->deserialize( itemData ) )
				{
					return boost::shared_ptr<CPaintItem>();
				}
			}catch(...)
			{
				return boost::shared_ptr<CPaintItem>();
			}

			return item;
		}
	};

	class CClearScreen
	{
	public:
		static std::string make( void )
		{		
			try
			{
				std::string body;
				return CommonPacketBuilder::makePacket( CODE_PAINT_CLEAR_SCREEN, body );
			}catch(...)
			{

			}

			return "";
		}

		static bool parse( const std::string &body )
		{		
			try
			{
				// NOTHING BODY
				return true;
			}catch(...)
			{
				return false;
			}
			return true;
		}
	};
};
