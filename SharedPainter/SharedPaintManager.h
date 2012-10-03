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
#include "GlobalDefine.h"

#define SharePaintManagerPtr()		CSingleton<CSharedPaintManager>::Instance()

class CAddItemTask;
class CRemoveItemTask;
class CUpdateItemTask;
class CMoveItemTask;
class CSharedPaintManager;

class ISharedPaintEvent
{
public:
	virtual void onISharedPaintEvent_ShowErrorMessage( CSharedPaintManager *self, const std::string &error ) = 0;
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
	virtual void onISharedPaintEvent_ResizeCanvas( CSharedPaintManager *self, int width, int height ) = 0;
	virtual void onISharedPaintEvent_ChangeCanvasScrollPos( CSharedPaintManager *self, int posH, int posV ) = 0;
	virtual void onISharedPaintEvent_ResizeWindowSplitter( CSharedPaintManager *self, std::vector<int> &sizes ) = 0;
	virtual void onISharedPaintEvent_SetBackgroundImage( CSharedPaintManager *self, boost::shared_ptr<CBackgroundImageItem> image ) = 0;
	virtual void onISharedPaintEvent_SetBackgroundColor( CSharedPaintManager *self, int r, int g, int b, int a ) = 0;
	virtual void onISharedPaintEvent_SetBackgroundGridLine( CSharedPaintManager *self, int size ) = 0;
	virtual void onISharedPaintEvent_ClearScreen( CSharedPaintManager *self ) = 0;
	virtual void onISharedPaintEvent_ClearBackground( CSharedPaintManager *self ) = 0;
	virtual void onISharedPaintEvent_EnterPaintUser( CSharedPaintManager *self, boost::shared_ptr<CPaintUser> user ) = 0;
	virtual void onISharedPaintEvent_LeavePaintUser( CSharedPaintManager *self, boost::shared_ptr<CPaintUser> user ) = 0;
	virtual void onISharedPaintEvent_UpdatePaintUser( CSharedPaintManager *self, boost::shared_ptr<CPaintUser> user ) = 0;
	virtual void onISharedPaintEvent_GetServerInfo( CSharedPaintManager *self, const std::string &paintChannel, const std::string &addr, int port ) = 0;
	virtual void onISharedPaintEvent_ReceivedBroadcastTextMessage( CSharedPaintManager *self, const std::string &paintChannel, const std::string &fromId, const std::string &nickName, const std::string &message ) = 0;
	virtual void onISharedPaintEvent_AddTask( CSharedPaintManager *self, int totalTaskCount, bool playBackWorking ) = 0;
	virtual void onISharedPaintEvent_ServerFinding( CSharedPaintManager *self, int sentCount ) = 0;
	virtual void onISharedPaintEvent_ChangedNickName( CSharedPaintManager *self, const std::string & userId, const std::string &prevNickName, const std::string &currNickName ) = 0;
	virtual void onISharedPaintEvent_ReceivedChatMessage( CSharedPaintManager *self, const std::string & userId, const std::string &nickName, const std::string &chatMsg ) = 0;
};


class CSharedPaintManager : public QObject, INetPeerServerEvent, INetBroadCastSessionEvent, INetUdpSessionEvent, IPaintSessionEvent
{
	Q_OBJECT

private:
	typedef std::map< std::string, boost::shared_ptr<CSharedPaintItemList> > ITEM_LIST_MAP;
	typedef std::map< std::string, boost::shared_ptr<CPaintUser> > USER_MAP;
	typedef std::vector< boost::shared_ptr<CPaintSession> > SESSION_LIST;

public:
	CSharedPaintManager(void);
	~CSharedPaintManager(void);

protected slots:
	void onTimeoutSyncStart( void );

public:

	void initialize( const std::string &myId );	// TODO : initialize check

	const std::string& myId( void ) 
	{
		return myUserInfo_->userId();
	}

