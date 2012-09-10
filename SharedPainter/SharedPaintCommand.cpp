#include "StdAfx.h"
#include "SharedPaintCommand.h"
#include "SharedPaintManager.h"

bool CAddItemCommand::execute( bool sendData )
{
	if ( sendData )
	{
		std::string msg = PaintPacketBuilder::CAddItem::make( item_ );
		int packetId = manager_->sendDataToUsers( msg );
		item_->setPacketId( packetId );
	}
	manager_->addPaintItem( item_ );
	return true;
}

void CAddItemCommand::undo( bool sendData )
{
	if ( sendData )
	{
		std::string msg = PaintPacketBuilder::CRemoveItem::make( item_->owner(), item_->itemId() );
		manager_->sendDataToUsers( msg );
	}
	manager_->removePaintItem( item_->owner(), item_->itemId() );
}


bool CRemoveItemCommand::execute( bool sendData )
{
	if ( sendData )
	{
		std::string msg = PaintPacketBuilder::CRemoveItem::make( item_->owner(), item_->itemId() );
		manager_->sendDataToUsers( msg );
	}
	manager_->removePaintItem( item_->owner(), item_->itemId() );
	return true;
}

void CRemoveItemCommand::undo( bool sendData )
{
	if( sendData )
	{
		std::string msg = PaintPacketBuilder::CAddItem::make( item_ );
		int packetId = manager_->sendDataToUsers( msg );
		item_->setPacketId( packetId );
	}
	manager_->addPaintItem( item_ );
}

bool CUpdateItemCommand::execute( bool sendData )
{
	if( sendData )
	{
		std::string msg = PaintPacketBuilder::CUpdateItem::make( item_ );
		int packetId = manager_->sendDataToUsers( msg );
		item_->setPacketId( packetId );
	}
	item_->setData( data_ );
	manager_->updatePaintItem( item_ );
	return true;
}

void CUpdateItemCommand::undo( bool sendData )
{
	if( sendData )
	{
		std::string msg = PaintPacketBuilder::CUpdateItem::make( item_ );
		int packetId = manager_->sendDataToUsers( msg );
		item_->setPacketId( packetId );
	}
	item_->setData( prevData_ );
	manager_->updatePaintItem( item_ );
}


bool CMoveItemCommand::execute( bool sendData )
{
	if( sendData )
	{
		std::string msg = PaintPacketBuilder::CMoveItem::make( item_->owner(), item_->itemId(), posX_, posY_ );	
		manager_->sendDataToUsers( msg );
	}

	manager_->movePaintItem( item_, posX_, posY_ );
	return true;
}

void CMoveItemCommand::undo( bool sendData )
{
	if( sendData )
	{
		std::string msg = PaintPacketBuilder::CMoveItem::make( item_->owner(), item_->itemId(), prevX_, prevY_ );
		manager_->sendDataToUsers( msg );
	}
	manager_->movePaintItem( item_, prevX_, prevY_ );
}
