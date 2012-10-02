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
#include "stdafx.h"
#include "SharedPaintManager.h"
#include "SharedPaintCommandManager.h"
#include "DefferedCaller.h"

static CDefferedCaller gCaller;

void CSharedPaintCommandManager::setAllowPainterToDraw( const std::string &userId, bool enabled )
{
	if( enabled )
	{
		if( isAllowPainterToDraw( userId ) )
			return;
		allowPainters_.insert( userId );
	}
	else
	{
		if( !isAllowPainterToDraw( userId ) )
			return;
		allowPainters_.erase( userId );
	}

	if( enabled )
	{
		for( int i = 0; i <= currentPlayPos_; i++ )
		{
			if( historyTaskList_[i]->owner() != userId )
				continue;
			historyTaskList_[i]->execute();
		}
	}
	else
	{
		for( int i = currentPlayPos_; i >= 0; i-- )
		{
			if( historyTaskList_[i]->owner() != userId )
				continue;
			historyTaskList_[i]->rollback();
		}
	}
}

bool CSharedPaintCommandManager::executeTask( boost::shared_ptr<CSharedPaintTask> task, bool sendData )
{
	task->setSharedPaintManager( spManager_ );
	task->setCommandManager( this );

	// check current playback position
	{
		boost::recursive_mutex::scoped_lock autolock(mutex_);

		bool playbackWorkingFlag = isPlaybackMode();

		historyTaskList_.push_back( task );

		if( ! playbackWorkingFlag )
			currentPlayPos_ = historyTaskList_.size() - 1;

		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_AddTask, spManager_, historyTaskList_.size(), playbackWorkingFlag ) );

		task->setSendData( sendData );

		// now playback working or user not allowed, skip to execute this task.
		if( playbackWorkingFlag || !isAllowPainterToDraw(task->owner()))
			return true;
	}

	if( !task->execute() )
		return false;

	return true;
}

void CSharedPaintCommandManager::playbackTo( int position )
{
	//qDebug() << "playbackTo()" << position;
	if( currentPlayPos_ < position )
	{
		_playforwardTo( currentPlayPos_, position );
	}
	else
	{
		_playbackwardTo( currentPlayPos_, position );
	}

	currentPlayPos_ = position;
}


void CSharedPaintCommandManager::_playforwardTo( int from, int to )
{
	if( from < -1 || from >= (int)historyTaskList_.size() )
		return;
	if( to < 0 || to >= (int)historyTaskList_.size() )
		return;

	for( int i = from + 1; i <= to; i++ )
	{
		//qDebug() << "_playforwardTo" << i << from << to;
		if( isAllowPainterToDraw( historyTaskList_[i]->owner() ) )
			historyTaskList_[i]->execute();
	}
}

void CSharedPaintCommandManager::_playbackwardTo( int from, int to )
{
	if( from < 0 || from >= (int)historyTaskList_.size() )
		return;
	if( to < -1 || to >= (int)historyTaskList_.size() )
		return;

	for( int i = from; i > to; i-- )
	{
		//qDebug() << "_playbackwardTo" << i << from << to;
		if( isAllowPainterToDraw( historyTaskList_[i]->owner() ) )
			historyTaskList_[i]->rollback();
	}
}
