#pragma once

#include <stack>
#include "SharedPaintCommand.h"
#include "SharedPaintManagementData.h"

class CSharedPaintCommandManager
{
public:
	typedef std::map< std::string, boost::shared_ptr<CSharedPaintItemList> > ITEM_LIST_MAP;

	CSharedPaintCommandManager() { }

	void clear( void )
	{
		while( commandList_.size() > 0 )
			commandList_.pop();
		while( redoCommandList_.size() > 0 )
			redoCommandList_.pop();
	}

	void pushTask( boost::shared_ptr<CSharedPaintTask> task )
	{
		historyTaskList_.push_back( task );
	}

	bool executeCommand( boost::shared_ptr< CSharedPaintCommand > command, bool sendData = true )
	{
		bool ret = command->execute();
		command->setManager( this );

		if( ret )
		{
			addItem( command->item() );
			commandList_.push( command );
		}

		// redo-list clear
		while( redoCommandList_.size() > 0 )
			redoCommandList_.pop();

		return ret;
	}

	bool redoCommand( bool sendData = true )
	{
		if( redoCommandList_.size() <= 0 )
			return false;

		boost::shared_ptr< CSharedPaintCommand > command = redoCommandList_.top();
		bool ret = command->execute();

		commandList_.push( command );
		redoCommandList_.pop();
		return ret;
	}

	bool undoCommand( bool sendData = true )
	{
		if( commandList_.size() <= 0 )
			return false;

		boost::shared_ptr< CSharedPaintCommand > command = commandList_.top();
		bool ret = command->undo();

		redoCommandList_.push( command );
		commandList_.pop();
		return ret;
	}

	void addItem( boost::shared_ptr<CPaintItem> item )
	{
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

		bool overwriteFlag = false;
		itemList->addItem( item, overwriteFlag );
	}

	void removeItem( const std::string &owner, int itemId )
	{
		if( itemId < 0 )
			return;

		boost::shared_ptr<CSharedPaintItemList> itemList = findItemList( owner );
		if( !itemList )
			return;

		boost::shared_ptr<CPaintItem> item = itemList->findItem( itemId );
		if( !item )
			return;

		itemList->removeItem( itemId );
	}

	boost::shared_ptr<CSharedPaintItemList> findItemList( const std::string &owner )
	{
		ITEM_LIST_MAP::iterator it = userItemListMap_.find( owner );
		if( it == userItemListMap_.end() )
			return boost::shared_ptr<CSharedPaintItemList>();

		return it->second;
	}

protected:
	typedef std::stack< boost::shared_ptr< CSharedPaintCommand > > COMMAND_LIST;
	typedef std::list< boost::shared_ptr<CSharedPaintTask> > TASK_LIST;

	TASK_LIST historyTaskList_;
	COMMAND_LIST commandList_;
	COMMAND_LIST redoCommandList_;
	ITEM_LIST_MAP userItemListMap_;
};
