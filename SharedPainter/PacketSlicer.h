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
// | 2byte magic | 2byte code | 4byte bodylen | from id 1byte string | to id 1byte string | (2byte paint type ) | body string ... |
// <------------------------ HEADER ------------------------------------------------------> <----------------- BODY ---------------->
//

class CPacketData
{
public:
	boost::uint16_t code;
	std::string fromId;
	std::string toId;
	std::string body;
};


class CPacketSlicer
{
public:
	enum ParsingState {
		STATE_INIT,
		STATE_HEADER_MAGIC,
		STATE_HEADER_CODE,
		STATE_HEADER_FROMID,
		STATE_HEADER_TOID,
		STATE_HEADER_BLEN,
		STATE_BODY
	};

	CPacketSlicer( void ) { init(); }

	~CPacketSlicer(void);

	void init( void )
	{
		currHeaderLen_ = 0;
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

	bool _readString8( boost::uint8_t &len, std::string &str )
	{
		if( len <= 0 )
		{
			if( buffer_.remainingSize() < 1 )
				return false;
			currHeaderLen_ += buffer_.readInt8( len );
		}
		
		if( len > 0 )
		{
			if( buffer_.remainingSize() < (size_t)len )
				return false;

			currHeaderLen_ += buffer_.read( str, len );
		}

		return true;
	}

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
					currFromIdLen_ = currToIdLen_ = 0;
					currHeaderLen_ = 0;
					if( buffer_.remainingSize() < 2 )
						return parsedItems_.size() > 0 ? true : false;

					boost::uint16_t magic = 0x0;
					currHeaderLen_ += buffer_.readInt16( magic );

					if( (boost::uint16_t)magic != NET_MAGIC_CODE )
					{
						init();
						return false;
					}
				}
				state_ = STATE_HEADER_CODE;
			case STATE_HEADER_CODE:
				if( buffer_.remainingSize() < 2 )
					return parsedItems_.size() > 0 ? true : false;

				currHeaderLen_ += buffer_.readInt16( currCode_ );
				qDebug() << "============================ packet recved " << currCode_;
				if( currCode_ < 0 || currCode_ >= CODE_MAX )
				{
					init();
					return false;
				}
				state_ = STATE_HEADER_FROMID;
			case STATE_HEADER_FROMID:
				{
					if( ! _readString8( currFromIdLen_, currFromId_ ) )
						return parsedItems_.size() > 0 ? true : false;

					state_ = STATE_HEADER_TOID;
				}
			case STATE_HEADER_TOID:
				{
					if( ! _readString8( currToIdLen_, currToId_ ) )
						return parsedItems_.size() > 0 ? true : false;

					state_ = STATE_HEADER_BLEN;
				}
			case STATE_HEADER_BLEN:
				if( buffer_.remainingSize() < 4 )
					return parsedItems_.size() > 0 ? true : false;

				currHeaderLen_ += buffer_.readInt32( currBodyLen_ );

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
				data->fromId = currFromId_;
				data->toId = currToId_;
				data->body.assign( (const char *)buffer_.basePtr() + currHeaderLen_, currBodyLen_ );

				parsedItems_.push_back( data );

				// read buffer init..
				buffer_.erase( 0, currHeaderLen_ + currBodyLen_ );
				buffer_.setReadPos( 0 );
				currHeaderLen_ = 0;

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
	boost::uint8_t currFromIdLen_;
	boost::uint8_t currToIdLen_;
	std::string currFromId_;
	std::string currToId_;
	boost::uint16_t currHeaderLen_;
	boost::uint16_t currCode_;
	boost::uint32_t currBodyLen_;
};
