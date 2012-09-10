#pragma once

#include "PaintItem.h"

class CSharedPaintManager;
class CSharedPaintCommandManager;

class CSharedPaintTask
{
public:
	CSharedPaintTask( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) { }
	virtual ~CSharedPaintTask( void ) { }

public:
	virtual bool doit( bool sendData = true ) = 0;

	boost::shared_ptr<CPaintItem> item( void ) { return item_; }

protected:
	friend class CSharedPaintCommandManager;
	CSharedPaintManager *manager_;
	boost::shared_ptr<CPaintItem> item_;
};

//------------------------------------------------------------------------------------------------------

class CAddItemTask : public CSharedPaintTask
{
public:
	CAddItemTask( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) : CSharedPaintTask(manager, item) { }
	~CAddItemTask( void )
	{
		qDebug() << "~CAddItemTask";
	}

	virtual bool doit( bool sendData = true );
};

class CRemoveItemTask : public CSharedPaintTask
{
public:
	CRemoveItemTask( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item ) : CSharedPaintTask(manager, item) { }
	~CRemoveItemTask( void )
	{
		qDebug() << "~CRemoveItemTask";
	}

	virtual bool doit( bool sendData = true );
};


class CUpdateItemTask : public CSharedPaintTask
{
public:
	CUpdateItemTask( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item, const struct SPaintData &data ) : CSharedPaintTask(manager, item)
		, data_(data) { }
	~CUpdateItemTask( void )
	{
		qDebug() << "~CUpdateItemTask";
	}

	virtual bool doit( bool sendData = true );

private:
	struct SPaintData data_;
};


class CMoveItemTask : public CSharedPaintTask
{
public:
	CMoveItemTask( CSharedPaintManager *manager, boost::shared_ptr<CPaintItem> item, double posX, double posY ) : CSharedPaintTask(manager, item)
		, posX_(posX), posY_(posY) { }
	~CMoveItemTask( void )
	{
		qDebug() << "~CMoveItemCommand";
	}

	virtual bool doit( bool sendData = true );

private:
	double posX_;
	double posY_;
};
