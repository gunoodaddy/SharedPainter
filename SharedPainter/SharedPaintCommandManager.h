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

#pragma once

#include <stack>
#include "SharedPaintCommand.h"
#include "SharedPaintManagementData.h"

class CSharedPaintManager;

class CSharedPaintCommandManager
{
public:
	static const int DEFAULT_INIT_PLAYBACK_POS = -2;	// unreachable value
	typedef std::map< std::string, boost::shared_ptr<CSharedPaintItemList> > ITEM_LIST_MAP;

	CSharedPaintCommandManager( CSharedPaintManager *spManager ) : spManager_(spManager), currentPlayPos_(DEFAULT_INIT_PLAYBACK_POS) { }

	void clear( void )
	{
		clearHistoryItem();
		clearHistoryTask();
		clearHistoryCommand();
	}

	void clearHistoryItem( void )
	{
		boost::recursive_mutex::scoped_lock autolock(mutex_);
	
		historyItemSet_.clear();
		userItemListMap_.clear();
	}

	void clearHistoryTask( void )
	{
		currentPlayPos_ = DEFAULT_INIT_PLAYBACK_POS;
		boost::recursive_mutex::scoped_lock autolock(mutex_);
		historyTaskList_.clear();
	}

	void clearHistoryCommand( void )
	{
		boost::recursive_mutex::scoped_lock autolock(mutex_);
		while( commandList_.size() > 0 )
			commandList_.pop();
		while( redoCommandList_.size() > 0 )
			redoCommandList_.pop();
	}

	size_t historyItemCount( void ) 
	{ 
		return historyItemSet_.size();
	}

	size_t historyTaskCount( void ) 
	{ 
		return historyTaskList_.size();
	}

	void lock( void ) { mutex_.lock(); }
	void unlock( void ) { mutex_.unlock(); }

	const ITEM_SET &historyItemSet( void )
	{
		return historyItemSet_;
	}

	const TASK_LIST &historyTaskList( void )
	{
		return historyTaskList_;
	}

	bool executeTask( boost::shared_ptr<CSharedPaintTask> task, bool sendData = true );

	bool executeCommand( boost::shared_ptr< CSharedPaintCommand > command )
	{
		command->setCommandManager( this );
		command->setSharedPaintManager( spManager_ );

		bool ret = command->execute();

		if( ret )
		{
			boost::recursive_mutex::scoped_lock autolock(mutex_);

			commandList_.push( command );

			// redo-list clear
			while( redoCommandList_.size() > 0 )
				redoCommandList_.pop();
		}
		return ret;
	}

	bool redoCommand( void  )
	{
		if( redoCommandList_.size() <= 0 )
			return false;

		boost::shared_ptr< CSharedPaintCommand > command = redoCommandList_.top();
		bool ret = command->execute();

		if( ret )
		{
			boost::recursive_mutex::scoped_lock autolock(mutex_);
			commandList_.push( command );
			redoCommandList_.pop();
		}
		return ret;
	}

	bool undoCommand( void )
	{
		if( commandList_.size() <= 0 )
			return false;

		boost::shared_ptr< CSharedPaintCommand > command = commandList_.top();
		bool ret = command->undo();
		
		if( ret )
		{
			boost::recursive_mutex::scoped_lock autolock(mutex_);
			redoCommandList_.push( command );
			commandList_.pop();
		}
		return ret;
	}

	bool isPlaybackMode( void )
	{
		if( DEFAULT_INIT_PLAYBACK_POS == currentPlayPos_ /* init position */
				|| ((int)historyTaskList_.size() - 1 == currentPlayPos_ /* last position */ ) )
		{
			return false;
		}
		return true;
	}

	void playbackTo( int position );

	void addHistoryItem( boost::shared_ptr<CPaintItem> item )
	{
		boost::recursive_mutex::scoped_lock autolock(mutex_);

		boost::shared_ptr<CSharedPaintItemList> itemList;
		ITEM_LIST_MAP::iterator it = userItemListMap_.find( item->owner() );
		if( it != userItemListMap_.end() )
		{
			itemList = it->second;
		}
		else
		{
			itemList = boost::shared_ptr<CSharedPaintItemList>( new CSharedPaintItemList( item->owner() ) );
			userItemListMap_.insert( ITEM_LIST_MAP::value_type(item->owner(), itemList) );
		}

		if(itemList->addItem( item ))
			historyItemSet_.insert( item );
	}

	boost::shared_ptr<CPaintItem> findItem( const std::string &owner, int itemId )
	{
		boost::recursive_mutex::scoped_lock autolock(mutex_);

		boost::shared_ptr<CSharedPaintItemList> itemList = _findItemList( owner );
		if( !itemList )
			return boost::shared_ptr<CPaintItem>();

		return itemList->findItem( itemId );
	}

	void addPainter( const std::string &userId )
	{
		boost::recursive_mutex::scoped_lock autolock(mutex_);
		allowPainters_.insert( userId );
	}

	void setAllowPainterToDraw( const std::string &userId, bool enabled );

	bool isAllowPainterToDraw( const std::string &userId )
	{
		return allowPainters_.find( userId ) != allowPainters_.end();
	}

private:

	void _playforwardTo( int from, int to );
	void _playbackwardTo( int from, int to );

	// not thread safe..
	boost::shared_ptr<CSharedPaintItemList> _findItemList( const std::string &owner )	
	{
		ITEM_LIST_MAP::iterator it = userItemListMap_.find( owner );
		if( it == userItemListMap_.end() )
			return boost::shared_ptr<CSharedPaintItemList>();

		return it->second;
	}

protected:
	typedef std::stack< boost::shared_ptr< CSharedPaintCommand > > COMMAND_LIST;

	CSharedPaintManager *spManager_;

	TASK_LIST historyTaskList_;
	ITEM_SET historyItemSet_;		// for iterating
	ITEM_LIST_MAP userItemListMap_;	// for searching

	COMMAND_LIST commandList_;
	COMMAND_LIST redoCommandList_;

	boost::recursive_mutex mutex_;

	std::set< std::string > allowPainters_;
	int currentPlayPos_;
};
