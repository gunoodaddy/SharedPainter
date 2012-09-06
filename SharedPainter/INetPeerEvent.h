#pragma once

class CNetPeerSession;
class CNetBroadCastSession;
class CNetPeerServer;

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

class INetBroadCastSessionEvent
{
public:
	virtual void onINetBroadCastSessionEvent_BroadCastReceived( CNetBroadCastSession *session, const std::string buffer ) = 0;
};