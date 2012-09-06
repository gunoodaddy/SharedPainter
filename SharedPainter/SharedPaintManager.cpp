#include "StdAfx.h"
#include "SharedPaintManager.h"
#include "BroadCastPacketBuilder.h"
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

CSharedPaintManager::CSharedPaintManager(void) : canvas_(NULL), acceptPort_(-1), serverMode_(false), lastPacketId_(-1)
{
	// default generate my id
	myId_ = generateMyId();
}

CSharedPaintManager::~CSharedPaintManager(void)
{
	if( netPeerServer_ )
		netPeerServer_->close();

	if( broadCastSession_ )
		broadCastSession_->close();

	sessionList_.clear();

	netRunner_.close();
}

bool CSharedPaintManager::startClient( void )
{
	if( netPeerServer_ )
		netPeerServer_->close();

	if( broadCastSession_ )
		broadCastSession_->close();

	broadCastSession_ = boost::shared_ptr< CNetBroadCastSession >(new CNetBroadCastSession( netRunner_.io_service() ));
	broadCastSession_->setEvent( this );

	if( broadCastSession_->startRead( DEFAULT_BROADCAST_PORT ) )
	{
		if( serverMode_ )
		{
			clearAllItems();
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
	
	if( broadCastSession_ )
		broadCastSession_->setBroadCastMessage( broadCastMsg );
}

void CSharedPaintManager::startServer( const std::string &broadCastChannel, int port )
{
	serverMode_ = true;

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
	if( broadCastSession_ )
		broadCastSession_->close();

	broadCastSession_ = boost::shared_ptr< CNetBroadCastSession >(new CNetBroadCastSession( netRunner_.io_service() ));
	broadCastSession_->setEvent( this );

	broadCastSession_->startSend( DEFAULT_BROADCAST_PORT, broadCastMsg, 3 );
}


void CSharedPaintManager::dispatchPaintPacket( boost::shared_ptr<CPacketData> packetData )
{
	switch( packetData->code )
	{
	case CODE_PAINT_CLEAR_SCREEN:
		{
			PaintPacketBuilder::CClearScreen::parse( packetData->body );	// nothing to do..
			caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ClearScreen, this ) );
		}
		break;
	case CODE_PAINT_CLEAR_BG_IMAGE:
		{
			PaintPacketBuilder::CClearScreen::parse( packetData->body );	// nothing to do..
			caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ClearBackgroundImage, this ) );
		}
		break;
	case CODE_PAINT_SET_BG_IMAGE:
		{
			boost::shared_ptr<CBackgroundImageItem> image = PaintPacketBuilder::CSetBackgroundImage::parse( packetData->body );
			caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_SetBackgroundImage, this, image ) );
		}
		break;
	case CODE_PAINT_ADD_ITEM:
		{
			boost::shared_ptr<CPaintItem> item = PaintPacketBuilder::CAddItem::parse( packetData->body );
			if( item )
				addPaintItem( item );
		}
		break;
	case CODE_PAINT_UPDATE_ITEM:
		{
			struct SPaintData data;
			if( PaintPacketBuilder::CUpdateItem::parse( packetData->body, data ) )
			{
				boost::shared_ptr<CPaintItem> item = findItem( data.owner, data.itemId );
				if( item )
				{
					item->setData( data );
					updatePaintItem( item );
				}
			}
		}
		break;
	case CODE_PAINT_REMOVE_ITEM:
		{
			std::string owner;
			int itemId;
			if( PaintPacketBuilder::CRemoveItem::parse( packetData->body, owner, itemId ) )
			{
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_RemovePaintItem, this, owner, itemId ) );
			}
		}
		break;
	case CODE_PAINT_MOVE_ITEM:
		{
			std::string owner;
			double x, y;
			int itemId;
			if( PaintPacketBuilder::CMoveItem::parse( packetData->body, owner, itemId, x, y ) )
			{
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_MovePaintItem, this, owner, itemId, x, y ) );
			}
		}
		break;
	case CODE_WINDOW_RESIZE_MAIN_WND:
		{
			std::string owner;
			int width, height;
			if( WindowPacketBuilder::CResizeMainWindow::parse( packetData->body, width, height ) )
			{
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
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_GetServerInfo, this, broadcastChannel, addr, port ) );
			}
		}
		break;
	}
}