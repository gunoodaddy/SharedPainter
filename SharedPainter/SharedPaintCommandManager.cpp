#include "stdafx.h"
#include "SharedPaintCommandManager.h"

void CSharedPaintCommandManager::playbackTo( int position )
{
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
	for( int i = from; i <= to; i++ )
	{
		qDebug() << "_playforwardTo" << i << from << to;
		historyTaskList_[i]->doit( false );
	}
}

void CSharedPaintCommandManager::_playbackwardTo( int from, int to )
{
	for( int i = from; i >= to; i-- )
	{
		qDebug() << "_playbackwardTo" << i << from << to;
		historyTaskList_[i]->doit( false );
	}
}