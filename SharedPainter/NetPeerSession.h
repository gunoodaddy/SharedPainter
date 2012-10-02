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

#include <boost/thread.hpp>  
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <deque>
#include "DefferedCaller.h"
#include "INetPeerEvent.h"

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

class CNetPeerSession : public boost::enable_shared_from_this<CNetPeerSession>
{
public:
	CNetPeerSession( boost::asio::io_service& io_service, int sessionId ) 
		: io_service_(io_service), sessionId_(sessionId), stopped_(true), connected_(false), evtTarget_(NULL), clientsocket_(io_service), deadline_(io_service) 
	{ 
		//qDebug() << "CNetPeerSession(void) " << this;
	}

	~CNetPeerSession( void )
	{
		//qDebug() << "~CNetPeerSession(void) " << this;
		close();
	}

	void setEvent( INetPeerSessionEvent * evt )
	{
		evtTarget_ = evt;
	}

	int sessionId( void ) { return sessionId_; }

	tcp::socket& socket() {
		return clientsocket_;
	}

	bool isConnecting( void ) { return ( clientsocket_.is_open() && !connected_ ); }
	bool isConnected( void ) { return (clientsocket_.is_open() && connected_); }

	bool connect( const std::string &ip, int port )
	{
		close();

		stopped_ = false;

		std::string portstr = boost::lexical_cast <std::string>(port);

		try
		{
			boost::asio::ip::tcp::resolver resolver( io_service_ );
			boost::asio::ip::tcp::resolver::query query( ip , portstr);
			boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve( query );

			_start_connect( iterator );

			deadline_.async_wait( boost::bind(&CNetPeerSession::_handle_check_deadline, shared_from_this()) );
			return true;
		}
		catch(...)
		{

		}
		return false;
	}

	void close( void )
	{
		// safe close for thread race condition..
		{
			boost::recursive_mutex::scoped_lock autolock(mutex_);

			if( clientsocket_.is_open() )
				clientsocket_.close();
		}

		if( connected_ )
		{
			connected_ = false;

			fireDisconnectedEvent();
		}
	}

	void start()
	{
		_connect_complete();
	}

	void sendData( const std::string & data )
	{
		boost::shared_ptr<CNetPacketData> packet = boost::shared_ptr<CNetPacketData>(new CNetPacketData(-1, data));
		sendData( packet );
	}
	
	void sendData( boost::shared_ptr<CNetPacketData> packet )
	{
		if( packet->buffer().totalSize() <= 0 )
			return;

		boost::recursive_mutex::scoped_lock autolock(mutex_);

		bool write_in_progress = !write_buffer_list_.empty(); // is there anything currently being written?

		write_buffer_list_.push_back( packet ); // store in write buffer

		if ( !write_in_progress ) // if nothing is currently being written, then start
			_start_write();
	}

public:
	void _connect_complete( void )
	{
		connected_ = true;
		deadline_.cancel();
		fireConnectSuccessEvent();
		// Start the input actor.
		_start_read();
	}

	void _start_connect(tcp::resolver::iterator endpoint_iter)
	{
		if (endpoint_iter != tcp::resolver::iterator())
		{
			deadline_.expires_from_now(boost::posix_time::seconds(10));

			// Start the asynchronous connect operation.
			clientsocket_.async_connect(endpoint_iter->endpoint(),
				boost::bind(&CNetPeerSession::_handle_connect,
				shared_from_this(), boost::asio::placeholders::error, endpoint_iter));

		}
		else
		{
			fireConnectFailEvent();

			// There are no more endpoints to try. Shut down the client.
			close();
		}
	}

