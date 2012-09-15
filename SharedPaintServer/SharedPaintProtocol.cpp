#include <Coconut.h>
#include "SharedPaintProtocol.h"

SharedPaintProtocol::~SharedPaintProtocol() { 
	LOG_TRACE("~SharedPaintProtocol() : %p", this);
}

bool SharedPaintProtocol::processRead(boost::shared_ptr<BaseVirtualTransport> transport) {
	LOG_TRACE("processRead() : state : %d", state_);

	try {
		switch(state_) {
			case Init:
				if(Init == state_) {
					state_ = MagicCode;
				}
			case MagicCode: 
				if(MagicCode == state_) {
					startRead_pos_ = readBuffer_->readPos();
					currHeaderLen_ = 0;
					if( readBuffer_->remainingSize() < 2 )
						break;

					currHeaderLen_ += readBuffer_->readInt16(currHeaderData_.magic);

					if( (boost::uint16_t)SharedPaintHeader::NET_MAGIC_CODE_BIG == (boost::uint16_t)currHeaderData_.magic ) {
						state_ = Code;
					} else {
						// Invalid packet..
						// will be closed!
						invalidPacketRecved_ = true;
						break;
					}
				}
			case Code: 
				if(Code == state_) {

					if( readBuffer_->remainingSize() < 2 )
						break;

					currHeaderLen_ += readBuffer_->readInt16(currHeaderData_.code);

					currIdLen_ = 0;
					state_ = FromId;
				}
			case FromId: 
				if(FromId == state_) {

					if( currIdLen_ <= 0 ) {
						if( readBuffer_->remainingSize() < 1 )
							break;
						currHeaderLen_ += readBuffer_->readInt8(currIdLen_);
					}

					if( currIdLen_ > 0 ) {
						if( readBuffer_->remainingSize() < (size_t)currIdLen_ )
							break;
						currHeaderLen_ += readBuffer_->read(currHeaderData_.fromId, currIdLen_);
					}
					currIdLen_ = 0;
					state_ = ToId;
				}
			case ToId: 
				if(ToId == state_) {

					if( currIdLen_ <= 0 ) {
						if( readBuffer_->remainingSize() < 1 )
							break;
						currHeaderLen_ += readBuffer_->readInt8(currIdLen_);
					}

					if( currIdLen_ > 0 ) {
						if( readBuffer_->remainingSize() < (size_t)currIdLen_ )
							break;
						currHeaderLen_ += readBuffer_->read(currHeaderData_.toId, currIdLen_);
					}
					currIdLen_ = 0;
					state_ = BodyLength;
				}
			case BodyLength: 
				if(BodyLength == state_) {
					if( readBuffer_->remainingSize() < 4 )
						break;

					//logger::hexdump( (unsigned char*)readBuffer_->currentPtr(), readBuffer_->remainingSize(), stdout);                       
					currHeaderLen_ += readBuffer_->readInt32(currHeaderData_.blen);

					state_ = Payload;

					// Header Complete
					header_.setHeaderLength( currHeaderLen_ );
					header_.setData( currHeaderData_ );

					payload_pos_ += currHeaderLen_;
				}
			case Payload: 
				if(Payload == state_) {
					// readBuffer_ is BaseProtocol's readBuffer(or payload buffer)
					// already tcp layer read and append to readBuffer_
					// you just verify this readBuffer_..
					int remain = header_.totalLength() - readBuffer_->totalSize();
					if(remain > 0)
						break; // need mode data..

					state_ = Complete;
				}
			case Complete:
				if(Complete == state_) {
					initHeader_ = header_;
					payload_pos_ = readBuffer_->readPos();
					//logger::hexdump((const unsigned char*)payloadPtr(), payloadSize(), stdout);

					//printf("TODO DELETE ME gametalk payload receved..\n");
					return true;
				}
		} 
	} catch(ProtocolException &e) {
		(void)e;
		// nothing to do..
	} catch(SocketException &e) {
		(void)e;
		// nothing to do..
	}
	return false;
}


bool SharedPaintProtocol::processSerialize(size_t bufferSize) {
	resetWritingBuffer();

	header_.serialize(writebuffer_, bufferSize + payload_.size());
	writebuffer_->write(payload_.c_str(), payload_.size());

	return true;
}


const void * SharedPaintProtocol::remainingBufferPtr() {
	if(isReadComplete())
		return (const char*)readBuffer_->currentPtr() + (initHeader_.totalLength() - payload_pos_ - readPayloadSize());
	return BaseProtocol::remainingBufferPtr();
}


size_t SharedPaintProtocol::remainingBufferSize() {
	if(isReadComplete()) {
		return readBuffer_->remainingSize() - (initHeader_.totalLength() - payload_pos_ - readPayloadSize());
	}
	return BaseProtocol::remainingBufferSize();
}

const void * SharedPaintProtocol::basePtr() {
	if(isReadComplete()) {
		return (const char*)readBuffer_->basePtr() + startRead_pos_;
	} else if( writebuffer_->remainingSize() > 0 ) {
		return writebuffer_->basePtr();		
	}
	return NULL;
}

size_t SharedPaintProtocol::totalSize() {
	if(isReadComplete()) {
		return header_.totalLength();
	} else if( writebuffer_->remainingSize() > 0 ) {
		return writebuffer_->remainingSize();		
	}
	return 0;
}


