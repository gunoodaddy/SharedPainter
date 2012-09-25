/*                                                                                                                                           
* Copyright (c) 2012, Eunhyuk Kim(gunoodaddy) 
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*   * Neither the name of Redis nor the names of its contributors may be used
*     to endorse or promote products derived from this software without
*     specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

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

