#include "StdAfx.h"
#include "SharedPaintCommand.h"
#include "SharedPaintManager.h"

bool CAddItemCommand::execute( void )
{
	boost::shared_ptr<CAddItemTask> task = boost::shared_ptr<CAddItemTask>(new CAddItemTask(spMngr_, item_->owner(), item_->itemId() ));
	return cmdMngr_->executeTask( task );
}

bool CAddItemCommand::undo( void )
{
	boost::shared_ptr<CRemoveItemTask> task = boost::shared_ptr<CRemoveItemTask>(new CRemoveItemTask(spMngr_, item_->owner(), item_->itemId()));
	return cmdMngr_->executeTask( task );
}

bool CRemoveItemCommand::execute( void )
{
	boost::shared_ptr<CRemoveItemTask> task = boost::shared_ptr<CRemoveItemTask>(new CRemoveItemTask(spMngr_, item_->owner(), item_->itemId()));
	return cmdMngr_->executeTask( task );
}

bool CRemoveItemCommand::undo( void )
{
	boost::shared_ptr<CAddItemTask> task = boost::shared_ptr<CAddItemTask>(new CAddItemTask(spMngr_, item_->owner(), item_->itemId()));
	return cmdMngr_->executeTask( task );
}

bool CUpdateItemCommand::execute( void )
{
	boost::shared_ptr<CUpdateItemTask> task = boost::shared_ptr<CUpdateItemTask>(new CUpdateItemTask(spMngr_, item_->owner(), item_->itemId(), data_));
	return cmdMngr_->executeTask( task );
}

bool CUpdateItemCommand::undo( void )
{
	boost::shared_ptr<CUpdateItemTask> task = boost::shared_ptr<CUpdateItemTask>(new CUpdateItemTask(spMngr_, item_->owner(), item_->itemId(), prevData_));
	return cmdMngr_->executeTask( task );
}


bool CMoveItemCommand::execute( void )
{
	boost::shared_ptr<CMoveItemTask> task = boost::shared_ptr<CMoveItemTask>(new CMoveItemTask(spMngr_, item_->owner(), item_->itemId(), posX_, posY_));
	return cmdMngr_->executeTask( task );
}

bool CMoveItemCommand::undo( void )
{
	boost::shared_ptr<CMoveItemTask> task = boost::shared_ptr<CMoveItemTask>(new CMoveItemTask(spMngr_, item_->owner(), item_->itemId(), prevX_, prevY_));
	return cmdMngr_->executeTask( task );
}
