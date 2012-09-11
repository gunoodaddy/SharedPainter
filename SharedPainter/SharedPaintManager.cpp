#include "StdAfx.h"
#include "SharedPaintManager.h"
#include <QHostInfo>

#define START_SERVER_PORT		4001
#define DEFAULT_BROADCAST_PORT	3336

static std::string generateMyId( void )
{
	std::string id;

	foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces()) 
	{ 
		// Return only the first non-loopback MAC Address 
		if (!(interface.flags() & QNetworkInterface::IsLoopBack)) 
		{
			id = interface.hardwareAddress().toStdString(); 
			break;
		}
	} 
	qint64 msecs = QDateTime::currentMSecsSinceEpoch();
	QString temp = QString::number( msecs );
	id += temp.toStdString();
	return id;
}

static std::string getMyIPAddress( void )
{
	std::string ip;

	foreach(QHostAddress address, QNetworkInterface::allAddresses()) 
	{ 
		QString qip = address.toString();	
		QString qscoped = address.scopeId();	
		QAbstractSocket::NetworkLayerProtocol protocol = address.protocol();
		if( protocol == QAbstractSocket::IPv4Protocol )
		{
			ip = qip.toStdString();
			return ip;
		}
	} 

	return ip;
}

CSharedPaintManager::CSharedPaintManager(void) : canvas_(NULL), acceptPort_(-1), serverMode_(false)
, lastWindowWidth_(0), lastWindowHeight_(0), gridLineSize_(0)
, lastPacketId_(-1)
{
	// default generate my id
	myId_ = generateMyId();

	// create my user info
	struct SPaintUserInfoData data;
	data.userId = myId_;

	myUserInfo_ = boost::shared_ptr<CPaintUser>(new CPaintUser);
	myUserInfo_->setData( data );

	backgroundColor_ = Qt::white;

	broadCastSessionForSendMessage_ = boost::shared_ptr< CNetBroadCastSession >(new CNetBroadCastSession( netRunner_.io_service() ));
	broadCastSessionForSendMessage_->setEvent( this );
	broadCastSessionForSendMessage_->openUdp();

	broadCastSessionForRecvMessage_ = boost::shared_ptr< CNetBroadCastSession >(new CNetBroadCastSession( netRunner_.io_service() ));
	broadCastSessionForRecvMessage_->setEvent( this );
	if( broadCastSessionForRecvMessage_->listenUdp( DEFAULT_BROADCAST_UDP_PORT_FOR_TEXTMSG ) == false )
	{
		// TODO : ignore this error..
	}
}

CSharedPaintManager::~CSharedPaintManager(void)
{
	close();
}

bool CSharedPaintManager::startClient( void )
{
	clearAllSessions();
	clearAllUsers();

	if( netPeerServer_ )
		netPeerServer_->close();

	if( broadCastSessionForConnection_ )
		broadCastSessionForConnection_->close();

	broadCastSessionForConnection_ = boost::shared_ptr< CNetBroadCastSession >(new CNetBroadCastSession( netRunner_.io_service() ));
	broadCastSessionForConnection_->setEvent( this );

	if( broadCastSessionForConnection_->listenUdp( DEFAULT_BROADCAST_PORT ) )
	{
		if( serverMode_ )
		{
			clearScreen();
			serverMode_ = false;
		}
		return true;
	}
	return false;
}

void CSharedPaintManager::setBroadCastChannel( const std::string & channel )
{
	std::string myIp = getMyIPAddress();
	std::string broadCastMsg = BroadCastPacketBuilder::CServerInfo::make( channel, myIp, acceptPort_ );
	
	if( broadCastSessionForConnection_ )
		broadCastSessionForConnection_->setBroadCastMessage( broadCastMsg );

	broadcastChannel_ = channel;
}

void CSharedPaintManager::startServer( const std::string &broadCastChannel, int port )
{
	serverMode_ = true;

	clearAllSessions();
	clearAllUsers();

	if( port <= 0 )
		port = START_SERVER_PORT;

	netPeerServer_ = boost::shared_ptr<CNetPeerServer>(new CNetPeerServer( netRunner_.io_service() ));
	netPeerServer_->setEvent( this );

	for( int port = START_SERVER_PORT; port < START_SERVER_PORT + 100; port++ )
	{
		if( netPeerServer_->start( port ) )
		{
			acceptPort_ = port;
			break;
		}
	}
	assert( acceptPort_ > 0 );

	std::string myIp = getMyIPAddress();

	std::string broadCastMsg = BroadCastPacketBuilder::CServerInfo::make( broadCastChannel, myIp, acceptPort_ );
	if( broadCastSessionForConnection_ )
		broadCastSessionForConnection_->close();

	broadCastSessionForConnection_ = boost::shared_ptr< CNetBroadCastSession >(new CNetBroadCastSession( netRunner_.io_service() ));
	broadCastSessionForConnection_->setEvent( this );

	broadCastSessionForConnection_->startSend( DEFAULT_BROADCAST_PORT, broadCastMsg, 3 );
}

// this function need to check session pointer null check!
void CSharedPaintManager::dispatchPaintPacket( boost::shared_ptr<CPaintSession> session, boost::shared_ptr<CPacketData> packetData )
{
	switch( packetData->code )
	{
	case CODE_SYSTEM_JOIN:
		{
			boost::shared_ptr<CPaintUser> user = SystemPacketBuilder::CJoinerUser::parse( packetData->body );
			if( session )
				user->setSessionId( session->sessionId() );

			addUser( user );
		}
		break;
	case CODE_SYSTEM_LEFT:
		{
			std::string userId;
			if( SystemPacketBuilder::CLeftUser::parse( packetData->body, userId ) )
			{
				removeUser( userId );
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
	}
}

void CSharedPaintManager::dispatchBroadCastPacket( boost::shared_ptr<CPacketData> packetData )
{
	switch( packetData->code )
	{
	case CODE_BROAD_SERVER_INFO:
		{
			std::string addr, broadcastChannel;
			int port;
			if( BroadCastPacketBuilder::CServerInfo::parse( packetData->body, broadcastChannel, addr, port ) )
			{
				if( broadcastChannel_ != broadcastChannel )
					return;

				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_GetServerInfo, this, broadcastChannel, addr, port ) );
			}
		}
		break;

	case CODE_BROAD_TEXT_MESSAGE:
		{
			std::string message, broadcastChannel, myId;
			if( BroadCastPacketBuilder::CTextMessage::parse( packetData->body, broadcastChannel, myId, message ) )
			{
				if( broadcastChannel_ != broadcastChannel )
					return;

				if( myId_ == myId )	// ignore myself message..
					return;
	
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ReceivedTextMessage, this, broadcastChannel, message ) );
			}
		}
		break;
	}
}
