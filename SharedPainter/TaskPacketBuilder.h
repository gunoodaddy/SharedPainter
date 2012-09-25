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

#include "CommonPacketBuilder.h"
#include "SharedPaintTaskFactory.h"

namespace TaskPacketBuilder
{
	class CExecuteTask
	{
	public:
		static std::string make( boost::shared_ptr<CSharedPaintTask> task, const std::string *target = NULL )
		{
			std::string body;
			std::string data = task->serialize();

			int pos = 0;
			try
			{
				pos += CPacketBufferUtil::writeInt16( body, pos, task->type(), true );
				body += data;

				return CommonPacketBuilder::makePacket( CODE_TASK_EXECUTE, body, NULL, target );
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
				boost::uint16_t temptype;
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
