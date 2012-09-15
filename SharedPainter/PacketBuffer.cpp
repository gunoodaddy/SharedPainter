#include "StdAfx.h"
#include "PacketBuffer.h"

bool CPacketBuffer::littleEndianMode = true;

CPacketBuffer::CPacketBuffer() : readPos_(0) { 
}

CPacketBuffer::~CPacketBuffer() {
}

int CPacketBuffer::erase(size_t pos, size_t size) {
	if(buffer_.size() < (pos + size))
		throw CPacketException("erase size greater than buffer size");
	buffer_.erase(pos, size);
	return size;
}

int CPacketBuffer::insertInt8(size_t pos, boost::uint8_t value) {
	boost::uint8_t *ptr = (boost::uint8_t*)&value;
	buffer_.insert(pos, (const char*)ptr, 1);
	return 1;
}

int CPacketBuffer::write(const char* ptr, size_t size) {
	return write((const void *)ptr, size);
}

int CPacketBuffer::write(const void* ptr, size_t size) {
	boost::recursive_mutex::scoped_lock autolock( lock_ );

	buffer_.append((const char*)ptr, size);
	return size;
}

int CPacketBuffer::read(std::string &data, size_t size) {
	boost::recursive_mutex::scoped_lock autolock( lock_ );

	if(remainingSize() < size) {
		size = remainingSize();
	}
	data.assign((const char *)currentPtr(), size);
	fastforward(size);
	return size;
}

int CPacketBuffer::read(void *ptr, size_t size) {
	boost::recursive_mutex::scoped_lock autolock( lock_ );
	if(remainingSize() < size) {
		size = remainingSize();
	}
	memcpy(ptr, currentPtr(), size);
	fastforward(size);
	return size;
}

int CPacketBuffer::peek(char *buffer, size_t size) {
	boost::recursive_mutex::scoped_lock autolock( lock_ );
	if(remainingSize() < size) {
		size = remainingSize();
	}
	memcpy(buffer, currentPtr(), size);
	return size;
}

const void * CPacketBuffer::peek(size_t &size) {
	boost::recursive_mutex::scoped_lock autolock( lock_ );
	if(remainingSize() < size) {
		size = remainingSize();
	}
	return currentPtr();
}

void CPacketBuffer::throwAway(size_t size) {
	fastforward(size);
}

size_t CPacketBuffer::totalSize() {
	return buffer_.size();
}

size_t CPacketBuffer::remainingSize() {
	return buffer_.size() - readPos_;
}

const void * CPacketBuffer::basePtr() {
	return buffer_.c_str();
}

const void * CPacketBuffer::currentPtr() {
	return buffer_.c_str() + readPos_;
}

size_t CPacketBuffer::readPos() {
	return readPos_;
}

void CPacketBuffer::setReadPos(size_t pos) {
	boost::recursive_mutex::scoped_lock autolock( lock_ );

	if(pos != 0 && pos >= totalSize())
		throw CPacketException("readpos less than 0 by rewind");

	readPos_ = pos;
}

void CPacketBuffer::clear() {
	boost::recursive_mutex::scoped_lock autolock( lock_ );
	readPos_ = 0;
	buffer_.clear();
}

void CPacketBuffer::rewind(size_t size) {
	boost::recursive_mutex::scoped_lock autolock( lock_ );
	int temp = readPos_ - size;
	if(temp < 0) {
		throw CPacketException("readpos less than 0 by rewind");
	}
	readPos_ = temp;
}

void CPacketBuffer::fastforward(size_t size) {
	boost::recursive_mutex::scoped_lock autolock( lock_ );
	int temp = readPos_ + size;
	if(temp > (int)buffer_.size())
		throw CPacketException("readpos exceed buffer size by fastforward");
	readPos_ = temp;
}

std::string CPacketBuffer::toStringFromCurrentPtr() {
	std::string str;
	str.assign((char *)currentPtr(), remainingSize());
	return str;
}

std::string CPacketBuffer::toStringFromBasePtr() {
	std::string str;
	str.assign((char *)basePtr(), totalSize());
	return str;
}

std::string CPacketBuffer::readString(size_t size) {
	boost::recursive_mutex::scoped_lock autolock( lock_ );
	if(remainingSize() < size) {
		size = remainingSize();
	}
	std::string str;
	str.assign((const char *)currentPtr(), size);
	fastforward(size);
	return str;
}

