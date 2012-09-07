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
		qDebug() << "CNetPeerServer()" << this;
	}

	~CNetPeerServer(void)
	{
		qDebug() << "~CNetPeerServer()" << this;
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
