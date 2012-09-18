#pragma once

#include <QNetworkInterface>
#include "Singleton.h"
#include "PaintItem.h"
#include "PacketSlicer.h"
#include "PaintPacketBuilder.h"
#include "WindowPacketBuilder.h"
#include "BroadCastPacketBuilder.h"
#include "UdpPacketBuilder.h"
#include "SystemPacketBuilder.h"
#include "TaskPacketBuilder.h"
#include "SharedPaintPolicy.h"
#include "DefferedCaller.h"
#include "SharedPaintCommandManager.h"
#include "PaintSession.h"
#include "NetPeerServer.h"
#include "NetBroadCastSession.h"
#include "NetUdpSession.h"
#include "NetServiceRunner.h"
#include "PaintUser.h"

#define SharePaintManagerPtr()		CSingleton<CSharedPaintManager>::Instance()

class CAddItemTask;
class CRemoveItemTask;
class CUpdateItemTask;
class CMoveItemTask;
class CSharedPaintManager;

class ISharedPaintEvent
{
public:
	virtual void onISharedPaintEvent_Connected( CSharedPaintManager *self ) = 0;
	virtual void onISharedPaintEvent_ConnectFailed( CSharedPaintManager *self ) = 0;
	virtual void onISharedPaintEvent_SendingPacket( CSharedPaintManager *self, int packetId, size_t wroteBytes, size_t totalBytes ) = 0;
	virtual void onISharedPaintEvent_ReceivedPacket( CSharedPaintManager *self ) = 0;
	virtual void onISharedPaintEvent_Disconnected( CSharedPaintManager *self ) = 0;
	virtual void onISharedPaintEvent_SyncStart( CSharedPaintManager *self ) = 0;
	virtual void onISharedPaintEvent_SyncComplete( CSharedPaintManager *self ) = 0;
	virtual void onISharedPaintEvent_AddPaintItem( CSharedPaintManager *self, boost::shared_ptr<CPaintItem> item ) = 0;
	virtual void onISharedPaintEvent_UpdatePaintItem( CSharedPaintManager *self, boost::shared_ptr<CPaintItem> item ) = 0;
	virtual void onISharedPaintEvent_RemovePaintItem( CSharedPaintManager *self, boost::shared_ptr<CPaintItem> item ) = 0;
	virtual void onISharedPaintEvent_MovePaintItem( CSharedPaintManager *self, boost::shared_ptr<CPaintItem> item, double x, double y ) = 0;
	virtual void onISharedPaintEvent_ResizeMainWindow( CSharedPaintManager *self, int width, int height ) = 0;
	virtual void onISharedPaintEvent_SetBackgroundImage( CSharedPaintManager *self, boost::shared_ptr<CBackgroundImageItem> image ) = 0;
	virtual void onISharedPaintEvent_SetBackgroundColor( CSharedPaintManager *self, int r, int g, int b, int a ) = 0;
	virtual void onISharedPaintEvent_SetBackgroundGridLine( CSharedPaintManager *self, int size ) = 0;
	virtual void onISharedPaintEvent_ClearScreen( CSharedPaintManager *self ) = 0;
	virtual void onISharedPaintEvent_ClearBackground( CSharedPaintManager *self ) = 0;
	virtual void onISharedPaintEvent_UpdatePaintUser( CSharedPaintManager *self, boost::shared_ptr<CPaintUser> user ) = 0;
	virtual void onISharedPaintEvent_GetServerInfo( CSharedPaintManager *self, const std::string &paintChannel, const std::string &addr, int port ) = 0;
	virtual void onISharedPaintEvent_ReceivedTextMessage( CSharedPaintManager *self, const std::string &paintChannel, const std::string &message ) = 0;
	virtual void onISharedPaintEvent_AddTask( CSharedPaintManager *self, int totalTaskCount, bool playBackWorking ) = 0;
	virtual void onISharedPaintEvent_ServerFinding( CSharedPaintManager *self, int sentCount ) = 0;
};


