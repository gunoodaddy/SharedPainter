#pragma once

#include "Coconut.h"
#include "PacketBuffer.h"
#include "SharedPaintCodeDefine.h"
#include "SharedPaintProtocol.h"
#include "SharedPaintClient.h"

namespace SystemPacketBuilder {

	class ChangeSuperPeer{
		public:
			static boost::shared_ptr<SharedPaintProtocol> make( boost::shared_ptr<SharedPaintClient> superPeer ) {

				int pos = 0;
				try
				{
					boost::shared_ptr<SharedPaintProtocol> prot(new SharedPaintProtocol);

					std::string body;
					pos += PacketBufferUtil::writeString8( body, pos, superPeer->user()->userId() );

					SharedPaintHeader::HeaderData data;
					data.code = CODE_SYSTEM_SUPERPEER_CHANGED;
					prot->header().setData( data );
					prot->setPayload( body.c_str(), body.size() );
					prot->processSerialize();
					return prot;
				}catch(...)
				{
				}
				return boost::shared_ptr<SharedPaintProtocol>();
			}
	};

	class RequestSync {
		public:
			static boost::shared_ptr<SharedPaintProtocol> make( const std::string &channel, const std::string &runner, const std::string &target )
			{
				int pos = 0;
				try
				{
					boost::shared_ptr<SharedPaintProtocol> prot(new SharedPaintProtocol);

					std::string body;
					pos += PacketBufferUtil::writeString8( body, pos, channel );
					pos += PacketBufferUtil::writeString8( body, pos, target );

					SharedPaintHeader::HeaderData data;
					data.code = CODE_SYSTEM_SYNC_REQUEST;
					data.toId = runner;
					prot->header().setData( data );
					prot->setPayload( body.c_str(), body.size() );
					prot->processSerialize();
					return prot;
				}catch(...)
				{
				}
				return boost::shared_ptr<SharedPaintProtocol>();
			}

	};

	class ResponseJoin {
		public:
			static boost::shared_ptr<SharedPaintProtocol> make( const std::string &channel, const std::string &joinerList, boost::shared_ptr<SharedPaintClient> superPeerSession )
			{
				int pos = 0;
				try
				{
					boost::shared_ptr<SharedPaintProtocol> prot(new SharedPaintProtocol);

					std::string body;
					pos += PacketBufferUtil::writeString8( body, pos, channel );
					pos += PacketBufferUtil::writeBinary( body, pos, joinerList.c_str(), joinerList.size() );
					pos += PacketBufferUtil::writeString8( body, pos, superPeerSession ? superPeerSession->user()->userId() : "");

					SharedPaintHeader::HeaderData data;
					data.code = CODE_SYSTEM_RES_JOIN;
					prot->header().setData( data );
					prot->setPayload( body.c_str(), body.size() );
					prot->processSerialize();
					return prot;

				}catch(...)
				{
				}
				return boost::shared_ptr<SharedPaintProtocol>();
			}
	};

	class NewJoiner {
		public:
			static boost::shared_ptr<SharedPaintProtocol> make( boost::shared_ptr<SharedPaintClient> client )
			{
				try
				{
					boost::shared_ptr<SharedPaintProtocol> prot(new SharedPaintProtocol);

					std::string body = client->user()->serialize();

					SharedPaintHeader::HeaderData data;
					data.code = CODE_SYSTEM_JOIN_SERVER;
					prot->header().setData( data );
					prot->setPayload( body.c_str(), body.size() );
					prot->processSerialize();
					return prot;
				}catch(...)
				{
				}
				return boost::shared_ptr<SharedPaintProtocol>();
			}
	};

	class LeftUser {
		public:
			static boost::shared_ptr<SharedPaintProtocol> make( const std::string &channel, const std::string &userId )
			{
				int pos = 0;
				try
				{
					boost::shared_ptr<SharedPaintProtocol> prot(new SharedPaintProtocol);

					std::string body;
					pos += PacketBufferUtil::writeString8( body, pos, channel );
					pos += PacketBufferUtil::writeString8( body, pos, userId );

					SharedPaintHeader::HeaderData data;
					data.code = CODE_SYSTEM_LEFT;
					prot->header().setData( data );
					prot->setPayload( body.c_str(), body.size() );
					prot->processSerialize();
					return prot;
				}catch(...)
				{
				}
				return boost::shared_ptr<SharedPaintProtocol>();
			}
	};
};
