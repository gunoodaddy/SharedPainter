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

#include "PaintItem.h"

class CSharedPaintTask;
class CSharedPaintManager;
class CSharedPaintCommandManager;

typedef std::vector< boost::shared_ptr<CSharedPaintTask> > TASK_LIST;

enum TaskType {
	Task_AddItem = 0,
	Task_UpdateItem,
	Task_RemoveItem,
	Task_MoveItem,
};

struct STaskData
{
	int itemId;
	std::string owner;
};

class CSharedPaintTask : public boost::enable_shared_from_this<CSharedPaintTask> 
{
public:
	CSharedPaintTask( void ) : spMngr_(NULL) { }
	CSharedPaintTask( const std::string &owner, int itemId ) : spMngr_(NULL)
	{
		 data_.owner = owner;
		 data_.itemId = itemId; 
	}
	virtual ~CSharedPaintTask( void ) { }

public:
	const std::string &owner( void ) { return data_.owner; }
	int itemId( void ) { return data_.itemId; }
	void setCommandManager( CSharedPaintCommandManager * cmdMngr ) { cmdMngr_ = cmdMngr; }
	void setSharedPaintManager( CSharedPaintManager * spMngr ) { spMngr_ = spMngr; }
	void setSendData( bool sendData ) { sendData_ = sendData; }
	void sendPacket( void );

	static bool deserializeBasicData( const std::string & data, struct STaskData &res, int *readPos = NULL ) 
	{
		try
		{
			boost::uint32_t t;
			int pos = readPos ? *readPos : 0;

			pos += CPacketBufferUtil::readString8( data, pos, res.owner );
			pos += CPacketBufferUtil::readInt32( data, pos, t, true );
			res.itemId = t;
			if( readPos )
				*readPos = pos;
		} catch(...)
		{
			return false;
		}
		return true;
	}

	static std::string serializeBasicData( const struct STaskData &data, int *writePos = NULL )
	{
		int pos = writePos ? *writePos : 0;
		std::string buf;

		pos += CPacketBufferUtil::writeString8( buf, pos, data.owner );
		pos += CPacketBufferUtil::writeInt32( buf, pos, data.itemId, true );
		if( writePos )
			*writePos = pos;
		return buf;
	}

	virtual std::string serialize( int *writePos = NULL )
	{
		return serializeBasicData( data_, writePos );	
	}

	virtual bool deserialize( const std::string & data, int *readPos = NULL )
	{
		return deserializeBasicData( data, data_, readPos );
	}

	virtual TaskType type( void ) = 0;
	virtual bool execute( void ) = 0;
	virtual void rollback( void ) = 0;
protected:
	friend class CSharedPaintCommandManager;
	CSharedPaintCommandManager *cmdMngr_;
	CSharedPaintManager *spMngr_;
	struct STaskData data_;
	bool sendData_;
};

//------------------------------------------------------------------------------------------------------

class CAddItemTask : public CSharedPaintTask
{
public:
	CAddItemTask( void ) : CSharedPaintTask() { }
	CAddItemTask( const std::string &owner, int itemId ) : CSharedPaintTask(owner, itemId) { }
	virtual ~CAddItemTask( void )
	{
		qDebug() << "~CAddItemTask";
	}

	virtual TaskType type( void ) { return Task_AddItem; }
	virtual bool execute( void );
	virtual void rollback( void );
};


class CRemoveItemTask : public CSharedPaintTask
{
public:
	CRemoveItemTask( void ) : CSharedPaintTask() { }
	CRemoveItemTask( const std::string &owner, int itemId ) : CSharedPaintTask(owner, itemId) { }
	virtual  ~CRemoveItemTask( void )
	{
		qDebug() << "~CRemoveItemTask";
	}

	virtual TaskType type( void ) { return Task_RemoveItem; }
	virtual bool execute( void );
	virtual void rollback( void );
};


class CUpdateItemTask : public CSharedPaintTask
{
public:
	CUpdateItemTask( void ) : CSharedPaintTask() { }
	CUpdateItemTask( const std::string &owner, int itemId, const struct SPaintData &prevData, const struct SPaintData &data ) : CSharedPaintTask(owner, itemId)
	  , prevPaintData_(prevData), paintData_(data) { }
	virtual  ~CUpdateItemTask( void )
	{
		qDebug() << "~CUpdateItemTask";
	}

	virtual TaskType type( void ) { return Task_UpdateItem; }
	virtual bool execute( void );
	virtual void rollback( void );

	virtual std::string serialize( int *writePos = NULL )
	{
		std::string data;

		data = CSharedPaintTask::serialize();
		data += CPaintItem::serializeBasicData( prevPaintData_ );
		data += CPaintItem::serializeBasicData( paintData_ );
		return data;
	}

	virtual bool deserialize( const std::string & data, int *readPos = NULL )
	{
		try
		{
			int pos = readPos ? *readPos : 0;
			if( ! CSharedPaintTask::deserialize( data, &pos ) )
				return false;

			CPaintItem::deserializeBasicData( data, prevPaintData_, &pos );
			CPaintItem::deserializeBasicData( data, paintData_, &pos );
		} catch(CPacketException &e) {
			(void)e;
			// nothing to do
			return false;
		}
		return true;
	}

private:
	struct SPaintData prevPaintData_;
	struct SPaintData paintData_;
};


class CMoveItemTask : public CSharedPaintTask
{
public:
	CMoveItemTask( void ) : CSharedPaintTask() { }
	CMoveItemTask( const std::string &owner, int itemId, double prevPosX, double prevPosY, double posX, double posY ) : CSharedPaintTask(owner, itemId)
		, prevPosX_(prevPosX), prevPosY_(prevPosY), posX_(posX), posY_(posY) { }
	virtual  ~CMoveItemTask( void )
	{
		qDebug() << "~CMoveItemCommand";
	}

	virtual TaskType type( void ) { return Task_MoveItem; }
	virtual bool execute( void );
	virtual void rollback( void );

	virtual std::string serialize( int *writePos = NULL )
	{
		int pos = 0;
		std::string data;

		data = CSharedPaintTask::serialize( &pos );
		pos += CPacketBufferUtil::writeDouble( data, pos, prevPosX_, true );
		pos += CPacketBufferUtil::writeDouble( data, pos, prevPosY_, true );
		pos += CPacketBufferUtil::writeDouble( data, pos, posX_, true );
		pos += CPacketBufferUtil::writeDouble( data, pos, posY_, true );
		return data;
	}

	virtual bool deserialize( const std::string & data, int *readPos = NULL )
	{
		try
		{
			int pos = 0;
			if( ! CSharedPaintTask::deserialize( data, &pos ) )
				return false;
			pos += CPacketBufferUtil::readDouble( data, pos, prevPosX_, true );
			pos += CPacketBufferUtil::readDouble( data, pos, prevPosY_, true );
			pos += CPacketBufferUtil::readDouble( data, pos, posX_, true );
			pos += CPacketBufferUtil::readDouble( data, pos, posY_, true );
			return true;
		}catch(...)
		{
		}
		return false;
	}

private:
	double prevPosX_;
	double prevPosY_;
	double posX_;
	double posY_;
};
