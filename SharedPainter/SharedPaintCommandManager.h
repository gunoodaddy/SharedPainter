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

	bool executeCommand( boost::shared_ptr< CSharedPaintCommand > command, bool sendData = true )
	{
		bool ret = command->execute( sendData );
		if( ret )
			commandList_.push( command );

		while( redoCommandList_.size() > 0 )
			redoCommandList_.pop();

		return ret;
	}

	void redoCommand( bool sendData = true )
	{
		if( redoCommandList_.size() <= 0 )
			return;

		boost::shared_ptr< CSharedPaintCommand > command = redoCommandList_.top();
		command->execute( sendData );

		commandList_.push( command );
		redoCommandList_.pop();
	}

	void undoCommand( bool sendData = true )
	{
		if( commandList_.size() <= 0 )
			return;

		boost::shared_ptr< CSharedPaintCommand > command = commandList_.top();
		command->undo( sendData );

		redoCommandList_.push( command );
		commandList_.pop();
	}

protected:
	typedef std::stack< boost::shared_ptr< CSharedPaintCommand > > commandlist_t;
	commandlist_t commandList_;
	commandlist_t redoCommandList_;
};