class CSharedPaintManager : public INetPeerServerEvent, INetBroadCastSessionEvent, INetUdpSessionEvent, IPaintSessionEvent
{
private:
	typedef std::map< std::string, boost::shared_ptr<CSharedPaintItemList> > ITEM_LIST_MAP;
	typedef std::map< std::string, boost::shared_ptr<CPaintUser> > USER_MAP;
	typedef std::vector< boost::shared_ptr<CPaintSession> > SESSION_LIST;

public:
	CSharedPaintManager(void);
	~CSharedPaintManager(void);

	const std::string& myId( void ) 
	{
		return myId_;
	}

	void close( void )
	{
		stopServer();
		stopFindingServer();

		if( broadCastSessionForSendMessage_ )
			broadCastSessionForSendMessage_->close();

		if( broadCastSessionForRecvMessage_ )
			broadCastSessionForRecvMessage_->close();

		netRunner_.close();

		findingServerMode_ = false;
	}

	void setCanvas( IGluePaintCanvas *canvas )
	{
		canvas_ = canvas;
	}

	void registerObserver( ISharedPaintEvent *obs )
	{
		observers_.remove( obs );
		observers_.push_back( obs );
	}

	void unregisterObserver( ISharedPaintEvent *obs )
	{
		observers_.remove( obs );
	}

	// Network
public:
	bool startFindingServer( void );
	void stopFindingServer( void );

	void setPaintChannel( const std::string & channel );

	bool connectToPeer( const std::string &addr, int port )
	{
		clearScreen();
		clearAllUsers();

		boost::shared_ptr<CNetPeerSession> session = netRunner_.newSession();
		boost::shared_ptr<CPaintSession> userSession(new CPaintSession(session, this));

		mutexSession_.lock();
		sessionList_.push_back( userSession );
		mutexSession_.unlock();

		// must be called here for preventing from a crash by thread race condition.
		userSession->session()->connect( addr, port );

		return true;
	}

	bool requestJoinServer( const std::string &addr, int port, const std::string &userid, const std::string &roomid )
	{
		stopFindingServer();
		startServer();

		clearAllUsers();
		clearAllSessions();
		clearAllItems();

		boost::shared_ptr<CNetPeerSession> session = netRunner_.newSession();
		boost::shared_ptr<CPaintSession> userSession(new CPaintSession(session, this));

		mutexSession_.lock();
		relayServerSession_ = userSession;
		mutexSession_.unlock();

		// must be called here for preventing from a crash by thread race condition.
		userSession->session()->connect( addr, port );
		return true;
	}

	void clearAllSessions( void )
	{
		mutexSession_.lock();

		superPeerSession_ = boost::shared_ptr<CPaintSession>():
		relayServerSession_ = boost::shared_ptr<CPaintSession>();

		SESSION_LIST::iterator it = sessionList_.begin();
		for( ; it != sessionList_.end(); it++ )
		{
			(*it)->close();
		}

		mutexSession_.unlock();
	}

	int acceptPort( void ) const
	{
		return listenTcpPort_;
	}

	int lastPacketId( void )
	{
		return lastPacketId_;
	}

	bool isFindingServerMode( void )
	{
		return findingServerMode_;
	}

	bool isConnecting( void )
	{
		boost::recursive_mutex::scoped_lock autolock(mutexSession_);

		if( relayServerSession_ && relayServerSession_->session()->isConnecting() )
			return true;
		
		if( superPeerSession_ && superPeerSession_->session()->isConnecting() )
			return true;

		SESSION_LIST::iterator it = sessionList_.begin();
		for( ; it != sessionList_.end(); it++ )
		{
			if( (*it)->session()->isConnecting() )
				return true;
		}
		return false;
	}

	bool isConnected( void )
	{
		boost::recursive_mutex::scoped_lock autolock(mutexSession_);
		
		if( relayServerSession_ && relayServerSession_->session()->isConnected() )
			return true;
		
		if( superPeerSession_ && superPeerSession_->session()->isConnected() )
			return true;

		SESSION_LIST::iterator it = sessionList_.begin();
		for( ; it != sessionList_.end(); it++ )
		{
			if( (*it)->session()->isConnected() )
				return true;
		}
		return false;
	}