	void close( void )
	{
		// all clear session, item, status
		qDebug() << "CSharedPaintManager::close()";

		clearScreen( false );	// DO NOT NOTIFY TO OTHERS.. JUST TRIGGER CLEAR SCREEN EVENT
		clearAllItems();
		clearAllUsers();
		clearAllSessions();
		setEnabled( true );

		stopFindingServer();

		syncStartedFlag_ = false;
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
	bool startServer( int port = 0 );
	bool startFindingServer( void );
	void stopFindingServer( void );

	void setPaintChannel( const std::string & channel );
	void changeNickName( const std::string & nickName );

	void reconnect( void )
	{
		qDebug() << "CSharedPaintManager::reconnect()" << lastConnectAddress_.c_str() << lastConnectPort_ << lastConnectMode_ << isConnected() << isConnecting();

		if( isConnected() || isConnecting() )
			return;

		if( lastConnectAddress_.empty() || lastConnectPort_ < 0 )
			return;
		
		switch( lastConnectMode_ )
		{
		case PEER_MODE:
			_connectToPeer( lastConnectAddress_, lastConnectPort_ );
			break;
		case SERVER_MODE:
			_requestJoinServer( lastConnectAddress_, lastConnectPort_, myUserInfo_->channel() );
			break;
		default:
			break;
		}
	}

	bool connectToPeer( const std::string &addr, int port )
	{
		retryServerReconnectCount_ = 0;

		close();
		
		return _connectToPeer( addr, port );
	}

	bool requestJoinServer( const std::string &addr, int port, const std::string &roomid )
	{
		retryServerReconnectCount_ = 0;

		close();

		return _requestJoinServer( addr, port, roomid );
	}

	void clearAllSessions( void )
	{
		mutexSession_.lock();

		superPeerSession_ = boost::shared_ptr<CPaintSession>();
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
			//qDebug() << "sendDataToUsers() : Netmode = to superpeer";
			sessionList.push_back( superPeerSession_ );
		}
		else if( sessionList_.size() > 0 )
		{
			//qDebug() << "sendDataToUsers() : Netmode = I'm superpeer";
			sessionList = sessionList_;
		}
		else if( relayServerSession_ && relayServerSession_->session()->isConnected() )
		{
			//qDebug() << "sendDataToUsers() : Netmode = to relay server";
			sessionList.push_back( relayServerSession_ );
		}
		else
			return -1;

		return sendDataToUsers( sessionList, msg, toSessionId );
	}

	void sendChatMessage( const std::string &msg );	// channel chatting API

	void sendBroadCastTextMessage( const std::string &paintChannel, const std::string &msg );

	// Shared Paint Action
public:
	void setEnabled( bool enabled )
	{
		enabled_ = enabled;
	}

	void redoCommand( void )
	{
		if( ! enabled_ )
			return;

		commandMngr_.redoCommand();
	}

	void undoCommand( void )
	{
		if( ! enabled_ )
			return;

		commandMngr_.undoCommand();
	}

	bool deserializeData( const char * data, size_t size );	// TODO throw exception logic
	
	std::string serializeData( const std::string *target = NULL );

	boost::shared_ptr<CPaintItem> findPaintItem( const std::string & owner, int itemId )
	{
		return commandMngr_.findItem( owner, itemId );
	}

