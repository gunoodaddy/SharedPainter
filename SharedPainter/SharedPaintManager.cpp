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

#include "StdAfx.h"
#include "SharedPaintManager.h"
#include <QHostInfo>

#define START_SERVER_PORT                       4001
#define DEFAULT_BROADCAST_PORT                  3336
#define DEFAULT_BROADCAST_UDP_PORT_FOR_TEXTMSG  3338
#define START_UDP_LISTEN_PORT	                5001

#define	TIMEOUT_SYNC_MSEC	5000

static bool isCompatibleVersion( const std::string &version )
{
	int major1, minor1, revision1;	// mine
	int major2, minor2, revision2;	// yours

	assert( Util::parseVersionString( VERSION_TEXT, major1, minor1, revision1 ) );
	
	if( ! Util::parseVersionString( version, major2, minor2, revision2 ) )
		return false;

	if( major1 != major2 )
		return false;
	if( minor1 != minor2 )
		return false;

	return true;
}


CSharedPaintManager::CSharedPaintManager( void ) : enabled_(true), syncStartedFlag_(false), commandMngr_(this), canvas_(NULL), listenTcpPort_(-1), listenUdpPort_(-1), findingServerMode_(false)
, lastWindowWidth_(0), lastWindowHeight_(0), gridLineSize_(0)
, lastPacketId_(-1)
{
	// create my user info
	struct SPaintUserInfoData data;
	data.userId =  Util::generateMyId();

	std::string myIp = Util::getMyIPAddress();
	myUserInfo_ = boost::shared_ptr<CPaintUser>(new CPaintUser);
	myUserInfo_->setLocalIPAddress( myIp );
	myUserInfo_->setData( data );

	backgroundColor_ = Qt::white;

	broadCastSessionForSendMessage_ = boost::shared_ptr< CNetBroadCastSession >(new CNetBroadCastSession( netRunner_.io_service() ));
	broadCastSessionForSendMessage_->setEvent( this );
	broadCastSessionForSendMessage_->openUdp();

	broadCastSessionForRecvMessage_ = boost::shared_ptr< CNetBroadCastSession >(new CNetBroadCastSession( netRunner_.io_service() ));
	broadCastSessionForRecvMessage_->setEvent( this );
	if( broadCastSessionForRecvMessage_->listenUdp( DEFAULT_BROADCAST_UDP_PORT_FOR_TEXTMSG ) == false )
	{
		// ignore this error.. by multi programs on a computer
	}

	clearAllUsers(); // clear & add my user info

	startListenBroadCast();
}

CSharedPaintManager::~CSharedPaintManager( void )
{
	stopServer();

	if( broadCastSessionForSendMessage_ )
		broadCastSessionForSendMessage_->close();
	if( broadCastSessionForRecvMessage_ )
		broadCastSessionForRecvMessage_->close();

	close();

	stopListenBroadCast();

	netRunner_.close();
}

void CSharedPaintManager::onTimeoutSyncStart( void )
{
	if( syncStartedFlag_ == false )
		close();
}

void CSharedPaintManager::changeNickName( const std::string & nickName )
{
	myUserInfo_->setNickName( nickName );

	std::string msg = SystemPacketBuilder::CChangeNickName::make( myUserInfo_->userId(), myUserInfo_->nickName() );
	sendDataToUsers( msg );
}

void CSharedPaintManager::setPaintChannel( const std::string & channel )
{
	std::string myIp = Util::getMyIPAddress();
	std::string broadCastMsg = BroadCastPacketBuilder::CProbeServer::make( channel, myIp, listenUdpPort_ );
	
	if( broadCastSessionForFinder_ )
		broadCastSessionForFinder_->setBroadCastMessage( broadCastMsg );
	
	myUserInfo_->setChannel( channel );
	myUserInfo_->setLocalIPAddress( myIp );
}

