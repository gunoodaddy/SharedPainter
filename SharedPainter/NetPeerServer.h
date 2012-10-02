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

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <deque>

#include "INetPeerEvent.h"
#include "NetPeerSession.h"
#include "NetServiceRunner.h"

using boost::asio::ip::tcp;

class CNetPeerServer : public boost::enable_shared_from_this<CNetPeerServer>
{
public:
	CNetPeerServer( boost::asio::io_service &io_service ) : evtTarget_(NULL), io_service_(io_service), acceptor_(io_service)
	{
		//qDebug() << "CNetPeerServer()" << this;
	}

	~CNetPeerServer(void)
	{
		//qDebug() << "~CNetPeerServer()" << this;
		close();
	}

	void setEvent( INetPeerServerEvent *evt ) 
	{
		evtTarget_ = evt;
	}

	bool start( int port )
	{
		try
		{
			boost::asio::ip::tcp::endpoint endpoint( boost::asio::ip::tcp::v4(), port );

			acceptor_.open( endpoint.protocol() );
			acceptor_.bind(endpoint);
			acceptor_.listen();

			_start_accept();
		}
		catch(...)
		{
			acceptor_.close();
			return false;
		}

		return true;
	}

	void close( void )
	{
		acceptor_.close();
	}

private:
	void _start_accept( void )
	{
		boost::shared_ptr<CNetPeerSession> session = boost::shared_ptr<CNetPeerSession>(new CNetPeerSession( io_service_, CNetServiceRunner::newSessionId() ));
		acceptor_.async_accept(session->socket(),
			boost::bind(&CNetPeerServer::_handle_accept, shared_from_this(), session,
			boost::asio::placeholders::error));
	}

	void _handle_accept( boost::shared_ptr<CNetPeerSession> new_session, const boost::system::error_code& error )
	{
		if( !error )
		{
			if( evtTarget_ )
				evtTarget_->onINetPeerServerEvent_Accepted( shared_from_this(), new_session );

			_start_accept();
		}
	}


private:
	INetPeerServerEvent *evtTarget_;
	boost::asio::io_service &io_service_;
	tcp::acceptor acceptor_;
};
