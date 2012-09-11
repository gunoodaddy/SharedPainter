#include "StdAfx.h"
#include "SharedPaintCommand.h"
#include "SharedPaintManager.h"
#include "TaskPacketBuilder.h"

#define DEBUG_PRINT_TASK()	qDebug() << __FUNCTION__ << sendData << "history item cnt : " << cmdMngr_->historyItemCount();

static CDefferedCaller gCaller;


void CSharedPaintTask::sendPacket( void )
{
	std::string msg = TaskPacketBuilder::CExecuteTask::make( shared_from_this() );
	spMngr_->sendDataToUsers( msg );
}

bool CAddItemTask::doit( bool sendData )
{
	DEBUG_PRINT_TASK();

	if ( sendData )
	{
		sendPacket();
	}
	
	boost::shared_ptr<CPaintItem> item = cmdMngr_->findItem( data_.owner, data_.itemId );
	if( item )
	{
		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_AddPaintItem, spMngr_, item ) );
	}

	return true;
}

bool CRemoveItemTask::doit( bool sendData )
{
	DEBUG_PRINT_TASK();

	if ( sendData )
	{
		sendPacket();
	}

	boost::shared_ptr<CPaintItem> item = cmdMngr_->findItem( data_.owner, data_.itemId );
	if( item )
	{
		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_RemovePaintItem, spMngr_, item ) );
	}
	return true;
}

bool CUpdateItemTask::doit( bool sendData )
{
	DEBUG_PRINT_TASK();

	if( sendData )
	{
		sendPacket();
	}

	boost::shared_ptr<CPaintItem> item = cmdMngr_->findItem( data_.owner, data_.itemId );
	if( item )
	{
		item->setData( paintData_ );
		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_UpdatePaintItem, spMngr_, item ) );
	}
	return true;
}

bool CMoveItemTask::doit( bool sendData )
{
	DEBUG_PRINT_TASK();

	if( sendData )
	{
		sendPacket();
	}

	boost::shared_ptr<CPaintItem> item = cmdMngr_->findItem( data_.owner, data_.itemId );
	if( item )
	{
		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_MovePaintItem, spMngr_, item, posX_, posY_ ) );
	}
	return true;
}
