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
	virtual bool execute( void ) = 0;
	virtual void undo( void ) = 0;

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

	virtual bool execute( void );
	virtual void undo( void );

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

	virtual bool execute( void );
	virtual void undo( void );

private:
	CSharedPaintManager *manager_;
	boost::shared_ptr<CPaintItem> item_;
};


class CUpdateItemCommand : public CSharedPaintCommand
{
public:
	CUpdateItemCommand( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) : manager_(manager), item_(item) { }
	~CUpdateItemCommand( void ) 
	{
		qDebug() << "~CUpdateItemCommand";
	}

	virtual bool execute( void );
	virtual void undo( void );

private:
	CSharedPaintManager *manager_;
	struct SPaintData prevData_;
	boost::shared_ptr<CPaintItem> item_;
};


class CMoveItemCommand : public CSharedPaintCommand
{
public:
	CMoveItemCommand( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) : manager_(manager), item_(item), prevX_(0.f), prevY_(0.f) { }
	~CMoveItemCommand( void ) 
	{
		qDebug() << "~CMoveItemCommand";
	}

	virtual bool execute( void );
	virtual void undo( void );

private:
	CSharedPaintManager *manager_;
	boost::shared_ptr<CPaintItem> item_;
	double prevX_;
	double prevY_;
};