	bool addPaintItem( boost::shared_ptr<CPaintItem> item )
	{
		if( ! enabled_ )
			return false;

		item->setItemId( commandMngr_.generateItemId() );

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

	int notifyChangeCanvasScrollPos( int scrollH, int scrollV )
	{
		if( ! enabled_ )
			return -1;

		std::string msg = WindowPacketBuilder::CChangeCanvasScrollPos::make( scrollH, scrollV );
		lastScrollHPos_ = scrollH;
		lastScrollVPos_ = scrollV;
		return sendDataToUsers( msg );
	}
	
	int notifyResizingCanvas( int width, int height )
	{
		if( ! enabled_ )
			return -1;

		std::string msg = WindowPacketBuilder::CResizeCanvas::make( width, height );
		lastCanvasWidth_ = width;
		lastCanvasHeight_ = height;
		return sendDataToUsers( msg );
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

	int notifyResizingWindowSplitter( const std::vector<int> &sizes )
	{
		if( ! enabled_ )
			return -1;

		std::string msg = WindowPacketBuilder::CResizeWindowSplitter::make( sizes );

		lastWindowSplitterSizes_ = sizes;
		return sendDataToUsers( msg );
	}

private:
	void clearAllItems( void )	// this function must be called on main thread!
	{
		qDebug() << "CSharedPaintManager::clearAllItems()";

		assert( caller_.isMainThread() );

		backgroundColor_ = Qt::white;
		backgroundImageItem_ = boost::shared_ptr<CBackgroundImageItem>();
		canvas_->clearScreen();

		// all data clear
		commandMngr_.clear();
	}

	// Playback
public:
	bool isPlaybackMode( void ) { return commandMngr_.isPlaybackMode(); }

	size_t historyTaskCount( void ) { return commandMngr_.historyTaskCount(); }

	void plabackTo( int position )
	{
		commandMngr_.playbackTo( position );
	}

	void setAllowPainterToDraw( const std::string &userId, bool enabled )
	{
		commandMngr_.setAllowPainterToDraw( userId, enabled );
	}

	bool isAllowPainterToDraw( const std::string &userId )
	{
		return commandMngr_.isAllowPainterToDraw( userId );
	}

private:
	void sendAllSyncData( int toSessionId )
	{
		if( isAlwaysP2PMode() == false )
			return;

		std::string packetPackage;
		packetPackage += SystemPacketBuilder::CSyncStart::make( myUserInfo_->channel(), myUserInfo_->userId(), "" );
		packetPackage += serializeData();
		packetPackage += serializeJoinerList();
		packetPackage += SystemPacketBuilder::CSyncComplete::make( "" );

		sendDataToUsers( packetPackage, toSessionId );
	}

	// User Mansagement and Sync
public:
	int userCount( void )
	{
		return joinerMap_.size();
	}

	USER_LIST userList( void );

	USER_LIST historyUserList( void ) { return joinerHistory_; }

	boost::shared_ptr<CPaintUser> findHistoryUser( const std::string &userId )
	{
		boost::recursive_mutex::scoped_lock autolock(mutexUser_);

		USER_LIST::iterator it = joinerHistory_.begin();
		for( ; it != joinerHistory_.end(); it++ )
		{
			if( (*it)->userId() == userId )
				return *it;
		}
		return boost::shared_ptr<CPaintUser>();
	}

private:
	void sendMyUserInfo( CPaintSession* session )
	{
		std::string msg;
		if( isRelayServerSession( session ) )
			msg = SystemPacketBuilder::CJoinToServer::make( myUserInfo_ );
		else
			msg = SystemPacketBuilder::CJoinerToSuperPeer::make( myUserInfo_ );

		session->session()->sendData( msg );
	}

	void notifyRemoveUserInfo( boost::shared_ptr<CPaintUser> user )
	{
		qDebug() << "notifyRemoveUserInfo" << joinerMap_.size();
		std::string msg = SystemPacketBuilder::CLeftUser::make( user->channel(), user->userId() );
		sendDataToUsers( msg );
	}

	bool addUser( boost::shared_ptr<CPaintUser> user )
	{
		mutexUser_.lock();
		boost::shared_ptr<CPaintUser> historyUser = findHistoryUser( user->userId() );
		if( historyUser )
		{
			historyUser->setData( user->data() );
			user = historyUser;
		}
		else
			addHistoryUser( user );

		bool firstFlag = true;

		std::pair<USER_MAP::iterator, bool> res = joinerMap_.insert( USER_MAP::value_type( user->userId(), user ) );
		if( !res.second )
		{
			res.first->second->setData( user->data() );	// overwrite;
			firstFlag = false;
		}
		mutexUser_.unlock();

		if( !user->isMyself() )
		{
			if( firstFlag )
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_EnterPaintUser, this, user ) );
			else
				caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_UpdatePaintUser, this, user ) );
		}

		return firstFlag;
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
			caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_LeavePaintUser, this, removing ) );
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

	void addHistoryUser( boost::shared_ptr<CPaintUser> user )
	{
		boost::recursive_mutex::scoped_lock autolock(mutexUser_);
		boost::shared_ptr<CPaintUser> res = findHistoryUser( user->userId() );
		if( !res )
		{
			joinerHistory_.push_back( user );

			commandMngr_.addPainter( user->userId() );
		}
	}

	void clearAllUsers( void )
	{
		boost::recursive_mutex::scoped_lock autolock(mutexUser_);
		joinerMap_.clear();
		joinerHistory_.clear();

		addUser( myUserInfo_ );
	}

	std::string serializeHistoryJoinerList( void )
	{
		std::string allData;

		// User Info
		allData = SystemPacketBuilder::CHistoryUserList::make( joinerHistory_ );

		return allData;
	}

	std::string serializeJoinerList( void )
	{
		std::string allData;

		// User Info
		mutexUser_.lock();
		USER_MAP::iterator it = joinerMap_.begin();
		for( ; it != joinerMap_.end(); it++ )
		{
			allData += SystemPacketBuilder::CJoinerToSuperPeer::make( it->second );
		}
		mutexUser_.unlock();

		return allData;
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
	void stopServer( void );
	void _stopFindingServer( void );
	bool startListenBroadCast( void );
	void stopListenBroadCast( void );

	bool _connectToSuperPeer( boost::shared_ptr<CPaintUser> user )
	{
		boost::shared_ptr<CNetPeerSession> session = NetServiceRunnerPtr()->newSession();
		boost::shared_ptr<CPaintSession> userSession = boost::shared_ptr<CPaintSession>(new CPaintSession(session, this));

		mutexSession_.lock();
		superPeerSession_ = userSession;
		mutexSession_.unlock();

		// must be called here for preventing from a crash by thread race condition.

		userSession->session()->connect( user->viewIPAddress(), user->listenTcpPort() );

		return true;
	}

	bool _connectToPeer( const std::string &addr, int port )
	{
		if( port == listenTcpPort_ )
			return false;

		boost::shared_ptr<CNetPeerSession> session = NetServiceRunnerPtr()->newSession();
		boost::shared_ptr<CPaintSession> userSession(new CPaintSession(session, this));

		mutexSession_.lock();
		sessionList_.push_back( userSession );
		mutexSession_.unlock();

		// must be called here for preventing from a crash by thread race condition.
		userSession->session()->connect( addr, port );

		lastConnectMode_ = PEER_MODE;
		lastConnectAddress_ = addr;
		lastConnectPort_ = port;
		return true;
	}

	bool _requestJoinServer( const std::string &addr, int port, const std::string &roomid )
	{
		boost::shared_ptr<CNetPeerSession> session = NetServiceRunnerPtr()->newSession();
		boost::shared_ptr<CPaintSession> userSession(new CPaintSession(session, this));

		mutexSession_.lock();
		relayServerSession_ = userSession;
		mutexSession_.unlock();

		// must be called here for preventing from a crash by thread race condition.
		userSession->session()->connect( addr, port );

		lastConnectMode_ = SERVER_MODE;
		lastConnectAddress_ = addr;
		lastConnectPort_ = port;
		return true;
	}

	void dispatchBroadCastPacket( CNetBroadCastSession *session, boost::shared_ptr<CPacketData> packetData );
	void dispatchUdpPacket( CNetUdpSession *session, boost::shared_ptr<CPacketData> packetData );
	bool dispatchPaintPacket( CPaintSession *session, boost::shared_ptr<CPacketData> packetData );

	inline bool isRelayServerSession( CPaintSession* session )
	{
		if ( relayServerSession_ && session->sessionId() == relayServerSession_->sessionId() )
			return true;
		return false;
	}

	inline bool isSuperPeerSession( CPaintSession* session )
	{
		if ( superPeerSession_ && session->sessionId() == superPeerSession_->sessionId() )
			return true;
		return false;
	}

	inline bool isMySelfSuperPeer( void ) 
	{
		if( ! superPeerSession_ && superPeerId_ == myUserInfo_->userId() )
			return true;
		return false;
	}

	inline bool isRelayServerMode( void )
	{
		return relayServerSession_ != NULL;
	}

	inline bool isAlwaysP2PMode( void )
	{
		return relayServerSession_ == NULL;
	}

	void _requestSyncData( void );

	void tryReconnectToRelayServer( CPaintSession* unconnectedSession = NULL )
	{
		if( ! unconnectedSession || isRelayServerSession( unconnectedSession ) )	// if only relay server session, try reconnect
		{
			if( retryServerReconnectCount_++ < DEFAULT_RECONNECT_TRY_COUNT )
			{
				//CDefferedCaller::singleShotAfterMiliseconds( boost::bind(&CSharedPaintManager::reconnect, this), 1000 );
				caller_.performMainThreadAfterMilliseconds( boost::bind(&CSharedPaintManager::reconnect, this), 2000 );
			}
		}
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
	}
	void fireObserver_DisConnected( void )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_Disconnected( this );
		}
	}
	void fireObserver_ShowErrorMessage( const std::string &error )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ShowErrorMessage( this, error );
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
	void fireObserver_ChangeCanvasScrollPos( int posH, int posV )
	{
		lastScrollHPos_ = posH;
		lastScrollVPos_ = posV;

		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ChangeCanvasScrollPos( this, posH, posV );
		}
	}
	void fireObserver_ResizeCanvas( int width, int height )
	{
		lastCanvasWidth_ = width;
		lastCanvasHeight_ = height;

		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ResizeCanvas( this, width, height );
		}
	}
	void fireObserver_ResizeWindowSplitter( std::vector<int> &sizes )
	{
		lastWindowSplitterSizes_ = sizes;

		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ResizeWindowSplitter( this, sizes );
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
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ClearScreen( this );
		}

		// clear here for giving a chance to handle current item data.
		clearAllItems();
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
	void fireObserver_ReceivedBroadcastTextMessage( const std::string &paintChannel, const std::string &fromId, const std::string &nickName, const std::string &message )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ReceivedBroadcastTextMessage( this, paintChannel, fromId, nickName, message );
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
	void fireObserver_EnterPaintUser( boost::shared_ptr<CPaintUser> user )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_EnterPaintUser( this, user );
		}
	}
	void fireObserver_LeavePaintUser( boost::shared_ptr<CPaintUser> user )
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_LeavePaintUser( this, user );
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

	void fireObserver_ChangedNickName( const std::string & userId, const std::string &prevNickName, const std::string &currNickName ) 
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ChangedNickName( this, userId, prevNickName, currNickName );
		}
	}

	void fireObserver_ReceivedChatMessage( const std::string & userId, const std::string &nickName, const std::string &chatMsg ) 
	{
		std::list<ISharedPaintEvent *> observers = observers_;
		for( std::list<ISharedPaintEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onISharedPaintEvent_ReceivedChatMessage( this, userId, nickName, chatMsg );
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

		if( isRelayServerMode() )
		{
			if( isMySelfSuperPeer() == false )
			{
				session->close();
				return;
			}
		}

		mutexSession_.lock();
		sessionList_.push_back( userSession );
		mutexSession_.unlock();

		if( isAlwaysP2PMode() )
			superPeerId_ = myUserInfo_->userId();

		// start read io!
		session->start();
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
	virtual void onIPaintSessionEvent_Connected( CPaintSession* session )
	{
		if( isSuperPeerSession(session) )
		{
			_requestSyncData();
		}

		if( isRelayServerSession( session ) || isSuperPeerSession( session ) || isAlwaysP2PMode() )
		{
			// send to my user info to relay server or super peer
			sendMyUserInfo( session );

			if( isRelayServerSession( session ) )
			{
				retryServerReconnectCount_ = 0;
			}
		}
		else
		{
			// I'm superpeer, <session> is that connected to me.
		}

		if( isAlwaysP2PMode() && isMySelfSuperPeer() )
			sendAllSyncData( session->sessionId() );

		if ( isRelayServerSession( session ) || isAlwaysP2PMode() )
		{
			caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_Connected, this, session->sessionId() ) );
		}
		else 
		{
			// when connected to superpeer, do not send my info to him.
		}
	}

	virtual void onIPaintSessionEvent_ConnectFailed( CPaintSession* session )
	{
		tryReconnectToRelayServer( session );

		if ( isRelayServerSession( session ) )
		{
			caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_ConnectFailed, this ) );
		}

		removeSession( session->sessionId() );
	}

	virtual void onIPaintSessionEvent_Disconnected( CPaintSession * session )
	{
		tryReconnectToRelayServer( session );

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

	virtual void onIPaintSessionEvent_ReceivedPacket( CPaintSession * session, const boost::shared_ptr<CPacketData> data )
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
			if( (*it)->sessionId() == session->sessionId() )
			{
				list.erase( it );
				break;
			}
		}

		std::string msg = CommonPacketBuilder::makePacket( data->code, data->body );
		sendDataToUsers( list, msg );
	}

	virtual void onIPaintSessionEvent_SendingPacket( CPaintSession * session, const boost::shared_ptr<CNetPacketData> packet )
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
				if( (*itD).session == session )
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

		caller_.performMainThread( boost::bind( &CSharedPaintManager::fireObserver_SendingPacket, this, packet->packetId(), wroteBytes, totalBytes ) );
	}

