#include "StdAfx.h"
#include "SharedPaintCommand.h"
#include "SharedPaintManager.h"
#include "TaskPacketBuilder.h"

#define DEBUG_PRINT_TASK()	qDebug() << __FUNCTION__ << "history item cnt : " << cmdMngr_->historyItemCount();

static CDefferedCaller gCaller;


void CSharedPaintTask::sendPacket( void )
{
	if( sendData_ )
	{
		std::string msg = TaskPacketBuilder::CExecuteTask::make( shared_from_this() );
		spMngr_->sendDataToUsers( msg );
		sendData_ = false;
	}
}

bool CAddItemTask::execute( void )
{
	DEBUG_PRINT_TASK();

	sendPacket();
	
	boost::shared_ptr<CPaintItem> item = cmdMngr_->findItem( data_.owner, data_.itemId );
	if( item )
	{
		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_AddPaintItem, spMngr_, item ) );
	}

	return true;
}

void CAddItemTask::rollback( void )
{
	DEBUG_PRINT_TASK();

	boost::shared_ptr<CPaintItem> item = cmdMngr_->findItem( data_.owner, data_.itemId );
	if( item )
	{
		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_RemovePaintItem, spMngr_, item ) );
	}
}


bool CRemoveItemTask::execute( void )
{
	DEBUG_PRINT_TASK();

	sendPacket();

	boost::shared_ptr<CPaintItem> item = cmdMngr_->findItem( data_.owner, data_.itemId );
	if( item )
	{
		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_RemovePaintItem, spMngr_, item ) );
	}
	return true;
}

void CRemoveItemTask::rollback( void )
{
	DEBUG_PRINT_TASK();

	boost::shared_ptr<CPaintItem> item = cmdMngr_->findItem( data_.owner, data_.itemId );
	if( item )
	{
		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_AddPaintItem, spMngr_, item ) );
	}
}


bool CUpdateItemTask::execute( void )
{
	DEBUG_PRINT_TASK();

	sendPacket();

	boost::shared_ptr<CPaintItem> item = cmdMngr_->findItem( data_.owner, data_.itemId );
	if( item )
	{
		item->setData( paintData_ );
		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_UpdatePaintItem, spMngr_, item ) );
	}
	return true;
}

void CUpdateItemTask::rollback( void )
{
	DEBUG_PRINT_TASK();

	boost::shared_ptr<CPaintItem> item = cmdMngr_->findItem( data_.owner, data_.itemId );
	if( item )
	{
		item->setData( prevPaintData_ );
		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_UpdatePaintItem, spMngr_, item ) );
	}
}


bool CMoveItemTask::execute( void )
{
	DEBUG_PRINT_TASK();

	sendPacket();

	boost::shared_ptr<CPaintItem> item = cmdMngr_->findItem( data_.owner, data_.itemId );
	if( item )
	{
		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_MovePaintItem, spMngr_, item, posX_, posY_ ) );
	}
	return true;
}

void CMoveItemTask::rollback( void )
{
	DEBUG_PRINT_TASK();

	boost::shared_ptr<CPaintItem> item = cmdMngr_->findItem( data_.owner, data_.itemId );
	if( item )
	{
		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_MovePaintItem, spMngr_, item, prevPosX_, prevPosY_ ) );
	}
}

