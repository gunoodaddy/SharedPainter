#include "StdAfx.h"
#include "SharedPaintCommand.h"
#include "SharedPaintManager.h"

bool CAddItemCommand::execute( void )
{
	boost::shared_ptr<CAddItemTask> task = boost::shared_ptr<CAddItemTask>(new CAddItemTask(manager_, item_));
	if( task->doit( true ) )
	{
		cmdManager_->pushTask( task );
		return true;
	}
	return false;
}

bool CAddItemCommand::undo( void )
{
	boost::shared_ptr<CRemoveItemTask> task = boost::shared_ptr<CRemoveItemTask>(new CRemoveItemTask(manager_, item_));
	if( task->doit( true ) )
	{
		cmdManager_->pushTask( task );
		return true;
	}
	return false;
}


bool CRemoveItemCommand::execute( void )
{
	boost::shared_ptr<CRemoveItemTask> task = boost::shared_ptr<CRemoveItemTask>(new CRemoveItemTask(manager_, item_));
	if( task->doit( true ) )
	{
		cmdManager_->pushTask( task );
		return true;
	}
	return false;
}

bool CRemoveItemCommand::undo( void )
{
	boost::shared_ptr<CAddItemTask> task = boost::shared_ptr<CAddItemTask>(new CAddItemTask(manager_, item_));
	if( task->doit( true ) )
	{
		cmdManager_->pushTask( task );
		return true;
	}
	return false;
}

bool CUpdateItemCommand::execute( void )
{
	boost::shared_ptr<CUpdateItemTask> task = boost::shared_ptr<CUpdateItemTask>(new CUpdateItemTask(manager_, item_, data_));
	if( task->doit( true ) )
	{
		cmdManager_->pushTask( task );
		return true;
	}
	return false;
}

bool CUpdateItemCommand::undo( void )
{
	boost::shared_ptr<CUpdateItemTask> task = boost::shared_ptr<CUpdateItemTask>(new CUpdateItemTask(manager_, item_, prevData_));
	if( task->doit( true ) )
	{
		cmdManager_->pushTask( task );
		return true;
	}
	return false;
}


bool CMoveItemCommand::execute( void )
{
	boost::shared_ptr<CMoveItemTask> task = boost::shared_ptr<CMoveItemTask>(new CMoveItemTask(manager_, item_, posX_, posY_));
	if( task->doit( true ) )
	{
		cmdManager_->pushTask( task );
		return true;
	}
	return false;
}

bool CMoveItemCommand::undo( void )
{
	boost::shared_ptr<CMoveItemTask> task = boost::shared_ptr<CMoveItemTask>(new CMoveItemTask(manager_, item_, prevX_, prevY_));
	if( task->doit( true ) )
	{
		cmdManager_->pushTask( task );
		return true;
	}
	return false;
}