	int sendDataToUsers( const SESSION_LIST &sessionList, const std::string &msg, int toSessionId = -1 )
	{
		static int PACKETID = 0;
		int sendCnt = 0;
		int packetId = ++PACKETID;
		std::vector<struct send_byte_info_t> infolist;

		SESSION_LIST sendableSessionList;
		SESSION_LIST::const_iterator it = sessionList.begin();
		for( ; it != sessionList.end(); it++ )
		{
			if( (*it)->session()->isConnected() )
			{
				if( toSessionId >= 0 && (*it)->sessionId() != toSessionId )
					continue;

				struct send_byte_info_t info;
				info.session = (*it).get();
				info.totalBytes = msg.size();
				info.wroteBytes = 0;

				infolist.push_back( info );

				sendableSessionList.push_back( *it );

				// DO NOT SEND HERE BUT DO IT OVER THERE
				//boost::shared_ptr<CNetPacketData> packet = boost::shared_ptr<CNetPacketData>(new CNetPacketData( packetId, msg ) );
				//(*it)->session()->sendData( packet );
				sendCnt ++;
			}
		}

		if ( sendCnt > 0 )
		{
			mutexSendInfo_.lock();
			sendInfoDataMap_.insert( send_info_map_t::value_type( packetId, infolist ) );
			lastPacketId_ = packetId;
			mutexSendInfo_.unlock();
			
			// MUST send after above codes for preventing from race condition..
			for( size_t i = 0; i < sendableSessionList.size(); i++ )
			{
				boost::shared_ptr<CNetPacketData> packet = boost::shared_ptr<CNetPacketData>(new CNetPacketData( packetId, msg ) );
				sendableSessionList[i]->session()->sendData( packet );
			}
		}
		else
			packetId = -1;

		return packetId;
	}

	int sendDataToUsers( const std::string &msg, int toSessionId = -1 )
	{
		SESSION_LIST sessionList;
		
		if( superPeerSession_ && superPeerSession_->session()->isConnected() )	// not me & super suer exist
		{
			qDebug() << "sendDataToUsers() : Netmode = to superpeer";
			sessionList.push_back( superPeerSession_ );
		}
		else if( sessionList_.size() > 0 )
		{
			qDebug() << "sendDataToUsers() : Netmode = I'm superpeer";
			sessionList = sessionList_;
		}
		else if( relayServerSession_->session()->isConnected() )
		{
			qDebug() << "sendDataToUsers() : Netmode = to relay server";
			sessionList.push_back( relayServerSession_ );
		}
		else
			return -1;

		return sendDataToUsers( sessionList, msg, toSessionId );
	}

	void sendBroadCastTextMessage( const std::string &paintChannel, const std::string &msg );

	// Shared Paint Action
public:
	void setEnabled( bool enabled )
	{
		enabled_ = enabled;
	}

	void redoCommand( void )
	{
		commandMngr_.redoCommand();
	}

	void undoCommand( void )
	{
		commandMngr_.undoCommand();
	}

	void deserializeData( const char * data, size_t size );
	
	std::string serializeData( const std::string *target = NULL );

	bool sendPaintItem( boost::shared_ptr<CPaintItem> item )
	{
		if( ! enabled_ )
			return false;

		std::string msg = PaintPacketBuilder::CCreateItem::make( item );
		sendDataToUsers( msg );

		commandMngr_.addHistoryItem( item );

		boost::shared_ptr<CAddItemCommand> command = boost::shared_ptr<CAddItemCommand>(new CAddItemCommand( item ));
		return commandMngr_.executeCommand( command );
	}

	void updatePaintItem( boost::shared_ptr< CPaintItem > item )
	{
		if( ! enabled_ )
			return;

		boost::shared_ptr<CUpdateItemCommand> command = boost::shared_ptr<CUpdateItemCommand>(new CUpdateItemCommand( item ));
		commandMngr_.executeCommand( command );
	}

	void movePaintItem( boost::shared_ptr< CPaintItem > item )
	{
		if( ! enabled_ )
			return;

		boost::shared_ptr<CMoveItemCommand> command = boost::shared_ptr<CMoveItemCommand>(new CMoveItemCommand( item ));
		commandMngr_.executeCommand( command );
	}

	void removePaintItem( boost::shared_ptr< CPaintItem > item )
	{
		if( ! enabled_ )
			return;

		boost::shared_ptr<CRemoveItemCommand> command = boost::shared_ptr<CRemoveItemCommand>(new CRemoveItemCommand( item ));
		commandMngr_.executeCommand( command );
	}