void CSharedPaintManager::sendChatMessage( const std::string &msg )
{
	std::string data;
	data = SystemPacketBuilder::CChatMessage::make( myUserInfo_->userId(), myUserInfo_->nickName(), msg );
	sendDataToUsers( data );

	caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ReceivedChatMessage, this, myUserInfo_->userId(), myUserInfo_->nickName(), msg ) );
	
}

void CSharedPaintManager::sendBroadCastTextMessage( const std::string &paintChannel, const std::string &msg )
{
	std::string data;
	data = BroadCastPacketBuilder::CTextMessage::make( paintChannel, myUserInfo_->userId(), msg );

	broadCastSessionForSendMessage_->sendData( DEFAULT_BROADCAST_UDP_PORT_FOR_TEXTMSG, data );

	caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ReceivedBroadcastTextMessage, this, paintChannel, myUserInfo_->userId(), msg ) );
}

bool CSharedPaintManager::startListenBroadCast( void )
{
	if( broadCastSessionForListener_ )
		broadCastSessionForListener_->close();
	broadCastSessionForListener_ = boost::shared_ptr< CNetBroadCastSession >(new CNetBroadCastSession( netRunner_.io_service() ));
	broadCastSessionForListener_->setEvent( this );

	if( !broadCastSessionForListener_->listenUdp( DEFAULT_BROADCAST_PORT ) )
	{
		qDebug() << "startListenBroadCast : fail" << DEFAULT_BROADCAST_PORT;
		return false;
	}
	qDebug() << "startListenBroadCast : ok" << DEFAULT_BROADCAST_PORT;
	return true;
}

bool CSharedPaintManager::startFindingServer( void )
{
	stopListenBroadCast();
	_stopFindingServer();

	// all previous connections are need to close.
	clearAllUsers();
	clearAllSessions();

	// for receiving server info
	if( udpSessionForConnection_ )
		udpSessionForConnection_->close();
	udpSessionForConnection_ = boost::shared_ptr< CNetUdpSession >(new CNetUdpSession( netRunner_.io_service() ));
	udpSessionForConnection_->setEvent(this);

	listenUdpPort_ = START_UDP_LISTEN_PORT;
	while( true )
	{
		if( udpSessionForConnection_->listen( listenUdpPort_ ) )
			break;
		listenUdpPort_++;
	}

	// broadcast for finding server
	std::string myIp = Util::getMyIPAddress();
	std::string broadCastMsg = BroadCastPacketBuilder::CProbeServer::make( myUserInfo_->channel(), myIp, listenUdpPort_ );

	if( broadCastSessionForFinder_ )
		broadCastSessionForFinder_->close();
	broadCastSessionForFinder_ = boost::shared_ptr< CNetBroadCastSession >(new CNetBroadCastSession( netRunner_.io_service() ));
	broadCastSessionForFinder_->setEvent( this );
	broadCastSessionForFinder_->startSend( DEFAULT_BROADCAST_PORT, broadCastMsg, 3 );

	qDebug() << "startFindingServer" << listenUdpPort_;
	findingServerMode_ = true;
	return true;
}


bool CSharedPaintManager::startServer( int port )
{
	if( netPeerServer_ )	// already started..
		return true;

	stopServer();

	if( port <= 0 )
		port = START_SERVER_PORT;

	netPeerServer_ = boost::shared_ptr<CNetPeerServer>(new CNetPeerServer( netRunner_.io_service() ));
	netPeerServer_->setEvent( this );

	for( int port = START_SERVER_PORT; port < START_SERVER_PORT + 100; port++ )
	{
		if( netPeerServer_->start( port ) )
		{
			listenTcpPort_ = port;
			break;
		}
	}
	assert( listenTcpPort_ > 0 );
	
	myUserInfo_->setListenTcpPort( listenTcpPort_ );

	qDebug() << "startServer" << listenTcpPort_;
	return true;
}

void CSharedPaintManager::stopListenBroadCast( void )
{
	qDebug() << "stopListenBroadCast";

	if( broadCastSessionForListener_ )
		broadCastSessionForListener_->close();
}

