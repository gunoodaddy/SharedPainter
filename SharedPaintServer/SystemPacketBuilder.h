#pragma once

#include "Coconut.h"
#include "PacketBuffer.h"
#include "SharedPaintCodeDefine.h"
#include "SharedPaintProtocol.h"

namespace SystemPacketBuilder {

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
					data.code = CODE_SYSTEM_SYNC_START;
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
			static boost::shared_ptr<SharedPaintProtocol> make( const std::string &channel, const std::string &joinerList )
			{
				int pos = 0;
				try
				{
					boost::shared_ptr<SharedPaintProtocol> prot(new SharedPaintProtocol);

					std::string body;
					pos += PacketBufferUtil::writeString8( body, pos, channel );
					pos += PacketBufferUtil::writeBinary( body, pos, joinerList.c_str(), joinerList.size() );

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

/*
			static bool parse( const std::string &body, std::string &channel, std::string &userId )
			{
				int pos = 0;
				try
				{
					pos += PacketBufferUtil::readString8( body, pos, channel );
					pos += PacketBufferUtil::readString8( body, pos, userId );
					return true;

				}catch(...)
				{
				}
				return false;
			}
			*/
	};
};
