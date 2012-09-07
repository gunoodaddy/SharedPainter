#pragma once

#include "SharedPaintCommand.h"

class CSharedPaintCommandManager
{
public:
	CSharedPaintCommandManager() { }

	void clear( void )
	{
		while( commandList_.size() > 0 )
			commandList_.pop();
	}

	bool executeCommand( boost::shared_ptr< CSharedPaintCommand > command )
	{
		bool ret = command->execute();
		if( ret )
			commandList_.push( command );

		return ret;
	}

	void undoCommand( void )
	{
		if( commandList_.size() <= 0 )
			return;

		boost::shared_ptr< CSharedPaintCommand > command = commandList_.top();
		command->undo();

		commandList_.pop();
	}

protected:
	typedef std::stack< boost::shared_ptr< CSharedPaintCommand > > commandlist_t;
	commandlist_t commandList_;
};