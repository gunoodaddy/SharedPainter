#pragma once

#include <string>
#include <vector>
#if ! defined(COCONUT_USE_PRECOMPILE)
#include <boost/shared_ptr.hpp>
#endif

using namespace coconut;
using namespace coconut::protocol;

#include "ProtocolManager.h"

class SharedPaintHeader {
public:
	SharedPaintHeader() { }

	enum {
		NET_MAGIC_CODE = 0xBE,
		NET_MAGIC_CODE_BIG = 0xBEBE,
	};

	struct HeaderData {
		HeaderData() : magic(NET_MAGIC_CODE_BIG), code(0), blen(0) { }
		boost::int16_t magic;
		boost::int16_t code;
		std::string fromId;
		std::string toId;
		boost::int32_t blen;
	};

	boost::uint16_t code() const {
		return data_.code;
	}
	const std::string fromId( void ) const { return data_.fromId; }
	const std::string toId( void ) const { return data_.toId; }

	void setToId( const std::string &id ) { data_.toId = id; }

	int totalLength() const {
		return data_.blen + hlen_;
	}

	void setHeaderLength( int hlen ) { hlen_ = hlen; }

	void setData( const struct HeaderData &data ) { data_ = data; }


	void serialize(boost::shared_ptr<BufferedTransport> buffer, size_t payloadSize) {
		
		VirtualTransportHelper::writeInt16(buffer, data_.magic);
		VirtualTransportHelper::writeInt16(buffer, data_.code);
		VirtualTransportHelper::writeString8(buffer, data_.fromId);
		VirtualTransportHelper::writeString8(buffer, data_.toId);
		VirtualTransportHelper::writeInt32(buffer, payloadSize);
	}

private:
	boost::uint32_t hlen_;	
	struct HeaderData data_;
};

class SharedPaintProtocol : public BaseProtocol {
public:
	enum State{
		Init,
		MagicCode,
		Code,
		FromId,
		ToId,
		BodyLength,
		Payload,
		Complete
	};

	SharedPaintProtocol() : header_()
			, initHeader_()
			, state_(Init)
			, payload_pos_(0)
			, invalidPacketRecved_(false) 
			, manager_(NULL)
	{ 
		LOG_TRACE("SharedPaintProtocol() : %p", this);
	}

	~SharedPaintProtocol();

	const char* className() {
		return "SharedPaintProtocol";
	}

	bool isReadComplete() {
		return state_ == Complete;
	}

	virtual bool processSerialize(size_t bufferSize = 0);
	virtual bool processRead(boost::shared_ptr<BaseVirtualTransport> transport);

	virtual const void * remainingBufferPtr();
	virtual size_t remainingBufferSize();
	virtual bool isInvalidPacketReceived() {
		return invalidPacketRecved_;
	}

private:
	size_t readPayloadSize() {
		return readBuffer_->readPos() - payload_pos_;
	}

public:
	const SharedPaintHeader &const_header() {
		return header_;
	}

	SharedPaintHeader &header() {
		return header_;
	}

	boost::uint32_t code() {
		return header_.code();
	}

	const void * basePtr();
	size_t totalSize();

	void setManager(ProtocolManager * manager) {
		manager_ = manager;
	}

	ProtocolManager * manager() {
		return manager_;
	}

	void setPayload(const void *payload, size_t size) {
		payload_.assign((char *)payload, size);
	}

private:
	struct SharedPaintHeader::HeaderData currHeaderData_;
	SharedPaintHeader header_;
	SharedPaintHeader initHeader_;
	State state_;
	std::string payload_;
	size_t payload_pos_;
	size_t startRead_pos_;
	bool invalidPacketRecved_;
	ProtocolManager * manager_;
	boost::int8_t currIdLen_;
	int currHeaderLen_;
};