	int sendBackgroundImage( boost::shared_ptr<CBackgroundImageItem> image )
	{
		if( ! enabled_ )
			return false;

		if( !image )
			return -1;

		backgroundImageItem_ = image;

		fireObserver_SetBackgroundImage( image );

		std::string msg = PaintPacketBuilder::CSetBackgroundImage::make( image );
		int packetId = sendDataToUsers( msg );
		return packetId;
	}

	void setBackgroundColor( int r, int g, int b, int a )
	{
		if( ! enabled_ )
			return;

		std::string msg = PaintPacketBuilder::CSetBackgroundColor::make( r, g, b, a );
		sendDataToUsers( msg );

		fireObserver_SetBackgroundColor( r, g, b, a );
	}

	void clearBackground( void )
	{
		if( ! enabled_ )
			return;

		// data init..
		backgroundImageItem_ = boost::shared_ptr<CBackgroundImageItem>();

		std::string msg = PaintPacketBuilder::CClearBackground::make();
		sendDataToUsers( msg );

		fireObserver_ClearBackground();
	}

	void clearScreen( bool sendData = true )
	{
		if( ! enabled_ )
			return;

		if( sendData )
		{
			std::string msg = PaintPacketBuilder::CClearScreen::make();
			sendDataToUsers( msg );
		}

		caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ClearScreen, this ) );
	}

	void setBackgroundGridLine( int size )
	{
		if( ! enabled_ )
			return;

		std::string msg = PaintPacketBuilder::CSetBackgroundGridLine::make( size );
		sendDataToUsers( msg );

		fireObserver_SetBackgroundGridLine( size );
	}
	
	int notifyResizingMainWindow( int width, int height )
	{
		if( ! enabled_ )
			return -1;

		std::string msg = WindowPacketBuilder::CResizeMainWindow::make( width, height );
		lastWindowWidth_ = width;
		lastWindowHeight_ = height;
		return sendDataToUsers( msg );
	}

	// Playback
public:
	bool isPlaybackMode( void ) { return commandMngr_.isPlaybackMode(); }

	size_t historyTaskCount( void ) { return commandMngr_.historyTaskCount(); }

	void plabackTo( int position )
	{
		commandMngr_.playbackTo( position );
	}

private:
	void sendAllSyncData( int toSessionId )
	{
		if( isAlwaysP2PMode() == false )
			return;

		std::string allData = serializeData();

		// User Info
		allData += generateJoinerInfoPacket();

		sendDataToUsers( allData, toSessionId );
	}

	// User Mansagement and Sync
public:
	int userCount( void )
	{
		return joinerMap_.size();
	}

