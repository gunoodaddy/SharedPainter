#pragma once

#include "CommonPacketBuilder.h"
#include "PaintItem.h"
#include "PaintItemFactory.h"

namespace PaintPacketBuilder
{
	class CSetBackgroundImage
	{
	public:
		static std::string make( boost::shared_ptr<CBackgroundImageItem> item )
		{		
			std::string data = item->generateData();

			int pos = 0;
			try
			{
				return CommonPacketBuilder::makePacket( CODE_PAINT_SET_BG_IMAGE, data );
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
				if( !item->loadData( body ) )
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

	class CClearBackgroundImage
	{
	public:
		static std::string make( void )
		{		
			int pos = 0;
			try
			{
				std::string body;
				return CommonPacketBuilder::makePacket( CODE_PAINT_CLEAR_BG_IMAGE, body );
			}catch(...)
			{

			}

			return "";
		}

		static bool parse( const std::string &body )
		{		
			int pos = 0;
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

	class CUpdateItem
	{
	public:
		static std::string make( boost::shared_ptr<CPaintItem> item )
		{		
			std::string body = CPaintItem::generateBasicData( item->data() );

			try
			{
				return CommonPacketBuilder::makePacket( CODE_PAINT_UPDATE_ITEM, body );
			}catch(...)
			{
				
			}

			return "";
		}

		static bool parse( const std::string &body, struct SPaintData &data )
		{		
			bool res = CPaintItem::loadBasicPaintData( body, data );
			return res;
		}
	};

	class CAddItem
	{
	public:
		static std::string make( boost::shared_ptr<CPaintItem> item )
		{		
			std::string body;
			std::string data = item->generateData();

			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::writeInt16( body, pos, item->type(), true );
				body += data;

				return CommonPacketBuilder::makePacket( CODE_PAINT_ADD_ITEM, body );
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
				boost::int16_t temptype;
				pos += CPacketBufferUtil::readInt16( body, pos, temptype, true );

				PaintItemType type = (PaintItemType)temptype;

				item = CPaintItemFactory::createItem( type );

				std::string itemData( (const char *)body.c_str() + pos, body.size() - pos );
				if( !item->loadData( itemData ) )
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

	class CMoveItem
	{
	public:
		static std::string make( const std::string &owner, int itemId, double x, double y )
		{		
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, owner  );
				pos += CPacketBufferUtil::writeInt32( body, pos, itemId, true );
				pos += CPacketBufferUtil::writeDouble( body, pos, x, true );
				pos += CPacketBufferUtil::writeDouble( body, pos, y, true );
				pos = 0;

				return CommonPacketBuilder::makePacket( CODE_PAINT_MOVE_ITEM, body );
			}catch(...)
			{
				
			}

			return "";
		}

		static bool parse( const std::string &body, std::string &owner, int &itemId, double &x, double &y )
		{		
			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::readString8( body, pos, owner );
				pos += CPacketBufferUtil::readInt32( body, pos, itemId, true );
				pos += CPacketBufferUtil::readDouble( body, pos, x, true );
				pos += CPacketBufferUtil::readDouble( body, pos, y, true );
			}catch(...)
			{
				return false;
			}
			return true;
		}
	};

	class CRemoveItem
	{
	public:
		static std::string make( const std::string &owner, int itemId )
		{		
			int pos = 0;
			try
			{
				std::string body;
				pos += CPacketBufferUtil::writeString8( body, pos, owner  );
				pos += CPacketBufferUtil::writeInt32( body, pos, itemId, true );

				return CommonPacketBuilder::makePacket( CODE_PAINT_REMOVE_ITEM, body );
			}catch(...)
			{

			}

			return "";
		}

		static bool parse( const std::string &body, std::string &owner, int &itemId )
		{		
			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::readString8( body, pos, owner );
				pos += CPacketBufferUtil::readInt32( body, pos, itemId, true );
			}catch(...)
			{
				return false;
			}
			return true;
		}
	};

	class CClearScreen
	{
	public:
		static std::string make( void )
		{		
			int pos = 0;
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
			int pos = 0;
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
