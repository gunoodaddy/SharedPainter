#include "stdafx.h"
#include "SharedPaintManager.h"
#include "SharedPaintCommandManager.h"
#include "DefferedCaller.h"

static CDefferedCaller gCaller;

bool CSharedPaintCommandManager::executeTask( boost::shared_ptr<CSharedPaintTask> task, bool sendData )
{
	task->setCommandManager( this );

	// check current playback position
	{
		boost::recursive_mutex::scoped_lock autolock(mutex_);
		historyTaskList_.push_back( task );

		bool playbackWorkingFlag = false;
		if( DEFAULT_INIT_PLAYBACK_POS != currentPlayPos_ /* init position */
				&& (historyTaskList_.size() - 2 != currentPlayPos_ /* previous last position */ ) )
		{
			playbackWorkingFlag = true;
		}
		else
			currentPlayPos_ = historyTaskList_.size() - 1;

		gCaller.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_AddTask, spManager_, historyTaskList_.size(), playbackWorkingFlag ) );

		// now playback working, skip to execute this task.
		if( playbackWorkingFlag )
			return true;
	}

	if( !task->execute( sendData ) )
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
	for( int i = from + 1; i <= to; i++ )
	{
		qDebug() << "_playforwardTo" << i << from << to;
		historyTaskList_[i]->execute( false );
	}
}

void CSharedPaintCommandManager::_playbackwardTo( int from, int to )
{
	for( int i = from; i > to; i-- )
	{
		qDebug() << "_playbackwardTo" << i << from << to;
		historyTaskList_[i]->rollback();
	}
}
