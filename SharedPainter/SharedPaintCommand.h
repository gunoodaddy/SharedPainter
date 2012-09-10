#pragma once

#include "PaintItem.h"
#include "PaintPacketBuilder.h"
#include "WindowPacketBuilder.h"

class CSharedPaintManager;
class CSharedPaintCommandManager;

class CSharedPaintCommand
{
public:
	CSharedPaintCommand(void) { }
	virtual ~CSharedPaintCommand(void) { }

private:
	virtual bool execute( bool sendData = true ) = 0;
	virtual void undo( bool sendData = true ) = 0;

	friend class CSharedPaintCommandManager;
};


class CAddItemCommand : public CSharedPaintCommand
{
public:
	CAddItemCommand( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) : manager_(manager), item_(item) { }
	~CAddItemCommand( void ) 
	{
		qDebug() << "~CAddItemCommand";
	}

	virtual bool execute( bool sendData = true );
	virtual void undo( bool sendData = true );

private:
	CSharedPaintManager *manager_;
	boost::shared_ptr<CPaintItem> item_;
};

class CRemoveItemCommand : public CSharedPaintCommand
{
public:
	CRemoveItemCommand( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) : manager_(manager), item_(item) { }
	~CRemoveItemCommand( void ) 
	{
		qDebug() << "~CRemoveItemCommand";
	}

	virtual bool execute( bool sendData = true );
	virtual void undo( bool sendData = true );

private:
	CSharedPaintManager *manager_;
	boost::shared_ptr<CPaintItem> item_;
};


class CUpdateItemCommand : public CSharedPaintCommand
{
public:
	CUpdateItemCommand( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) 
		: manager_(manager), item_(item)
		, prevData_(item->prevData()), data_(item->data()) { }
	~CUpdateItemCommand( void ) 
	{
		qDebug() << "~CUpdateItemCommand";
	}

	virtual bool execute( bool sendData = true );
	virtual void undo( bool sendData = true );

private:
	CSharedPaintManager *manager_;
	struct SPaintData prevData_;
	struct SPaintData data_;
	boost::shared_ptr<CPaintItem> item_;
};


class CMoveItemCommand : public CSharedPaintCommand
{
public:
	CMoveItemCommand( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) 
		: manager_(manager), item_(item)
		, prevX_(item_->prevData().posX), prevY_(item_->prevData().posY)
		, posX_(item->posX()), posY_(item->posY()) { }
	~CMoveItemCommand( void ) 
	{
		qDebug() << "~CMoveItemCommand";
	}

	virtual bool execute( bool sendData = true );
	virtual void undo( bool sendData = true );

private:
	CSharedPaintManager *manager_;
	boost::shared_ptr<CPaintItem> item_;
	double prevX_;
	double prevY_;
	double posX_;
	double posY_;
};