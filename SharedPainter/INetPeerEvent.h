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

class CNetPeerSession;
class CNetBroadCastSession;
class CNetPeerServer;
class CNetUdpSession;

class INetPeerServerEvent
{
public:
	virtual void onINetPeerServerEvent_Accepted( boost::shared_ptr<CNetPeerServer> server, boost::shared_ptr<CNetPeerSession> session ) = 0;
};

class INetPeerSessionEvent
{
public:
	virtual void onINetPeerSessionEvent_Connected( CNetPeerSession *session ) = 0;
	virtual void onINetPeerSessionEvent_ConnectFailed( CNetPeerSession *session ) = 0;
	virtual void onINetPeerSessionEvent_Received( CNetPeerSession *session, const std::string buffer ) = 0;
	virtual void onINetPeerSessionEvent_Sending( CNetPeerSession *session, boost::shared_ptr<CNetPacketData> packet ) = 0;
	virtual void onINetPeerSessionEvent_Disconnected( CNetPeerSession *session ) = 0;
};

class INetUdpSessionEvent
{
public:
	virtual void onINetUdpSessionEvent_Received( CNetUdpSession *session, const std::string buffer ) = 0;
};

class INetBroadCastSessionEvent
{
public:
	virtual void onINetBroadCastSessionEvent_SentMessage( CNetBroadCastSession *session, int sentCount ) = 0;
	virtual void onINetBroadCastSessionEvent_BroadCastReceived( CNetBroadCastSession *session, const std::string buffer ) = 0;
};