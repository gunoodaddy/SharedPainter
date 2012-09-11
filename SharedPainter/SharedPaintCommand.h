#pragma once

#include "PaintItem.h"
#include "SharedPaintTask.h"

class CSharedPaintManager;
class CSharedPaintCommandManager;

class CSharedPaintCommand
{
public:
	CSharedPaintCommand( CSharedPaintManager *spMngr, boost::shared_ptr<CPaintItem> item ) 
		: cmdMngr_(NULL), spMngr_(spMngr), item_(item){ }
	virtual ~CSharedPaintCommand( void ) { }

private:
	virtual bool execute( void ) = 0;
	virtual bool undo( void ) = 0;

	boost::shared_ptr<CPaintItem> item( void ) { return item_; }
	void setCommandManager( CSharedPaintCommandManager *cmdManager ) { cmdMngr_ = cmdManager; }

protected:
	friend class CSharedPaintCommandManager;
	CSharedPaintCommandManager *cmdMngr_;
	CSharedPaintManager *spMngr_;
	boost::shared_ptr<CPaintItem> item_;
};


class CAddItemCommand : public CSharedPaintCommand
{
public:
	CAddItemCommand( CSharedPaintManager *spMngr, boost::shared_ptr<CPaintItem> item ) : CSharedPaintCommand(spMngr, item) { }
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
	CRemoveItemCommand( CSharedPaintManager *spMngr, boost::shared_ptr<CPaintItem> item ) : CSharedPaintCommand(spMngr, item) { }
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
	CUpdateItemCommand( CSharedPaintManager *spMngr, boost::shared_ptr<CPaintItem> item ) 
		: CSharedPaintCommand(spMngr, item)
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
	CMoveItemCommand( CSharedPaintManager *spMngr, boost::shared_ptr<CPaintItem> item ) 
		: CSharedPaintCommand(spMngr, item)
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
