#include <Coconut.h>
#include "SharedPaintManager.h"
#include "SharedPaintClient.h"
#include "SharedPaintController.h"
#include "SystemPacketBuilder.h"

#define TCP_CHECK_TIMER	1111
#define TCP_CHECK_DEADLINE_MSEC	3000 // 3sec
#define SELF_PTR boost::static_pointer_cast<SharedPaintClient>(shared_from_this())

IOServiceContainer *SharedPaintClient::gIOServiceContainer_;
boost::shared_ptr<SharedPaintController::SharedPaintProtocolFactory> SharedPaintClient::gProtocolFactory_;

SharedPaintClient::SharedPaintClient() : invalidSessionFlag_(false) {
	LOG_TRACE("SharedPaintClient() %p\n", this);
	user_ =  boost::shared_ptr<CPaintUser>(new CPaintUser);
}

SharedPaintClient::~SharedPaintClient() {
	LOG_TRACE("~SharedPaintClient() %p\n", this);
}

void SharedPaintClient::checkIfSuperPeer() {

	if( user()->isSuperPeerCandidate() ) {

		SharedPaintManagerPtr()->setSuperPeerSession( SELF_PTR );
	}

	boost::shared_ptr<TcpTestClient> newClient(new TcpTestClient);
	NetworkHelper::connectTcp(gIOServiceContainer_, user()->viewIPAddress().c_str(), user()->listenTcpPort(), newClient);

	testClient_ = newClient;
	setTimer( TCP_CHECK_TIMER, TCP_CHECK_DEADLINE_MSEC, false );
}

 
void SharedPaintClient::onTimer(unsigned short id) {
	if( TCP_CHECK_TIMER != id )
		return;
	
	// clear!
	testClient_ = boost::shared_ptr<TcpTestClient>();
}

void SharedPaintClient::_handle_CODE_SYSTEM_JOIN_TO_SERVER(boost::shared_ptr<SharedPaintProtocol> prot) {

	std::string body( (char *)prot->payloadBuffer()->currentPtr(), prot->payloadBuffer()->remainingSize() );
	user_->deserialize( body );
	user_->setViewIPAddress( tcpSocket()->peerAddress()->ip() );

	bool firstFlag = false;
	SharedPaintManagerPtr()->joinRoom( SELF_PTR, firstFlag );
	
	// roomcast this packet => notify new joiner
	boost::shared_ptr<SharedPaintProtocol> notiProt = SystemPacketBuilder::NewJoiner::make( SELF_PTR );
	SharedPaintManagerPtr()->roomCast( user_->roomId(), user_->userId(), notiProt, false );

	checkIfSuperPeer();

	// make response
	std::string userlist = SharedPaintManagerPtr()->serializeJoinerInfoPacket( user_->roomId() );
	boost::shared_ptr<SharedPaintClient> superPeerSession = SharedPaintManagerPtr()->currentSuperPeerSession( user_->roomId() );
	boost::shared_ptr<SharedPaintProtocol> resProt 
		= SystemPacketBuilder::ResponseJoin::make( user_->roomId(), firstFlag, userlist, superPeerSession );
	resProt->processWrite( tcpSocket() );
}


void SharedPaintClient::_handle_CODE_SYSTEM_LEAVE(boost::shared_ptr<SharedPaintProtocol> prot) {

	SharedPaintManagerPtr()->leaveRoom( SELF_PTR );
}

void SharedPaintClient::_handle_CODE_SYSTEM_CHANGE_NICKNAME(boost::shared_ptr<SharedPaintProtocol> prot) {

	std::string body( (char *)prot->payloadBuffer()->currentPtr(), prot->payloadBuffer()->remainingSize() );

	std::string userid, nickname;
	SystemPacketBuilder::ChangeNickName::parse( body, userid, nickname );

	boost::shared_ptr<SharedPaintClient> client = SharedPaintManagerPtr()->findUser( user_->roomId(), userid );
	if( !client )
		return;
	client->lock();
	client->user()->setNickName( nickname );
	client->unlock();

	// relay!
	if( prot->header().toId().empty() )
		SharedPaintManagerPtr()->roomCast( user_->roomId(), user_->userId(), prot, false );
	else
		SharedPaintManagerPtr()->uniCast( user_->roomId(), prot );
}

void SharedPaintClient::_handle_CODE_SYSTEM_SYNC_REQUEST(boost::shared_ptr<SharedPaintProtocol> prot) {

	// choose paint data sync runner by round robin for me..
	SharedPaintManagerPtr()->syncStart( user_->roomId(), user_->userId() );
}

void SharedPaintClient::_handle_CODE_SYSTEM_TCPACK(boost::shared_ptr<SharedPaintProtocol> prot) {
	// setting super peer
	LOG_INFO("------------------- SUPER PEER OK! -------------------- : %s:%d", tcpSocket()->peerAddress()->ip(), user()->listenTcpPort() );

	user_->setSuperPeerCandidate();

	SharedPaintManagerPtr()->setSuperPeerSession( SELF_PTR );
}


void SharedPaintClient::onClosed( void ) {
	if( invalidSessionFlag_ )
		return;
	SharedPaintManagerPtr()->leaveRoom( SELF_PTR );
}

void SharedPaintClient::onError(int error, const char *strerror) {
	if( invalidSessionFlag_ )
		return;
	SharedPaintManagerPtr()->leaveRoom( SELF_PTR );
}

void SharedPaintClient::onSharedPaintReceived(boost::shared_ptr<SharedPaintProtocol> prot) {

	const SharedPaintHeader &header = prot->header();

	LOG_TRACE(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> !!onSharedPaintReceived() %x, %p\n", header.code(),  this);

	switch( prot->code() )
	{
		case CODE_SYSTEM_JOIN_TO_SERVER:
			_handle_CODE_SYSTEM_JOIN_TO_SERVER( prot );
			break;
		case CODE_SYSTEM_LEFT:
			_handle_CODE_SYSTEM_LEAVE( prot );
			break;
		case CODE_SYSTEM_TCPACK:
			_handle_CODE_SYSTEM_TCPACK( prot );
			break;
		case CODE_SYSTEM_SYNC_REQUEST:
			_handle_CODE_SYSTEM_SYNC_REQUEST( prot );
			break;
		case CODE_SYSTEM_CHANGE_NICKNAME:
			_handle_CODE_SYSTEM_CHANGE_NICKNAME( prot );
			break;
		default:
			{
				// just relay!
				if( prot->header().toId().empty() )
					SharedPaintManagerPtr()->roomCast( user_->roomId(), user_->userId(), prot, false );
				else
					SharedPaintManagerPtr()->uniCast( user_->roomId(), prot );
				break;
			}
	}
}

