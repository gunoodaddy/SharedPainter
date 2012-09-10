#pragma once

#include "PaintItem.h"
#include "SharedPaintTask.h"

class CSharedPaintManager;
class CSharedPaintCommandManager;

class CSharedPaintCommand
{
public:
	CSharedPaintCommand( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) : manager_(manager), item_(item){ }
	virtual ~CSharedPaintCommand( void ) { }

private:
	virtual bool execute( void ) = 0;
	virtual bool undo( void ) = 0;

	void setManager( CSharedPaintCommandManager * manager ) { cmdManager_ = manager; }
	boost::shared_ptr<CPaintItem> item( void ) { return item_; }

protected:
	friend class CSharedPaintCommandManager;
	CSharedPaintManager *manager_;
	CSharedPaintCommandManager *cmdManager_;
	boost::shared_ptr<CPaintItem> item_;
};


class CAddItemCommand : public CSharedPaintCommand
{
public:
	CAddItemCommand( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) : CSharedPaintCommand(manager, item) { }
	~CAddItemCommand( void ) 
	{
		qDebug() << "~CAddItemCommand";
	}

	virtual bool execute( void );
	virtual bool undo( void );
};

class CRemoveItemCommand : public CSharedPaintCommand
{
public:
	CRemoveItemCommand( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) : CSharedPaintCommand(manager, item) { }
	~CRemoveItemCommand( void ) 
	{
		qDebug() << "~CRemoveItemCommand";
	}

	virtual bool execute( void );
	virtual bool undo( void );
};


class CUpdateItemCommand : public CSharedPaintCommand
{
public:
	CUpdateItemCommand( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) 
		: CSharedPaintCommand(manager, item)
		, prevData_(item->prevData()), data_(item->data()) { }
	~CUpdateItemCommand( void ) 
	{
		qDebug() << "~CUpdateItemCommand";
	}

	virtual bool execute( void );
	virtual bool undo( void );

private:
	struct SPaintData prevData_;
	struct SPaintData data_;
};


class CMoveItemCommand : public CSharedPaintCommand
{
public:
	CMoveItemCommand( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) 
		: CSharedPaintCommand(manager, item)
		, prevX_(item_->prevData().posX), prevY_(item_->prevData().posY)
		, posX_(item->posX()), posY_(item->posY()) { }
	~CMoveItemCommand( void ) 
	{
		qDebug() << "~CMoveItemCommand";
	}

	virtual bool execute( void );
	virtual bool undo( void );

private:
	double prevX_;
	double prevY_;
	double posX_;
	double posY_;
};
