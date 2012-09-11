#pragma once

#include "CommonPacketBuilder.h"
#include "SharedPaintTaskFactory.h"

namespace TaskPacketBuilder
{
	class CExecuteTask
	{
	public:
		static std::string make( boost::shared_ptr<CSharedPaintTask> task )
		{
			std::string body;
			std::string data = task->serialize();

			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::writeInt16( body, pos, task->type(), true );
				body += data;

				return CommonPacketBuilder::makePacket( CODE_TASK_EXECUTE, body );
			}catch(...)
			{
				
			}

			return "";
		}

		static boost::shared_ptr<CSharedPaintTask> parse( const std::string &body )
		{		
			boost::shared_ptr< CSharedPaintTask > task;

			try
			{
				int pos = 0;
				boost::int16_t temptype;
				pos += CPacketBufferUtil::readInt16( body, pos, temptype, true );

				TaskType type = (TaskType)temptype;

				task = CSharedPaintTaskFactory::createTask( type );

				std::string taskData( (const char *)body.c_str() + pos, body.size() - pos );
				if( !task->deserialize( taskData ) )
				{
					return boost::shared_ptr<CSharedPaintTask>();
				}
			}catch(...)
			{
				return boost::shared_ptr<CSharedPaintTask>();
			}

			return task;
		}
	};
};