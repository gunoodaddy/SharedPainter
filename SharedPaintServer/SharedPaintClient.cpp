#include <Coconut.h>
#include "SharedPaintManager.h"
#include "SharedPaintClient.h"
#include "SharedPaintController.h"
#include "SystemPacketBuilder.h"

boost::shared_ptr<SharedPaintController::SharedPaintProtocolFactory> SharedPaintClient::gProtocolFactory_;

SharedPaintClient::SharedPaintClient() : invalidSessionFlag_(false) {
	LOG_TRACE("SharedPaintClient() %p\n", this);
	user_ =  boost::shared_ptr<CPaintUser>(new CPaintUser);
}

SharedPaintClient::~SharedPaintClient() {
	LOG_TRACE("~SharedPaintClient() %p\n", this);
}


void SharedPaintClient::_handle_CODE_SYSTEM_JOIN(boost::shared_ptr<SharedPaintProtocol> prot) {

	// packet parse
	SPaintUserInfoData userInfo;
	prot->payloadBuffer()->readString8( userInfo.roomId );
	prot->payloadBuffer()->readString8( userInfo.userId );

	user_->setData( userInfo );

	SharedPaintManagerPtr()->joinRoom( boost::static_pointer_cast<SharedPaintClient>(shared_from_this()) );
	
	// roomcast this packet => notify new joiner
	SharedPaintManagerPtr()->roomCast( user_->roomId(), user_->userId(), prot, false );

	// choose paint data sync runner by round robin for me..
	SharedPaintManagerPtr()->syncStart( user_->roomId(), user_->userId() );

	// make response
	std::string userlist = SharedPaintManagerPtr()->generateJoinerInfoPacket( userInfo.roomId );
	boost::shared_ptr<SharedPaintProtocol> resProt = SystemPacketBuilder::ResponseJoin::make( userInfo.roomId, userlist );
	resProt->processWrite( tcpSocket() );
}


void SharedPaintClient::_handle_CODE_SYSTEM_LEAVE(boost::shared_ptr<SharedPaintProtocol> prot) {

	// packet parse
	SPaintUserInfoData userInfo;
	prot->payloadBuffer()->readString8( userInfo.roomId );
	prot->payloadBuffer()->readString8( userInfo.userId );

	SharedPaintManagerPtr()->leaveRoom( userInfo.roomId, userInfo.userId );
}

void SharedPaintClient::onClosed( void ) {
	if( invalidSessionFlag_ )
		return;
	SharedPaintManagerPtr()->leaveRoom( user_->roomId(), user_->userId() );
}

void SharedPaintClient::onError(int error, const char *strerror) {
	if( invalidSessionFlag_ )
		return;
	SharedPaintManagerPtr()->leaveRoom( user_->roomId(), user_->userId() );
}

void SharedPaintClient::onSharedPaintReceived(boost::shared_ptr<SharedPaintProtocol> prot) {

	const SharedPaintHeader &header = prot->header();

	LOG_TRACE("!!onSharedPaintReceived() %x, %p\n", header.code(),  this);

	switch( prot->code() )
	{
		case CODE_SYSTEM_JOIN:
			_handle_CODE_SYSTEM_JOIN( prot );
			break;
		case CODE_SYSTEM_LEFT:
			_handle_CODE_SYSTEM_LEAVE( prot );
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

