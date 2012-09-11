#pragma once

#include <stack>
#include "SharedPaintCommand.h"
#include "SharedPaintManagementData.h"

class CSharedPaintCommandManager
{
public:
	typedef std::map< std::string, boost::shared_ptr<CSharedPaintItemList> > ITEM_LIST_MAP;

	CSharedPaintCommandManager() : currentPlayPos_(0) { }

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

	const TASK_ARRAY &historyTaskList( void )
	{
		return historyTaskList_;
	}

	bool executeTask( boost::shared_ptr<CSharedPaintTask> task, bool sendData = true )
	{
		task->setCommandManager( this );

		mutex_.lock();
		historyTaskList_.push_back( task );
		currentPlayPos_ = historyTaskList_.size() - 1;
		mutex_.unlock();

		if( !task->execute( sendData ) )
			return false;

		return true;
	}

	bool executeCommand( boost::shared_ptr< CSharedPaintCommand > command, bool sendData = true )
	{
		command->setCommandManager( this );

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

	bool redoCommand( bool sendData = true )
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

	bool undoCommand( bool sendData = true )
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

	TASK_ARRAY historyTaskList_;
	ITEM_SET historyItemSet_;		// for iterating
	ITEM_LIST_MAP userItemListMap_;	// for searching

	COMMAND_LIST commandList_;
	COMMAND_LIST redoCommandList_;

	boost::recursive_mutex mutex_;

	int currentPlayPos_;
};