private:
	friend class CSharedPaintCommandManager;
	friend class CAddItemTask;
	friend class CRemoveItemTask;
	friend class CUpdateItemTask;
	friend class CMoveItemTask;

	CDefferedCaller caller_;
	bool enabled_;
	bool syncStartedFlag_;

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
	int lastCanvasWidth_;
	int lastCanvasHeight_;
	boost::int16_t lastScrollHPos_;
	boost::int16_t lastScrollVPos_;
	std::vector<int> lastWindowSplitterSizes_;
	QColor backgroundColor_;
	int gridLineSize_;

	// user management
	boost::recursive_mutex mutexUser_;
	boost::shared_ptr<CPaintUser> myUserInfo_;
	USER_MAP joinerMap_;
	USER_LIST joinerHistory_;

	// network
	enum ConnectionMode
	{
		INIT_MODE,
		PEER_MODE,
		SERVER_MODE
	};

	bool findingServerMode_;
	int listenTcpPort_;
	int listenUdpPort_;
	int retryServerReconnectCount_;
	ConnectionMode lastConnectMode_;
	std::string lastConnectAddress_;
	int lastConnectPort_;
	std::string superPeerId_;
	SESSION_LIST sessionList_;
	boost::shared_ptr<CPaintSession> superPeerSession_;
	boost::shared_ptr<CPaintSession> relayServerSession_;
	boost::recursive_mutex mutexSession_;
	boost::shared_ptr<CNetPeerServer> netPeerServer_;
	boost::shared_ptr< CNetUdpSession > udpSessionForConnection_;
	boost::shared_ptr< CNetBroadCastSession > broadCastSessionForListener_;
	boost::shared_ptr< CNetBroadCastSession > broadCastSessionForFinder_;
	boost::shared_ptr< CNetBroadCastSession > broadCastSessionForSendMessage_;
	boost::shared_ptr< CNetBroadCastSession > broadCastSessionForRecvMessage_;

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
