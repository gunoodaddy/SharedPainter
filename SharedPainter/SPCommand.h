#pragma once

#include "PaintItem.h"
#include "PaintPacketBuilder.h"
#include "WindowPacketBuilder.h"

class CSharedPaintManager;
class CSPCommandManager;

class CSPCommand
{
public:
	CSPCommand(void) { }
	virtual ~CSPCommand(void) { }

private:
	virtual bool execute( void ) = 0;
	virtual void undo( void ) = 0;

	friend class CSPCommandManager;
};


class CAddItemCommand : public CSPCommand
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


class CUpdateItemCommand : public CSPCommand
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


class CMoveItemCommand : public CSPCommand
{
public:
	CMoveItemCommand( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) : manager_(manager), item_(item) { }

	virtual bool execute( void );
	virtual void undo( void );

private:
	CSharedPaintManager *manager_;
	boost::shared_ptr<CPaintItem> item_;
};