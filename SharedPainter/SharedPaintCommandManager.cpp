#include "stdafx.h"
#include "SharedPaintManager.h"
#include "SharedPaintCommandManager.h"
#include "DefferedCaller.h"

static CDefferedCaller gCaller;

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

		// now playback working, skip to execute this task.
		if( playbackWorkingFlag )
			return true;
	}

	if( !task->execute() )
		return false;

	return true;
}

void CSharedPaintCommandManager::playbackTo( int position )
{
	qDebug() << "playbackTo()" << position;
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
		qDebug() << "_playforwardTo" << i << from << to;
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
		qDebug() << "_playbackwardTo" << i << from << to;
		historyTaskList_[i]->rollback();
	}
}
