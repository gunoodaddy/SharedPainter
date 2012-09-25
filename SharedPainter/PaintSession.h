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

#include <boost/enable_shared_from_this.hpp>
#include "PacketSlicer.h"
#include "NetPeerSession.h"

class CPaintSession;

class IPaintSessionEvent
{
public:
	virtual void onIPaintSessionEvent_Connected( CPaintSession * session ) = 0;
	virtual void onIPaintSessionEvent_ConnectFailed( CPaintSession * session ) = 0;
	virtual void onIPaintSessionEvent_ReceivedPacket( CPaintSession * session, const boost::shared_ptr<CPacketData> data ) = 0;
	virtual void onIPaintSessionEvent_SendingPacket( CPaintSession * session, const boost::shared_ptr<CNetPacketData> data ) = 0;
	virtual void onIPaintSessionEvent_Disconnected( CPaintSession* session ) = 0;
};

class CPaintSession : public boost::enable_shared_from_this<CPaintSession>, INetPeerSessionEvent
{
public:
	CPaintSession( boost::shared_ptr<CNetPeerSession> session, IPaintSessionEvent *evt ) : session_(session), evtTarget_(evt)
	{
		session_->setEvent( this );
		qDebug() << "CPaintSession(void) " << this;
	}
	
	~CPaintSession(void) 
	{
		qDebug() << "~CPaintSession(void) start" << this;
		session_->close();
		session_->setEvent( NULL );
		qDebug() << "~CPaintSession(void) end" << this;
	}

	int sessionId( void )
	{
		return session_->sessionId();
	}

	void close( void )
	{
		session_->close();
	}

	boost::shared_ptr<CNetPeerSession> session( void )
	{
		return session_;
	}

	virtual void onINetPeerSessionEvent_Connected( CNetPeerSession *session )
	{
		if( evtTarget_ )
			evtTarget_->onIPaintSessionEvent_Connected( this );
	}
	virtual void onINetPeerSessionEvent_ConnectFailed( CNetPeerSession *session )
	{
		if( evtTarget_ )
			evtTarget_->onIPaintSessionEvent_ConnectFailed( this );
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
				evtTarget_->onIPaintSessionEvent_ReceivedPacket( this, data );
		}
	}
	virtual void onINetPeerSessionEvent_Disconnected( CNetPeerSession *session )
	{
		if( evtTarget_ )
			evtTarget_->onIPaintSessionEvent_Disconnected( this );
	}
	virtual void onINetPeerSessionEvent_Sending( CNetPeerSession *session, boost::shared_ptr<CNetPacketData> packet )
	{
		if( evtTarget_ )
			evtTarget_->onIPaintSessionEvent_SendingPacket( this, packet );
	}

private:
	IPaintSessionEvent *evtTarget_;
	boost::shared_ptr<CNetPeerSession> session_;
	CPacketSlicer packetSlicer_;

	std::deque< boost::shared_ptr<CNetPacketData> > packetList_;
};
