#pragma once

#include <stack>
#include "SharedPaintCommand.h"

class CSharedPaintCommandManager
{
public:
	CSharedPaintCommandManager() { }

	void clear( void )
	{
		while( commandList_.size() > 0 )
			commandList_.pop();
		while( redoCommandList_.size() > 0 )
			redoCommandList_.pop();
	}

	bool executeCommand( boost::shared_ptr< CSharedPaintCommand > command )
	{
		bool ret = command->execute();
		if( ret )
			commandList_.push( command );

		return ret;
	}

	void redoCommand( void )
	{
		if( redoCommandList_.size() <= 0 )
			return;

		boost::shared_ptr< CSharedPaintCommand > command = redoCommandList_.top();
		command->execute();

		commandList_.push( command );
		redoCommandList_.pop();
	}

	void undoCommand( void )
	{
		if( commandList_.size() <= 0 )
			return;

		boost::shared_ptr< CSharedPaintCommand > command = commandList_.top();
		command->undo();

		redoCommandList_.push( command );
		commandList_.pop();
	}

protected:
	typedef std::stack< boost::shared_ptr< CSharedPaintCommand > > commandlist_t;
	commandlist_t commandList_;
	commandlist_t redoCommandList_;
};