private:
	void sendMyUserInfo( boost::shared_ptr<CPaintSession> session )
	{
		std::string msg;
		if( isRelayServerSession( session ) )
			msg = SystemPacketBuilder::CJoinerServerUser::make( myUserInfo_ );
		else
			msg = SystemPacketBuilder::CJoinerSuperPeerUser::make( myUserInfo_ );

		session->session()->sendData( msg );
	}

	void notifyRemoveUserInfo( boost::shared_ptr<CPaintUser> user )
	{
		qDebug() << "notifyRemoveUserInfo" << joinerMap_.size();
		std::string msg = SystemPacketBuilder::CLeftUser::make( user->channel(), user->userId() );
		sendDataToUsers( msg );
	}

	void addUser( boost::shared_ptr<CPaintUser> user )
	{
		mutexUser_.lock();
		std::pair<USER_MAP::iterator, bool> res = joinerMap_.insert( USER_MAP::value_type( user->userId(), user ) );
		if( !res.second )
			res.first->second = user;	// overwrite;
		mutexUser_.unlock();

		caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_UpdatePaintUser, this, user ) );
	}

	boost::shared_ptr<CPaintUser> findUser( int sessionId )
	{
		boost::recursive_mutex::scoped_lock autolock(mutexUser_);

		USER_MAP::iterator it = joinerMap_.begin();
		for( ; it != joinerMap_.end(); it++ )
		{
			if( it->second->sessionId() == sessionId )
			{
				return it->second;
			}
		}
		return boost::shared_ptr<CPaintUser>();
	}

	boost::shared_ptr<CPaintUser> findUser( const std::string &userId )
	{
		boost::recursive_mutex::scoped_lock autolock(mutexUser_);

		USER_MAP::iterator it = joinerMap_.find( userId );
		if( it != joinerMap_.end() )
			return it->second;
		return boost::shared_ptr<CPaintUser>();
	}

	void removeUser( const std::string & userId )
	{
		boost::shared_ptr<CPaintUser> removing;
		// erase from map
		{
			boost::recursive_mutex::scoped_lock autolock(mutexUser_);

			USER_MAP::iterator it = joinerMap_.find( userId );
			if( it == joinerMap_.end() )
				return;

			removing = it->second;
			joinerMap_.erase( it );
		}

		if( removing )
			caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_UpdatePaintUser, this, removing ) );
	}

	void removeUser( boost::shared_ptr<CPaintUser> user )
	{
		removeUser( user->userId() ); 
	}

	void removeUser( int sessionId )
	{
		boost::shared_ptr<CPaintUser> removing = findUser( sessionId );

		removeUser( removing );
	}

	void clearAllUsers( void )
	{
		boost::recursive_mutex::scoped_lock autolock(mutexUser_);
		joinerMap_.clear();
		joinerMap_.insert( USER_MAP::value_type(myUserInfo_->userId(), myUserInfo_) );
	}

	std::string generateJoinerInfoPacket( void )
	{
		std::string allData;

		// User Info
		mutexUser_.lock();
		USER_MAP::iterator it = joinerMap_.begin();
		for( ; it != joinerMap_.end(); it++ )
		{
			std::string msg = SystemPacketBuilder::CJoinerSuperPeerUser::make( it->second );
			allData += msg;
		}
		mutexUser_.unlock();

		return allData;
	}

public:
	boost::shared_ptr<CPaintItem> findPaintItem( const std::string & owner, int itemId )
	{
		return commandMngr_.findItem( owner, itemId );
	}

private:
	void clearAllItems( void )	// this function must be called on main thread!
	{
		assert( caller_.isMainThread() );

		backgroundColor_ = Qt::white;
		backgroundImageItem_ = boost::shared_ptr<CBackgroundImageItem>();
		canvas_->clearScreen();

		// all data clear
		commandMngr_.clear();
	}

	boost::shared_ptr<CPaintSession> findSession( int sessionId )
	{
		boost::recursive_mutex::scoped_lock autolock(mutexSession_);

		SESSION_LIST::iterator it = sessionList_.begin();
		for( ; it != sessionList_.end(); it++ )
		{
			if( (*it)->sessionId() == sessionId )
			{
				return *it;
			}
		}
		return boost::shared_ptr<CPaintSession>();
	}

private:
	bool startServer( int port = 0 );
	void stopServer( void );

	bool connectToSuperPeer( boost::shared_ptr<CPaintUser> user )
	{
		boost::shared_ptr<CNetPeerSession> session = netRunner_.newSession();
		boost::shared_ptr<CPaintSession> userSession = boost::shared_ptr<CPaintSession>(new CPaintSession(session, this));

		mutexSession_.lock();
		superPeerSession_ = userSession;
		mutexSession_.unlock();

		// must be called here for preventing from a crash by thread race condition.

		userSession->session()->connect( user->localIPAddress()/*TODO RELAYMODE*/, user->listenTcpPort() );

		return true;
	}

	void dispatchBroadCastPacket( CNetBroadCastSession *session, boost::shared_ptr<CPacketData> packetData );
	void dispatchUdpPacket( CNetUdpSession *session, boost::shared_ptr<CPacketData> packetData );
	void dispatchPaintPacket( boost::shared_ptr<CPaintSession> session, boost::shared_ptr<CPacketData> packetData );

	inline bool isRelayServerSession( boost::shared_ptr<CPaintSession> session )
	{
		if ( relayServerSession_ && session->sessionId() == relayServerSession_->sessionId() )
			return true;
		return false;
	}

	inline bool isSuperPeerSession( boost::shared_ptr<CPaintSession> session )
	{
		if ( superPeerSession_ && session->sessionId() == superPeerSession_->sessionId() )
			return true;
		return false;
	}

	inline bool isMySelfSuperPeer( void ) 
	{
		if( ! superPeerSession_ && superPeerId_ == myId_ )
			return true;
		return false;
	}

	inline bool isAlwaysP2PMode( void )
	{
		return relayServerSession_ == NULL;	// TODO : isAwaysP2PMode
	}

