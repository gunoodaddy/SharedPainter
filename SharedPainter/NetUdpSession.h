#pragma once

#include "INetPeerEvent.h"

class CNetUdpSession : public boost::enable_shared_from_this<CNetUdpSession>
{
public:
	CNetUdpSession( boost::asio::io_service& io_service ) 
		: io_service_(io_service), socket_(io_service), evtTarget_(NULL)
	{
		qDebug() << "CNetUdpSession" << this;
	}

	~CNetUdpSession(void)
	{
		qDebug() << "~CNetUdpSession" << this;
		close();
	}

	void setEvent( INetUdpSessionEvent * evt )
	{
		evtTarget_ = evt;
	}

	void sendData( int port, const std::string &data )
	{
		boost::asio::ip::udp::endpoint senderEndpoint( boost::asio::ip::address_v4::broadcast(), port ); 
		sendData( senderEndpoint, data );
	}

	void sendData( const std::string &addr, int port, const std::string &data )
	{
		char temp[100];
#if defined(Q_WS_WIN)
		 _itoa(port, temp, 10);
#else
		itoa(port, temp, 10);
#endif
		std::string portstr = temp;

		boost::asio::ip::udp::resolver resolver(io_service_);  
		boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), addr, portstr);  
		boost::asio::ip::udp::resolver::iterator itr = resolver.resolve(query); 
		boost::asio::ip::udp::endpoint endpoint = *itr;  
           
		sendData( endpoint, data );
	}

	void sendData( boost::asio::ip::udp::endpoint &endpoint, const std::string &data )
	{
		if( !socket_.is_open() )
			if( !open() )
				return;

		if( socket_.is_open() )
		{
			socket_.send_to( boost::asio::buffer(data), endpoint); 
		}
	}

	bool open( void )
	{
		if( socket_.is_open() )
			return false;

		boost::system::error_code error; 
		socket_.open( boost::asio::ip::udp::v4(), error ); 

		if (!error) 
		{ 
			socket_.set_option( boost::asio::ip::udp::socket::reuse_address(true) ); 
			return true;
		} 
	
		return false;
	}

	bool listen( int port )
	{
		try
		{
			if( !open() )
				return false;

			boost::asio::ip::udp::endpoint listen_endpoint( boost::asio::ip::address_v4::any(), port); 
			socket_.bind( listen_endpoint ); 
		}catch(...) 
		{
			return false;
		}

		_start_receive_from();
		return true;
	}

	void close( void )
	{
		socket_.close();
	}

	void _start_receive_from( void )
	{
		socket_.async_receive_from( 
			boost::asio::buffer(read_buffer_, _BUF_SIZE), sender_endpoint_, 
			boost::bind(&CNetUdpSession::_handle_receive_from, shared_from_this(), 
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred)); 
	}

	void _handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd) 
	{ 
		if( !error )
		{
			//last_sender_endpoint_ = sender_endpoint_;
			fireReceiveEvent( read_buffer_, bytes_recvd );
		}

		_start_receive_from();
	}

private:
	void fireReceiveEvent( const char * buffer, size_t len )
	{
		if( evtTarget_ )
		{
			std::string tempbuffer( buffer, len );
			evtTarget_->onINetUdpSessionEvent_Received( this, tempbuffer );
		}
	}

private:
	static const int _BUF_SIZE = 4096;
	boost::asio::io_service& io_service_;

	INetUdpSessionEvent *evtTarget_;
	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint sender_endpoint_;
	//boost::asio::ip::udp::endpoint last_sender_endpoint_;	// not thread safe
	char read_buffer_[_BUF_SIZE];
};
