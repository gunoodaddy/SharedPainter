#pragma once

#include "SPCommand.h"

class CSPCommandManager
{
public:
	CSPCommandManager() { }

	void clear( void )
	{
		while( commandList_.size() > 0 )
			commandList_.pop();
	}

	bool executeCommand( boost::shared_ptr< CSPCommand > command )
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

		boost::shared_ptr< CSPCommand > command = commandList_.top();
		command->undo();

		commandList_.pop();
	}

protected:
	typedef std::stack< boost::shared_ptr< CSPCommand > > commandlist_t;
	commandlist_t commandList_;
};