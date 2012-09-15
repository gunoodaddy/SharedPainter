#include "StdAfx.h"
#include "SharedPaintCommand.h"
#include "SharedPaintManager.h"

bool CAddItemCommand::execute( void )
{
	boost::shared_ptr<CAddItemTask> task(new CAddItemTask(item_->owner(), item_->itemId() ));
	return cmdMngr_->executeTask( task );
}

bool CAddItemCommand::undo( void )
{
	boost::shared_ptr<CRemoveItemTask> task(new CRemoveItemTask(item_->owner(), item_->itemId()));
	return cmdMngr_->executeTask( task );
}

bool CRemoveItemCommand::execute( void )
{
	boost::shared_ptr<CRemoveItemTask> task(new CRemoveItemTask(item_->owner(), item_->itemId()));
	return cmdMngr_->executeTask( task );
}

bool CRemoveItemCommand::undo( void )
{
	boost::shared_ptr<CAddItemTask> task(new CAddItemTask(item_->owner(), item_->itemId()));
	return cmdMngr_->executeTask( task );
}

bool CUpdateItemCommand::execute( void )
{
	boost::shared_ptr<CUpdateItemTask> task(new CUpdateItemTask(item_->owner(), item_->itemId(), prevData_, data_));
	return cmdMngr_->executeTask( task );
}

bool CUpdateItemCommand::undo( void )
{
	boost::shared_ptr<CUpdateItemTask> task(new CUpdateItemTask(item_->owner(), item_->itemId(), prevData_, data_));
	return cmdMngr_->executeTask( task );
}


bool CMoveItemCommand::execute( void )
{
	boost::shared_ptr<CMoveItemTask> task(new CMoveItemTask(item_->owner(), item_->itemId(), prevX_, prevY_, posX_, posY_));
	return cmdMngr_->executeTask( task );
}

bool CMoveItemCommand::undo( void )
{
	boost::shared_ptr<CMoveItemTask> task(new CMoveItemTask(item_->owner(), item_->itemId(), prevX_, prevY_, posX_, posY_));
	return cmdMngr_->executeTask( task );
}