void CSharedPaintManager::_stopFindingServer( void )
{
	qDebug() << "_stopFindingServer" << findingServerMode_;

	if( ! findingServerMode_ )
		return;

	if( broadCastSessionForFinder_ )
		broadCastSessionForFinder_->close();

	if( udpSessionForConnection_ )
		udpSessionForConnection_->close();

	findingServerMode_ = false;
}


void CSharedPaintManager::stopFindingServer( void )
{
	_stopFindingServer();
	startListenBroadCast();
}

void CSharedPaintManager::stopServer( void )
{
	if( netPeerServer_ )
		netPeerServer_->close();

	netPeerServer_ = boost::shared_ptr<CNetPeerServer>();
}

bool CSharedPaintManager::deserializeData( const char * data, size_t size )
{
	CPacketSlicer slicer;
	slicer.addBuffer( data, size );

	if( slicer.parse() == false )
	{
		return false;
	}

	boost::shared_ptr<CPacketData> packetData = slicer.parsedItem( 0 );
	if( packetData->code != CODE_SYSTEM_VERSION_INFO )
	{
		return false;
	}

	std::string version;
	if( ! SystemPacketBuilder::CVersionInfo::parse( packetData->body, version ) )
	{
		return false;
	}

	if( ! isCompatibleVersion( version ) )
	{
		return false;
	}

	// must start from 1 index..
	for( size_t i = 1; i < slicer.parsedItemCount(); i++ )
	{
		boost::shared_ptr<CPacketData> data = slicer.parsedItem( i );
		dispatchPaintPacket( NULL, data );
	}

	std::string allData(data, size);
	sendDataToUsers( allData );

	return true;
}


std::string CSharedPaintManager::serializeData( const std::string *target )
{
	std::string allData;

	allData += SystemPacketBuilder::CVersionInfo::make( VERSION_TEXT );

	// Window Size
	allData += WindowPacketBuilder::CResizeMainWindow::make( lastWindowWidth_, lastWindowHeight_, target );

	// Window Splitter Sizes
	allData += WindowPacketBuilder::CResizeWindowSplitter::make( lastWindowSplitterSizes_, target );

	// Background Grid Line
	if( gridLineSize_ > 0 )
		allData += PaintPacketBuilder::CSetBackgroundGridLine::make( gridLineSize_, target );

	// Background Color
	if( backgroundColor_ != Qt::white )
		allData += PaintPacketBuilder::CSetBackgroundColor::make( backgroundColor_.red(), backgroundColor_.green(), backgroundColor_.blue(), backgroundColor_.alpha(), target );

	// Background Image
	if( backgroundImageItem_ )
		allData += PaintPacketBuilder::CSetBackgroundImage::make( backgroundImageItem_, target );

	// History all paint item
	commandMngr_.lock();
	size_t itemSize = 0;
	const ITEM_SET &set = commandMngr_.historyItemSet();
	ITEM_SET::const_iterator itItem = set.begin();
	for( ; itItem != set.end(); itItem++ )
	{
		std::string msg = PaintPacketBuilder::CCreateItem::make( *itItem, target );
		allData += msg;
		itemSize += msg.size();
	}
	commandMngr_.unlock();

	// History all task
	commandMngr_.lock();
	size_t taskSize = 0;
	const TASK_ARRAY &taskList = commandMngr_.historyTaskList();
	TASK_ARRAY::const_iterator itTask = taskList.begin();
	for( ; itTask != taskList.end(); itTask++ )
	{
		std::string msg = TaskPacketBuilder::CExecuteTask::make( boost::const_pointer_cast<CSharedPaintTask>(*itTask), target );
		allData += msg;
		taskSize += msg.size();
	}
	commandMngr_.unlock();

	qDebug() << "SharedPaintManager::serializeData() size = " << allData.size() << ", task size = " << taskSize << ", item size = " << itemSize;
	return allData;
}

