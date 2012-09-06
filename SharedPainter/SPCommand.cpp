#include "StdAfx.h"
#include "SPCommand.h"
#include "SharedPaintManager.h"

bool CAddItemCommand::execute( void )
{
	manager_->addPaintItem( item_ );

	std::string msg = PaintPacketBuilder::CAddItem::make( item_ );
	int packetId = manager_->sendDataToUsers( msg );
	item_->setPacketId( packetId );
	return true;
}

void CAddItemCommand::undo( void )
{
	std::string msg = PaintPacketBuilder::CRemoveItem::make( item_->owner(), item_->itemId() );
	manager_->sendDataToUsers( msg );
	manager_->removePaintItem( item_->owner(), item_->itemId() );
}


bool CUpdateItemCommand::execute( void )
{
	prevData_ = item_->prevData();

	manager_->updatePaintItem( item_ );

	std::string msg = PaintPacketBuilder::CUpdateItem::make( item_ );
	int packetId = manager_->sendDataToUsers( msg );
	item_->setPacketId( packetId );
	return true;
}

void CUpdateItemCommand::undo( void )
{
	item_->setData( prevData_ );

	manager_->updatePaintItem( item_ );

	std::string msg = PaintPacketBuilder::CUpdateItem::make( item_ );
	int packetId = manager_->sendDataToUsers( msg );
	item_->setPacketId( packetId );
}


bool CMoveItemCommand::execute( void )
{
	std::string msg = PaintPacketBuilder::CMoveItem::make( item_->owner(), item_->itemId(), item_->posX(), item_->posY() );
	manager_->sendDataToUsers( msg );
	return true;
}

void CMoveItemCommand::undo( void )
{
	double x = item_->prevData().posX;
	double y = item_->prevData().posY;
	item_->move( x, y );

	std::string msg = PaintPacketBuilder::CMoveItem::make( item_->owner(), item_->itemId(), item_->posX(), item_->posY() );
	manager_->sendDataToUsers( msg );
}