private:
	// observer methods
	void fireObserver_ConnectFailed( void )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ConnectFailed( this );
		}
	}
	void fireObserver_Connected( int sessionId )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_Connected( this );
		}

		if( isAlwaysP2PMode() && isMySelfSuperPeer() )
			sendAllSyncData( sessionId );
	}
	void fireObserver_DisConnected( void )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_Disconnected( this );
		}
	}
	void fireObserver_SyncStart( void )
	{
		enabled_ = false;

		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_SyncStart( this );
		}
	}
	void fireObserver_SyncComplete( void )
	{
		enabled_ = true;

		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_SyncComplete( this );
		}
	}
	void fireObserver_UpdatePaintItem( boost::shared_ptr<CPaintItem> item )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_UpdatePaintItem( this, item );
		}
	}
	void fireObserver_AddPaintItem( boost::shared_ptr<CPaintItem> item )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_AddPaintItem( this, item );
		}
	}
	void fireObserver_MovePaintItem( boost::shared_ptr<CPaintItem> item, double x, double y )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_MovePaintItem( this, item, x, y );
		}
	}
	void fireObserver_RemovePaintItem( boost::shared_ptr<CPaintItem> item )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_RemovePaintItem( this, item );
		}
	}
	void fireObserver_ResizeMainWindow( int width, int height )
	{
		lastWindowWidth_ = width;
		lastWindowHeight_ = height;

		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ResizeMainWindow( this, width, height );
		}
	}
	void fireObserver_SendingPacket( int packetId, size_t wroteBytes, size_t totalBytes )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_SendingPacket( this, packetId, wroteBytes, totalBytes );
		}
	}
	void fireObserver_ClearScreen( void )
	{
		clearAllItems();

		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ClearScreen( this );
		}
	}
	void fireObserver_ClearBackground( void )
	{
		backgroundImageItem_ = boost::shared_ptr<CBackgroundImageItem>(); // clear

		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ClearBackground( this );
		}
	}
	void fireObserver_SetBackgroundImage( boost::shared_ptr<CBackgroundImageItem> image )
	{
		backgroundImageItem_ = image;

		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_SetBackgroundImage( this, image );
		}
	}
	void fireObserver_SetBackgroundColor( int r, int g, int b, int a )
	{
		backgroundColor_ = QColor(r, g, b, a);

		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_SetBackgroundColor( this, r, g, b, a );
		}
	}
	void fireObserver_SetBackgroundGridLine( int size )
	{
		gridLineSize_ = size;

		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_SetBackgroundGridLine( this, size );
		}
	}
	void fireObserver_ReceivedTextMessage( const std::string &paintChannel, const std::string &message )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ReceivedTextMessage( this, paintChannel, message );
		}
	}
	void fireObserver_GetServerInfo( const std::string &paintChannel, const std::string &addr, int port )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_GetServerInfo( this, paintChannel, addr, port );
		}
	}
	void fireObserver_UpdatePaintUser( boost::shared_ptr<CPaintUser> user )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_UpdatePaintUser( this, user );
		}
	}
	void fireObserver_ReceivedPacket( void )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ReceivedPacket( this );
		}
	}

	void fireObserver_AddTask( int totalTaskCount, bool playBackWorking )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_AddTask( this, totalTaskCount, playBackWorking );
		}
	}

	void fireObserver_ServerFinding( int sentCount )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ServerFinding( this, sentCount );
		}
	}

	// delaying remove session feature
private:
	void _delayedRemoveSession( int sessionId )
	{
		boost::recursive_mutex::scoped_lock autolock(mutexSession_);

		SESSION_LIST::iterator it = sessionList_.begin();
		for( ; it != sessionList_.end(); it++ )
		{
			if( (*it)->session()->sessionId() == sessionId )
			{
				sessionList_.erase(it);
				break;
			}
		}

		if( superPeerSession_ && superPeerSession_->sessionId() == sessionId )
			superPeerSession_ = boost::shared_ptr<CPaintSession>();

		if( relayServerSession_ && relayServerSession_->sessionId() == sessionId )
			relayServerSession_ = boost::shared_ptr<CPaintSession>();
	}
	void removeSession( int sessionId )
	{
		caller_.performMainThreadAlwaysDeffered( boost::bind(&CSharedPaintManager::_delayedRemoveSession, this, sessionId) );
	}