void CSharedPaintManager::_requestSyncData( void )
{
	if( syncStartedFlag_ )
		return;

	QTimer::singleShot( TIMEOUT_SYNC_MSEC, this, SLOT(onTimeoutSyncStart()) );

	std::string msg = SystemPacketBuilder::CSyncRequest::make();
	if( relayServerSession_ )
		relayServerSession_->session()->sendData( msg );
}

// this function need to check session pointer null check!
void CSharedPaintManager::dispatchPaintPacket( CPaintSession * session, boost::shared_ptr<CPacketData> packetData )
{
	switch( packetData->code )
	{
	case CODE_SYSTEM_VERSION_INFO:
		{
			bool error = false;
			std::string version;
			if( SystemPacketBuilder::CVersionInfo::parse( packetData->body, version ) )
			{
				if( ! isCompatibleVersion( version ) )
				{
					error = true;
				}
			}
			else
			{
				error = true;
			}

			if( error )
			{
				qDebug() << "Version invalied.. not compatible with target user..";
				caller_.performMainThread( boost::bind( &CSharedPaintManager::close, this ) );
			}
		}
		break;
	case CODE_SYSTEM_CHANGE_NICKNAME:
		{
			std::string userid, nickname;
			if( SystemPacketBuilder::CChangeNickName::parse( packetData->body,  userid, nickname ) )
			{
				boost::shared_ptr<CPaintUser> joiner = findUser( userid );
				if( joiner )
				{
					std::string prevNickName = joiner->nickName();
					joiner->setNickName( nickname );
					caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ChangedNickName, this, userid, prevNickName, nickname ) );
				}
			}
		}
		break;
	case CODE_SYSTEM_JOIN_TO_SERVER:
		{
			boost::shared_ptr<CPaintUser> user = SystemPacketBuilder::CJoinToServer::parse( packetData->body );
			
			assert( isRelayServerSession( session ) );
		
			addUser( user );
		}
		break;
	case CODE_SYSTEM_JOIN_TO_SUPERPEER:
		{
			boost::shared_ptr<CPaintUser> user = SystemPacketBuilder::CJoinerToSuperPeer::parse( packetData->body );

			if( user )
			{
				if( isAlwaysP2PMode() )
				{
					addUser( user );
				}

				boost::shared_ptr<CPaintUser> joiner = findUser( user->userId() );
				if( joiner )
					joiner->setSessionId( session->sessionId() );
			}
		}
		break;
	case CODE_SYSTEM_RES_JOIN:
		{
			bool connectSuperPeerFlag = false;

			std::string channel, superId;
			USER_LIST list;
			if( SystemPacketBuilder::CResponseJoin::parse( packetData->body, channel, list, superId ) )
			{
				for( size_t i = 0; i < list.size(); i++ )
					addUser( list[i] );

				if( superId != myUserInfo_->userId() )
				{
					boost::shared_ptr<CPaintUser> user = findUser( superId );
					if( user )
					{
						connectToSuperPeer( user );
						connectSuperPeerFlag = true;
					}
				}
			}

			bool firstUserFlag = false;
			if( joinerMap_.size() == 1 )
				firstUserFlag = true;

			if( false == firstUserFlag 
				&& false == connectSuperPeerFlag 
				&& relayServerSession_ )
			{
				_requestSyncData();
			}
		}
		break;
	case CODE_SYSTEM_CHANGE_SUPERPEER:
		{
			std::string userid;
			if( SystemPacketBuilder::CChangeSuperPeer::parse( packetData->body, userid ) )
			{
				if( userid != myUserInfo_->userId() )
				{
					boost::shared_ptr<CPaintUser> user = findUser( userid );
					if( user )
					{
						connectToSuperPeer( user );
					}
				}
				superPeerId_ = userid;
			}
		}
		break;
	case CODE_SYSTEM_TCPSYN:
		{
			assert( relayServerSession_ );
			std::string channel;
			if( SystemPacketBuilder::CTcpSyn::parse( packetData->body ) )
			{
				// SEND ACK PACKET TO SERVER
				std::string msg = SystemPacketBuilder::CTcpAck::make();
				relayServerSession_->session()->sendData( msg );
			}
		}
		break;
	case CODE_SYSTEM_SYNC_START:
		{
			std::string channel;
			if( SystemPacketBuilder::CSyncStart::parse( packetData->body, channel ) )
			{
				syncStartedFlag_ = true;
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_SyncStart, this ) );
			}
		}
		break;
	case CODE_SYSTEM_SYNC_COMPLETE:
		{
			std::string channel;
			if( SystemPacketBuilder::CSyncComplete::parse( packetData->body ) )
			{
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_SyncComplete, this ) );
			}
		}
		break;
	case CODE_SYSTEM_SYNC_REQUEST:
		{
			std::string channel, target;
			if( SystemPacketBuilder::CSyncRequest::parse( packetData->body, channel, target ) )
			{
				std::string packetPackage;
				packetPackage += SystemPacketBuilder::CSyncStart::make( channel, myUserInfo_->userId(), target );
				packetPackage += serializeData( &target );
				packetPackage += SystemPacketBuilder::CSyncComplete::make( target );

				qDebug() << "CODE_SYSTEM_SYNC_REQUEST" << packetPackage.size() << target.c_str() << joinerMap_.size();

				if( isMySelfSuperPeer() )
				{
					// I'm superpeer
					assert( superPeerSession_ == NULL && sessionList_.size() > 0 );

					boost::shared_ptr<CPaintUser> user = findUser( target );
					if( ! user )
					{
						qDebug() << "CODE_SYSTEM_SYNC_REQUEST : NOT FOUND JOINER" << joinerMap_.size();
						break; // exception case.. ignore it..
					}

					sendDataToUsers( packetPackage, user->sessionId() ); 
				}
				else
				{
					// I'm normal user but be forced to do sync for you by server..
					assert( relayServerSession_ );
					relayServerSession_->session()->sendData( packetPackage );
				}
			}
		}
		break;
	case CODE_SYSTEM_LEFT:
		{
			std::string userId, channel;
			if( SystemPacketBuilder::CLeftUser::parse( packetData->body, channel, userId ) )
			{
				removeUser( userId );
			}
		}
		break;
	case CODE_SYSTEM_CHAT_MESSAGE:
		{
			std::string userId, nickName, msg;
			if( SystemPacketBuilder::CChatMessage::parse( packetData->body, userId, nickName, msg ) )
			{
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ReceivedChatMessage, this, userId, nickName, msg ) );
			}
		}
		break;
	case CODE_PAINT_CLEAR_SCREEN:
		{
			PaintPacketBuilder::CClearScreen::parse( packetData->body );	// nothing to do..
			caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ClearScreen, this ) );
		}
		break;
	case CODE_PAINT_CLEAR_BG:
		{
			PaintPacketBuilder::CClearScreen::parse( packetData->body );	// nothing to do..
			caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ClearBackground, this ) );
		}
		break;
	case CODE_PAINT_SET_BG_IMAGE:
		{
			boost::shared_ptr<CBackgroundImageItem> image = PaintPacketBuilder::CSetBackgroundImage::parse( packetData->body );
			caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_SetBackgroundImage, this, image ) );
		}
		break;
	case CODE_PAINT_SET_BG_GRID_LINE:
		{
			int size;
			if( PaintPacketBuilder::CSetBackgroundGridLine::parse( packetData->body, size ) )
			{
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_SetBackgroundGridLine, this, size ) );
			}
		}
		break;
	case CODE_PAINT_SET_BG_COLOR:
		{
			int r, g, b, a;
			if( PaintPacketBuilder::CSetBackgroundColor::parse( packetData->body, r, g, b, a ) )
			{
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_SetBackgroundColor, this, r, g, b, a ) );
			}
		}
		break;	
	case CODE_PAINT_CREATE_ITEM:
		{
			std::string owner;
			int itemId;
			boost::shared_ptr<CPaintItem> item = PaintPacketBuilder::CCreateItem::parse( packetData->body );
			if( item )
			{
				commandMngr_.addHistoryItem( item );
			}
		}
		break;
	case CODE_TASK_EXECUTE:
		{
			std::string owner;
			int itemId;
			boost::shared_ptr<CSharedPaintTask> task = TaskPacketBuilder::CExecuteTask::parse( packetData->body );
			if( task )
			{
				task->setSharedPaintManager( this );

				commandMngr_.executeTask( task, false );
			}
		}
		break;
	case CODE_WINDOW_RESIZE_MAIN_WND:
		{
			std::string owner;
			int width, height;
			if( WindowPacketBuilder::CResizeMainWindow::parse( packetData->body, width, height ) )
			{
				if( width <= 0 || height <= 0 )
					return;
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ResizeMainWindow, this, width, height ) );
			}
		}
		break;
	case CODE_WINDOW_RESIZE_WND_SPLITTER:
		{
			std::vector<int> sizes;
			if( WindowPacketBuilder::CResizeWindowSplitter::parse( packetData->body, sizes ) )
			{
				if( sizes.size() <= 0 )
					return;
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ResizeWindowSplitter, this, sizes ) );
			}
		}
		break;
	}
}


