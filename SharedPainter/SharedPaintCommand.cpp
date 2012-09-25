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