	void _start_read()
	{
		clientsocket_.async_receive(boost::asio::buffer(read_buffer_, _BUF_SIZE),
			boost::bind(&CNetPeerSession::_handle_read,
			shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	void _start_write()
	{
		boost::recursive_mutex::scoped_lock autolock(mutex_);

		if( write_buffer_list_.empty() )
			return;

		boost::shared_ptr<CNetPacketData> packet = write_buffer_list_.front();

		int readSize = packet->buffer().read( curr_write_buffer_, _BUF_SIZE );
		assert( readSize > 0 );

		boost::asio::async_write(clientsocket_,
			boost::asio::buffer(curr_write_buffer_, readSize),
			boost::bind(&CNetPeerSession::_handle_write,
			shared_from_this(),
			boost::asio::placeholders::error));
	}

	void _handle_connect(const boost::system::error_code& ec,
		tcp::resolver::iterator endpoint_iter)
	{
		// The async_connect() function automatically opens the socket at the start
		// of the asynchronous operation. If the socket is closed at this time then
		// the timeout handler must have run first.
		if (!clientsocket_.is_open())
		{
			// timed out
			// Try the next available endpoint.
			_start_connect(++endpoint_iter);
		}

		// Check if the connect operation failed before the deadline expired.
		else if (ec)
		{
			// We need to close the socket used in the previous connection attempt
			// before starting a new one.
			close();

			// Try the next available endpoint.
			_start_connect(++endpoint_iter);
		}

		// Otherwise we have successfully established a connection.
		else
		{
			// Success!
			start();
		}
	}

	void _handle_read( const boost::system::error_code& error, size_t bytes_transferred )
	{ 
		// the asynchronous read operation has now completed or failed and returned an error
		if( !error )
		{ 
			fireReceivedEvent( read_buffer_, bytes_transferred );

			// read completed, so process the data
			_start_read(); // start waiting for another asynchronous read again
		}
		else
			close();
	}

	void _handle_write( const boost::system::error_code& ec )
	{
		// the asynchronous read operation has now completed or failed and returned an error
		if(!ec)
		{
			boost::shared_ptr<CNetPacketData> packet = write_buffer_list_.front();

			fireSendingEvent( packet );

			mutex_.lock();
			if(packet->buffer().remainingSize() <= 0)
				write_buffer_list_.pop_front();
			
			// write completed, so send next write data
			if( !write_buffer_list_.empty() ) // if there is anthing left to be written
				_start_write(); // then start sending the next item in the buffer
			mutex_.unlock();
		}
		else
			close();
	}

	void _handle_check_deadline()
	{
		if( stopped_ )
			return;

		// Check whether the deadline has passed. We compare the deadline against
		// the current time since a new asynchronous operation may have moved the
		// deadline before this actor had a chance to run.
		if( deadline_.expires_at() <= deadline_timer::traits_type::now() )
		{
			// There is no longer an active deadline. The expiry is set to positive
			// infinity so that the actor takes no action until a new deadline is set.
			deadline_.expires_at( boost::posix_time::pos_infin );

			// The deadline has passed. The socket is closed so that any outstanding
			// asynchronous operations are cancelled.
			close();
			return;
		}
	}

private:
	void fireDisconnectedEvent( void )
	{
		if( evtTarget_ )
		{
			evtTarget_->onINetPeerSessionEvent_Disconnected( this );
		}
	}

	void fireConnectFailEvent( void )
	{
		if( evtTarget_ )
		{
			evtTarget_->onINetPeerSessionEvent_ConnectFailed( this );
		}
	}

	void fireConnectSuccessEvent( void )
	{
		if( evtTarget_ )
		{
			evtTarget_->onINetPeerSessionEvent_Connected( this );
		}
	}

	void fireReceivedEvent( const char * buffer, size_t len )
	{
		if( evtTarget_ )
		{
			std::string tempbuffer( buffer, len );
			evtTarget_->onINetPeerSessionEvent_Received( this, tempbuffer );
		}
	}

	void fireSendingEvent( boost::shared_ptr<CNetPacketData> packet )
	{
		if( evtTarget_ )
		{
			evtTarget_->onINetPeerSessionEvent_Sending( this, packet );
		}
	}

private:
	static const int _BUF_SIZE = 4096;

	boost::asio::io_service& io_service_;
	int sessionId_;
	bool stopped_;
	bool connected_;
	INetPeerSessionEvent * evtTarget_;

	boost::asio::ip::tcp::socket clientsocket_;
	boost::asio::deadline_timer deadline_;

	char read_buffer_[_BUF_SIZE];
	std::deque< boost::shared_ptr<CNetPacketData> > write_buffer_list_;

	char curr_write_buffer_[_BUF_SIZE];
	boost::recursive_mutex mutex_;
};