void CSharedPaintManager::dispatchUdpPacket( CNetUdpSession *session, boost::shared_ptr<CPacketData> packetData )
{
	switch( packetData->code )
	{
	case CODE_UDP_SERVER_INFO:
		{
			std::string addr, paintChannel;
			int port;
			if( UdpPacketBuilder::CServerInfo::parse( packetData->body, paintChannel, addr, port ) )
			{
				qDebug() << "CODE_UDP_SERVER_INFO : " << paintChannel.c_str() << addr.c_str() << port;
				if( myUserInfo_->channel() != paintChannel )
					return;

				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_GetServerInfo, this, paintChannel, addr, port ) );
			}
		}
		break;
	}
}

void CSharedPaintManager::dispatchBroadCastPacket( CNetBroadCastSession *session, boost::shared_ptr<CPacketData> packetData )
{
	switch( packetData->code )
	{
	case CODE_BROAD_PROBE_SERVER:
		{
			std::string addr;
			int port;
			std::string paintChannel;
			if( BroadCastPacketBuilder::CProbeServer::parse( packetData->body, paintChannel, addr, port ) )
			{
				qDebug() << "CODE_BROAD_PROBE_SERVER : " << myUserInfo_->channel().c_str() << paintChannel.c_str() << addr.c_str() << port;
				if( myUserInfo_->channel() != paintChannel )
					return;

				// make server info
				std::string myIp = Util::getMyIPAddress();
				std::string broadCastMsg = UdpPacketBuilder::CServerInfo::make( myUserInfo_->channel(), myIp, listenTcpPort_ );

				if( udpSessionForConnection_ )
					udpSessionForConnection_->close();

				udpSessionForConnection_ = boost::shared_ptr< CNetUdpSession >(new CNetUdpSession( netRunner_.io_service() ));
				udpSessionForConnection_->sendData( addr, port, broadCastMsg );
			}
		};

	case CODE_BROAD_TEXT_MESSAGE:
		{
			std::string message, paintChannel, fromId;
			if( BroadCastPacketBuilder::CTextMessage::parse( packetData->body, paintChannel, fromId, message ) )
			{
				if( myUserInfo_->channel() != paintChannel )
					return;

				if( myUserInfo_->userId() == fromId )	// ignore myself message..
					return;
	
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ReceivedBroadcastTextMessage, this, paintChannel, fromId, message ) );
			}
		}
		break;
	}
}
