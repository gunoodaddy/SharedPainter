#pragma once

#include "INetPeerEvent.h"

class CNetBroadCastSession : public boost::enable_shared_from_this<CNetBroadCastSession>
{
public:
	static const int DEFAULT_SEND_SEC = 3;

	CNetBroadCastSession( boost::asio::io_service& io_service ) 
		: io_service_(io_service), socket_(io_service), evtTarget_(NULL)
		, broadCastPort_(0), sendMsgSecond_(DEFAULT_SEND_SEC), sentCount_(0)
		, stopBroadCastMsgFlag_(true), broadcast_timer_(io_service)
	{
		qDebug() << "CNetBroadCastSession" << this;
	}

	~CNetBroadCastSession(void)
	{
		qDebug() << "~CNetBroadCastSession" << this;
		close();
	}

	void setEvent( INetBroadCastSessionEvent * evt )
	{
		evtTarget_ = evt;
	}

	void setBroadCastMessage( const std::string &msg )
	{
		broadCastMsg_ = msg;
	}

	void sendData( int port, const std::string &data )
	{
		boost::asio::ip::udp::endpoint senderEndpoint( boost::asio::ip::address_v4::broadcast(), port ); 
		
		if( socket_.is_open() )
		{
			socket_.send_to( boost::asio::buffer(data), senderEndpoint); 
		}
	}

	bool openUdp( void )
	{
		if( socket_.is_open() )
			return false;

		boost::system::error_code error; 
		socket_.open( boost::asio::ip::udp::v4(), error ); 

		if (!error) 
		{ 
			socket_.set_option( boost::asio::ip::udp::socket::reuse_address(true) ); 
			socket_.set_option( boost::asio::socket_base::broadcast(true) ); 
			return true;
		} 
	
		return false;
	}

	bool listenUdp( int port )
	{
		try
		{
			if( !openUdp() )
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
		stopBroadCastMsgFlag_ = true;
		broadcast_timer_.cancel();
	}

	bool startSend( int port, const std::string &msg, int second = 0 )
	{
		broadCastPort_ = port;

		if( socket_.is_open() || openUdp() )
		{
			stopBroadCastMsgFlag_ = false;

			setBroadCastMessage(msg);

			if( second <= 0 )
				second = DEFAULT_SEND_SEC;

			_start_broadcast_timer( second );
			return true;
		} 

		return false;
	}

	void pauseSend( void )
	{
		sentCount_ = 0;
		stopBroadCastMsgFlag_ = true;
	}

	void resumeSend( void )
	{
		startSend( broadCastPort_, broadCastMsg_, sendMsgSecond_ );
	}

	void _start_receive_from( void )
	{
		socket_.async_receive_from( 
			boost::asio::buffer(read_buffer_, _BUF_SIZE), sender_endpoint_, 
			boost::bind(&CNetBroadCastSession::_handle_receive_from, shared_from_this(), 
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

	void _start_broadcast_timer( int second )
	{
		sendMsgSecond_ = second;

		broadcast_timer_.expires_from_now(boost::posix_time::seconds(second));
		broadcast_timer_.async_wait( boost::bind(&CNetBroadCastSession::_handle_broadcast_timer, shared_from_this()) );
	}

	void _handle_broadcast_timer( void )
	{
		if( stopBroadCastMsgFlag_ )
			return;

		if( broadcast_timer_.expires_at() <= deadline_timer::traits_type::now() )
		{
			broadcast_timer_.expires_at( boost::posix_time::pos_infin );

			sendData( broadCastPort_, broadCastMsg_ );
			sentCount_++;
			fireSentMessageEvent();
		}

		_start_broadcast_timer( sendMsgSecond_ );
	}


private:
	void fireReceiveEvent( const char * buffer, size_t len )
	{
		if( evtTarget_ )
		{
			std::string tempbuffer( buffer, len );
			evtTarget_->onINetBroadCastSessionEvent_BroadCastReceived( this, tempbuffer );
		}
	}

	void fireSentMessageEvent( void )
	{
		if( evtTarget_ )
		{
			evtTarget_->onINetBroadCastSessionEvent_SentMessage( this, sentCount_ );
		}
	}

private:
	static const int _BUF_SIZE = 4096;
	boost::asio::io_service& io_service_;

	bool stopBroadCastMsgFlag_;
	boost::asio::deadline_timer broadcast_timer_;
	int broadCastPort_;
	std::string broadCastMsg_;
	int sendMsgSecond_;
	int sentCount_;

	INetBroadCastSessionEvent *evtTarget_;
	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint sender_endpoint_;
	char read_buffer_[_BUF_SIZE];
};
