#pragma once
#include <boost/shared_ptr.hpp>

#include "PacketCodeDefine.h"
#include "PaintItem.h"
#include "PacketBuffer.h"
#include "NetPacketData.h"

//---------------------------------------------
// packet format
//---------------------------------------------
//
// | 1byte magic | 2byte code | 4byte bodylen ||| (2byte paint type ) | body string ... |
// <----------------- HEADER ------------------> <----------------- BODY ---------------->
//

class CPacketData
{
public:
	boost::int16_t code;
	std::string body;
};


class CPacketSlicer
{
public:
	static const int HeaderSize = 7;

	enum ParsingState {
		STATE_INIT,
		STATE_HEADER_MAGIC,
		STATE_HEADER_CODE,
		STATE_HEADER_BLEN,
		STATE_BODY
	};

	CPacketSlicer( void ) { init(); }

	~CPacketSlicer(void);

	void init( void )
	{
		state_ = STATE_HEADER_MAGIC;
		buffer_.clear();
		parsedItems_.clear();
	}

	const char * buffer_str( void )
	{
		return (char *)buffer_.currentPtr();
	}

	size_t buffer_size( void )
	{
		return buffer_.totalSize();
	}

	void addBuffer( const std::string & buffer )
	{
		buffer_.write( buffer.c_str(), buffer.size() );
	}

	void addBuffer( const char * buffer, size_t len )
	{
		std::string temp( buffer, len );
		addBuffer( temp );
	}

	bool parse( void )
	{
		parsedItems_.clear();
		return doParse();
	}

	size_t parsedItemCount( void )
	{
		return parsedItems_.size();
	}

	const boost::shared_ptr<CPacketData> parsedItem( size_t index )
	{
		if( index >= parsedItems_.size() )
			return boost::shared_ptr<CPacketData>();

		return parsedItems_[index];
	}

private:
	
	bool doParse( void )
	{
		try{
			switch( state_ )
			{
			case STATE_INIT:
				init();
				state_ = STATE_HEADER_MAGIC;
			case STATE_HEADER_MAGIC:
				{
					if( buffer_.remainingSize() < 1 )
						return parsedItems_.size() > 0 ? true : false;

					boost::int8_t magic = 0x0;
					buffer_.readInt8( magic );

					if( (boost::uint8_t)magic != NET_MAGIC_CODE )
					{
						init();
						return false;
					}
				}
				state_ = STATE_HEADER_CODE;
			case STATE_HEADER_CODE:
				if( buffer_.remainingSize() < 2 )
					return parsedItems_.size() > 0 ? true : false;

				buffer_.readInt16( currCode_ );
				if( currCode_ < 0 || currCode_ >= CODE_MAX )
				{
					init();
					return false;
				}
				state_ = STATE_HEADER_BLEN;
			case STATE_HEADER_BLEN:
				if( buffer_.remainingSize() < 4 )
					return parsedItems_.size() > 0 ? true : false;
				buffer_.readInt32( currBodyLen_ );

				if( currBodyLen_ > 0x1312D00 )	// 20MB
				{
					init();
					return false;
				}
				state_ = STATE_BODY;
			case STATE_BODY:
				if( buffer_.remainingSize() < (size_t)currBodyLen_ )
				{
					return parsedItems_.size() > 0 ? true : false;
				}
				
				boost::shared_ptr<CPacketData> data = boost::shared_ptr<CPacketData>(new CPacketData);
				data->code = currCode_;
				data->body.assign( (const char *)buffer_.basePtr() + HeaderSize, currBodyLen_ );

				parsedItems_.push_back( data );

				// read buffer init..
				buffer_.erase( 0, HeaderSize + currBodyLen_ );
				buffer_.setReadPos( 0 );

				state_ = STATE_HEADER_MAGIC;
				return doParse();
			}
		} catch(CPacketException &e) {
			(void)e;
			// nothing to do
		}

		return parsedItems_.size() > 0 ? true : false;
	}

private:
	CPacketBuffer buffer_;
	std::vector< boost::shared_ptr<CPacketData> > parsedItems_;

	ParsingState state_;
	boost::int16_t currCode_;
	boost::int32_t currBodyLen_;
};
