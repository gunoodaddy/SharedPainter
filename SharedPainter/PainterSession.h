#pragma once

#include <boost/enable_shared_from_this.hpp>
#include "PacketSlicer.h"
#include "NetPeerSession.h"

class CPainterSession;

class IPainterSessionEvent
{
public:
	virtual void onIPainterSessionEvent_Connected( boost::shared_ptr<CPainterSession> session ) = 0;
	virtual void onIPainterSessionEvent_ConnectFailed( boost::shared_ptr<CPainterSession> session ) = 0;
	virtual void onIPainterSessionEvent_ReceivedPacket( boost::shared_ptr<CPainterSession> session, const boost::shared_ptr<CPacketData> data ) = 0;
	virtual void onIPainterSessionEvent_SendingPacket( boost::shared_ptr<CPainterSession> session, const boost::shared_ptr<CNetPacketData> data ) = 0;
	virtual void onIPainterSessionEvent_Disconnected( boost::shared_ptr<CPainterSession> session ) = 0;
};

class CPainterSession : public boost::enable_shared_from_this<CPainterSession>, INetPeerSessionEvent
{
public:
	CPainterSession( boost::shared_ptr<CNetPeerSession> session, IPainterSessionEvent *evt ) : session_(session), evtTarget_(evt)
	{
		session_->setEvent( this );
		qDebug() << "CPainterSession(void) " << this;
	}
	
	~CPainterSession(void) 
	{
		session_->setEvent( NULL );
		qDebug() << "~CPainterSession(void) " << this;
	}

	int sessionId( void )
	{
		return session_->sessionId();
	}

	boost::shared_ptr<CNetPeerSession> session( void )
	{
		return session_;
	}

	virtual void onINetPeerSessionEvent_Connected( CNetPeerSession *session )
	{
		if( evtTarget_ )
			evtTarget_->onIPainterSessionEvent_Connected( shared_from_this() );
	}
	virtual void onINetPeerSessionEvent_ConnectFailed( CNetPeerSession *session )
	{
		if( evtTarget_ )
			evtTarget_->onIPainterSessionEvent_ConnectFailed( shared_from_this() );
	}
	virtual void onINetPeerSessionEvent_Received( CNetPeerSession *session, const std::string buffer )
	{
		packetSlicer_.addBuffer( buffer );

		if( packetSlicer_.parse() == false )
			return;

		for( size_t i = 0; i < packetSlicer_.parsedItemCount(); i++ )
		{
			boost::shared_ptr<CPacketData> data = packetSlicer_.parsedItem( i );

			if( evtTarget_ )
				evtTarget_->onIPainterSessionEvent_ReceivedPacket( shared_from_this(), data );
		}
	}
	virtual void onINetPeerSessionEvent_Disconnected( CNetPeerSession *session )
	{
		if( evtTarget_ )
			evtTarget_->onIPainterSessionEvent_Disconnected( shared_from_this() );
	}
	virtual void onINetPeerSessionEvent_Sending( CNetPeerSession *session, boost::shared_ptr<CNetPacketData> packet )
	{
		if( evtTarget_ )
			evtTarget_->onIPainterSessionEvent_SendingPacket( shared_from_this(), packet );
	}

private:
	IPainterSessionEvent *evtTarget_;
	boost::shared_ptr<CNetPeerSession> session_;
	CPacketSlicer packetSlicer_;

	std::deque< boost::shared_ptr<CNetPacketData> > packetList_;
};