protected:
	// INetPeerServerEvent
	virtual void onINetPeerServerEvent_Accepted( boost::shared_ptr<CNetPeerServer> server, boost::shared_ptr<CNetPeerSession> session )
	{
		boost::shared_ptr<CPaintSession> userSession = boost::shared_ptr<CPaintSession>(new CPaintSession(session, this));
		
		mutexSession_.lock();
		sessionList_.push_back( userSession );
		mutexSession_.unlock();

		// start read io!
		session->start();

		if( isAlwaysP2PMode() )
			superPeerId_ = myId_;
	}

	// INetBroadCastSessionEvent
	virtual void onINetBroadCastSessionEvent_BroadCastReceived( CNetBroadCastSession *session, const std::string buffer )
	{
		CPacketSlicer slicer;

		slicer.addBuffer( buffer );

		if( slicer.parse() == false )
			return;

		for( size_t i = 0; i < slicer.parsedItemCount(); i++ )
		{
			boost::shared_ptr<CPacketData> data = slicer.parsedItem( i );

			dispatchBroadCastPacket( session, data );
		}
	}

	virtual void onINetBroadCastSessionEvent_SentMessage( CNetBroadCastSession *session, int sentCount )
	{
		caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ServerFinding, this, sentCount ) );
	}

	// INetUdpSessionEvent
	virtual void onINetUdpSessionEvent_Received( CNetUdpSession *session, const std::string buffer )
	{
		CPacketSlicer slicer;

		slicer.addBuffer( buffer );

		if( slicer.parse() == false )
			return;

		for( size_t i = 0; i < slicer.parsedItemCount(); i++ )
		{
			boost::shared_ptr<CPacketData> data = slicer.parsedItem( i );

			dispatchUdpPacket( session, data );
		}
	}

	// IPaintSessionEvent
	virtual void onIPaintSessionEvent_Connected( boost::shared_ptr<CPaintSession> session )
	{
		if( isFindingServerMode() )
			broadCastSessionForConnection_->pauseSend();

		if( isRelayServerSession( session ) || isSuperPeerSession(session) || isAlwaysP2PMode() )
		{
			// send to my user info to relay server or super peer
			sendMyUserInfo( session );
		}

		if ( isRelayServerSession( session ) || isAlwaysP2PMode() )
		{
			caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_Connected, this, session->sessionId() ) );
		}
		else 
		{
			// when connected to superpeer, do not send my info to him.
		}
	}

	virtual void onIPaintSessionEvent_ConnectFailed( boost::shared_ptr<CPaintSession> session )
	{
		if ( isRelayServerSession( session ) )
		{
			caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ConnectFailed, this ) );
		}

		removeSession( session->sessionId() );
	}

	virtual void onIPaintSessionEvent_Disconnected( boost::shared_ptr<CPaintSession> session )
	{
		if( isFindingServerMode() )
			broadCastSessionForConnection_->resumeSend();

		if( isConnected() == false )
			caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_DisConnected, this ) );

		boost::shared_ptr<CPaintUser> user = findUser( session->sessionId() );
		if( user )
		{
			if( isAlwaysP2PMode() )
			{
				notifyRemoveUserInfo( user );
			}
			removeUser( user );
		}

		removeSession( session->sessionId() );
	}

	virtual void onIPaintSessionEvent_ReceivedPacket( boost::shared_ptr<CPaintSession> session, const boost::shared_ptr<CPacketData> data )
	{
		dispatchPaintPacket( session, data );

		caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ReceivedPacket, this ) );

		// to send the others without this user
		if( sessionList_.size() <= 1 )
			return;

		SESSION_LIST list = sessionList_;
		SESSION_LIST::iterator it = list.begin();
		for( ; it != list.end(); it++ )
		{
			if( *it == session )
			{
				list.erase( it );
				break;
			}
		}

		std::string msg = CommonPacketBuilder::makePacket( data->code, data->body );
		sendDataToUsers( list, msg );
	}

	virtual void onIPaintSessionEvent_SendingPacket( boost::shared_ptr<CPaintSession> session, const boost::shared_ptr<CNetPacketData> packet )
	{
		//qDebug() << "Packet sending " << packet->packetId() << packet->buffer().remainingSize() << packet->buffer().totalSize();
		if( packet->packetId() < 0 )
		{
			qDebug() << "onIPaintSessionEvent_SendingPacket error : packet id < 0";
			return;	// ignore this!
		}

		size_t totalBytes = 0;
		size_t wroteBytes = 0;
	
		// processing wrote bytes for all joiners
		{
			boost::recursive_mutex::scoped_lock autolock(mutexSendInfo_);

			send_info_map_t::iterator it = sendInfoDataMap_.find( packet->packetId() );
			if( it == sendInfoDataMap_.end() )
			{
				qDebug() << "onIPaintSessionEvent_SendingPacket error : not found info" << packet->packetId();
				return;	// not found. unexpected error..
			}

			std::vector<struct send_byte_info_t>::iterator itD = it->second.begin();
			for( ; itD != it->second.end(); itD++ )
			{
				if( (*itD).session == session.get() )
				{
					(*itD).wroteBytes = packet->buffer().totalSize() -  packet->buffer().remainingSize();
					break;
				}
			}

			for( size_t i = 0; i < it->second.size(); i++ )
			{
				const struct send_byte_info_t &info = it->second[i];
				totalBytes += info.totalBytes;
				wroteBytes += info.wroteBytes;
			}

			if( totalBytes <= wroteBytes )
			{
				//qDebug() << "sendInfoDataMap_.erase !!i!!" << packet->packetId() << wroteBytes << totalBytes;
				sendInfoDataMap_.erase( it );
			}
		}

		//qDebug() << "sendInfoDataMap_.noti !!i!!" << packet->packetId() << wroteBytes << totalBytes << "====" << packet->buffer().remainingSize() << packet->buffer().totalSize();

		caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_SendingPacket, this, packet->packetId(), wroteBytes, totalBytes ) );
	}

