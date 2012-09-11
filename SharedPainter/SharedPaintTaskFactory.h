#pragma once

#include "SharedPaintTask.h"

class CSharedPaintTaskFactory
{
public:
	static boost::shared_ptr<CSharedPaintTask> createTask( TaskType type )
	{
		boost::shared_ptr<CSharedPaintTask> task;
		switch( type )
		{
		case Task_AddItem:
			task = boost::shared_ptr<CAddItemTask>(new CAddItemTask);
			break;
		case Task_RemoveItem:
			task = boost::shared_ptr<CRemoveItemTask>(new CRemoveItemTask);
			break;
		case Task_MoveItem:
			task = boost::shared_ptr<CMoveItemTask>(new CMoveItemTask);
			break;
		case Task_UpdateItem:
			task = boost::shared_ptr<CUpdateItemTask>(new CUpdateItemTask);
			break;
		default:
			assert( false && "not supported task type" );
		}
		return task;
	}
};