private:
	friend class CSharedPaintCommandManager;
	friend class CAddItemTask;
	friend class CRemoveItemTask;
	friend class CUpdateItemTask;
	friend class CMoveItemTask;

	std::string myId_;
	CDefferedCaller caller_;
	bool enabled_;

	// obsevers
	std::list<ISharedPaintEvent *> observers_;

	// my action command
	CSharedPaintCommandManager commandMngr_;

	// paint item
	IGluePaintCanvas *canvas_;

	// for paint item temporary caching..
	boost::shared_ptr<CBackgroundImageItem> backgroundImageItem_;
	int lastWindowWidth_;
	int lastWindowHeight_;
	QColor backgroundColor_;
	int gridLineSize_;

	// user management
	boost::recursive_mutex mutexUser_;
	boost::shared_ptr<CPaintUser> myUserInfo_;
	USER_MAP joinerMap_;
	
	// network
	CNetServiceRunner netRunner_;
	bool findingServerMode_;
	int listenTcpPort_;
	int listenUdpPort_;
	std::string superPeerId_;
	SESSION_LIST sessionList_;
	boost::shared_ptr<CPaintSession> superPeerSession_;
	boost::shared_ptr<CPaintSession> relayServerSession_;
	boost::recursive_mutex mutexSession_;
	boost::shared_ptr<CNetPeerServer> netPeerServer_;
	boost::shared_ptr< CNetUdpSession > udpSessionForConnection_;
	boost::shared_ptr< CNetBroadCastSession > broadCastSessionForConnection_;
	boost::shared_ptr< CNetBroadCastSession > broadCastSessionForSendMessage_;
	boost::shared_ptr< CNetBroadCastSession > broadCastSessionForRecvMessage_;
	std::string paintChannel_;

	// seding byte management
	boost::recursive_mutex mutexSendInfo_;
	struct send_byte_info_t
	{
		int wroteBytes;
		int totalBytes;
		CPaintSession *session;
	};
	typedef std::map< int, std::vector<struct send_byte_info_t> > send_info_map_t;
	send_info_map_t sendInfoDataMap_;
	int lastPacketId_;
};
