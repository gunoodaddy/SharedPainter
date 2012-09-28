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

#define USE_LITTLE_ENDIAN_MODE 1

typedef std::vector<std::string> stringlist_t;

class CPacketException : public std::exception {
public:
	CPacketException(const char *message) {
		setMessage(message);
	}

	virtual ~CPacketException() throw() {}

	virtual const char* what() const throw() {
		if (strlen(message_) <= 0) {
			return "Default Exception.";
		} else {
			return message_;
		}
	}

private:
	void setMessage(const char* msg) {
		size_t len = strlen(msg);
		if(len > sizeof(message_) - 1)
			len = sizeof(message_) - 1;
		memset(message_, 0, sizeof(message_));
		memcpy(message_, msg, len);
	}

protected:
	enum { MSG_SIZE = 128 };
	char message_[MSG_SIZE + 1];
};


class CPacketBufferUtil
{
	template <typename To, typename From>
	static inline To bitwise_cast(From from) {
		BOOST_STATIC_ASSERT(sizeof(From) == sizeof(To));

		// BAD!!!  These are all broken with -O2.
		//return *reinterpret_cast<To*>(&from);  // BAD!!!
		//return *static_cast<To*>(static_cast<void*>(&from));  // BAD!!!
		//return *(To*)(void*)&from;  // BAD!!!

		// Super clean and paritally blessed by section 3.9 of the standard.
		//unsigned char c[sizeof(from)];
		//memcpy(c, &from, sizeof(from));
		//To to;
		//memcpy(&to, c, sizeof(c));
		//return to;

		// Slightly more questionable.
		// Same code emitted by GCC.
		//To to;
		//memcpy(&to, &from, sizeof(from));
		//return to;

		// Technically undefined, but almost universally supported,
		// and the most efficient implementation.
		union {
			From f;
			To t;
		} u;
		u.f = from;
		return u.t;
	} 

	// Writing helper method
public:
	static size_t writeDouble( std::string &buf, size_t pos, double value, bool LE ) {
		boost::uint64_t bits = bitwise_cast<boost::uint64_t>(value);

		if(LE) {
			bits = (boost::uint64_t)bits;
		} else {
#ifdef USE_HTONLL
			bits = (boost::uint64_t)htonll(bits);
#else
			assert( false && "not support htonll()");
#endif
		}
		buf.insert( pos, (const char*)&bits, 8 );
		return 8;
	}

	static size_t writeInt32( std::string &buf, size_t pos, boost::uint32_t value, bool LE ) {
		boost::uint32_t net;
		if(LE) {
			net = (boost::uint32_t)value;
		} else {
			net = (boost::uint32_t)htonl(value);
		}
		buf.insert( pos, (const char*)&net, 4 );
		return 4;
	}

	static size_t writeInt16( std::string &buf, size_t pos, boost::uint16_t value, bool LE ) {
		boost::uint16_t net;
		if(LE) {
			net = (boost::uint16_t)value;
		} else {
			net = (boost::uint16_t)htons(value);
		}
		buf.insert( pos, (const char*)&net, 2 );
		return 2;
	}

	static size_t writeInt8( std::string &buf, size_t pos, boost::uint8_t value ) {
		buf.insert( pos, (const char*)&value, 1 );
		return 1;
	}

	static size_t writeBinary( std::string &buf, size_t pos, const void *data, size_t size ) {
		buf.insert( pos, (const char*)data, size );
		return size;
	}

	static size_t writeString8( std::string &buf, size_t pos, const std::string &value) {
		if( value.size() > 0xff )
			throw CPacketException("the string size is bigger than 1byte length..");

		pos += writeInt8( buf, pos, value.size() );
		buf.insert( pos, value );
		return 1 + value.size();
	}

	static size_t writeString16( std::string &buf, size_t pos, const std::string &value, bool LE ) {
		if( value.size() > 0xffff )
			throw CPacketException("the string size is bigger than 2byte length..");

		pos += writeInt16( buf, pos, value.size(), LE );
		buf.insert( pos, value );
		return 2 + value.size();
	}

	static size_t writeString32( std::string &buf, size_t pos, const std::string &value, bool LE ) {
		if( value.size() > 0xffffffff )
			throw CPacketException("the string size is bigger than 4byte length..");

		pos += writeInt32( buf, pos, value.size(), LE );
		buf.insert( pos, value );
		return 4 + value.size();
	}

	static size_t writeString32List( std::string &buf, size_t pos, const stringlist_t &list, bool LE ) {
		pos += writeInt32( buf, pos, (boost::uint32_t)list.size(), LE );
		for(size_t i = 0; i < list.size(); i++) {
			pos += writeString32( buf, pos, list[i], LE );
		}
		return pos;
	}

	// Reading helper method
public:
	static int read( const std::string &buf, size_t pos, std::string &data, size_t size) {
		if( buf.size() - pos < size) {
			size = buf.size() - pos;
		}
		data.assign(buf.c_str() + pos, size);
		return size;
	}

	static int read( const std::string &buf, size_t pos, void *ptr, size_t size ) {
		if( buf.size() - pos < size) {
			size = buf.size() - pos;
		}
		memcpy(ptr, buf.c_str() + pos, size);
		return size;
	}

	static size_t readDouble( const std::string &buf, size_t pos, double &value, bool LE ) {

		union bytes {                         
			boost::uint8_t b[8];                       
			boost::uint64_t all;                       
		} theBytes; 

		if( read( buf, pos, (void *)theBytes.b, 8 ) == 8 )
		{
			if( LE )
				theBytes.all = (boost::uint64_t)(theBytes.all);  
			else
#ifdef USE_NTOHLL
				theBytes.all = ntohll(theBytes.all);  
#else
				assert( false && "not support ntohll()");
#endif		
		}
		else
			throw CPacketException("readInt32 failed..");

		value = bitwise_cast<double>(theBytes.all);
		return 8;                             
	}

	static boost::uint32_t readInt32( const std::string &buf, size_t pos, boost::uint32_t &value, bool LE ) {
		union bytes {
			boost::uint8_t b[4];
			boost::uint32_t all;
		} theBytes;

		if( read( buf, pos, (void *)theBytes.b, 4 ) == 4 )
		{
			if( LE )
				value = (boost::uint32_t)theBytes.all;
			else
				value = (boost::uint32_t)ntohl(theBytes.all);
		}
		else
			throw CPacketException("readInt32 failed..");

		return 4;
	}

	static boost::uint16_t readInt16( const std::string &buf, size_t pos, boost::uint16_t &value, bool LE ) {
		union bytes {
			boost::uint8_t b[2];
			boost::uint16_t all;
		} theBytes;

		if( read( buf, pos, (void *)theBytes.b, 2 ) == 2 )
		{
			if( LE )
				value = (boost::uint16_t)theBytes.all;
			else
				value = (boost::uint16_t)ntohs(theBytes.all);
		}
		else
			throw CPacketException("readInt16 failed..");

		return 2;
	}

	static boost::uint8_t readInt8( const std::string &buf, size_t pos, boost::uint8_t &value ) {
		boost::uint8_t b[1];
		if( read( buf, pos, (void *)b, 1) == 1 )
			value = b[0];
		else
			throw CPacketException("readInt8 failed..");
			
		return 1;
	}

	static boost::uint32_t readString32( const std::string &buf, size_t pos, std::string &value, bool LE ) {
		boost::uint32_t len = 0;
		pos += readInt32( buf, pos, len, LE );
		int nread = read( buf, pos, value, len );
		if(nread != (int)len)
			throw CPacketException("readString32 failed..");
		return 4 + len;
	}

	static boost::uint32_t readString16( const std::string &buf, size_t pos, std::string &value, bool LE ) {
		boost::uint16_t len = 0;
		pos += readInt16( buf, pos, len, LE );
		int nread = read( buf, pos, value, len );
		if(nread != len)
			throw CPacketException("readString16 failed..");
		return 2 + len;
	}

	static boost::uint32_t readString8( const std::string &buf, size_t pos, std::string &value ) {
		boost::uint8_t len = 0;
		pos += readInt8( buf, pos, len );
		int nread = read( buf, pos, value, len );
		if(nread != len)
			throw CPacketException("readString8 failed..");
		return 1 + len;
	}
};


class CPacketBuffer 
{
public:
	CPacketBuffer();
	~CPacketBuffer();

public:
	int erase(size_t pos, size_t size);
	int write(const char* ptr, size_t size);
	int write(const void* ptr, size_t size);
	int read(std::string &data, size_t size);
	int read(void *ptr, size_t size);
	const void * peek(size_t &size);
	int peek(char *buffer, size_t size);
	void throwAway(size_t size);
	int insertInt8(size_t pos, boost::uint8_t value);

	boost::uint32_t writeInt32(boost::uint32_t value) {
		boost::uint32_t net;
		if(littleEndianMode) {
			net = (boost::uint32_t)value;
		} else {
			net = (boost::uint32_t)htonl(value);
		}
		write((boost::uint8_t*)&net, 4);
		return 4;
	}

	boost::uint16_t writeInt16(boost::uint16_t value) {
		boost::uint16_t net;
		if(littleEndianMode) {
			net = (boost::uint16_t)value;
		} else {
			net = (boost::uint16_t)htons(value);
		}
		write((boost::uint8_t*)&net, 2);
		return 2;
	}

	boost::uint8_t writeInt8(boost::uint8_t value) {
		write((boost::uint8_t*)&value, 1);
		return 1;
	}

	boost::uint32_t writeBinary(const void *data, size_t size) {
		boost::uint32_t pos = 0;
		pos += write(data, size);
		return pos;
	}

	boost::uint32_t writeString8(const std::string &value) {
		boost::uint8_t pos = 0;
		pos += writeInt8(value.size());
		pos += write(value.c_str(), value.size());
		return pos;
	}

	boost::uint32_t writeString16(const std::string &value) {
		boost::uint16_t pos = 0;
		pos += writeInt16(value.size());
		pos += write(value.c_str(), value.size());
		return pos;
	}

	boost::uint32_t writeString32(const std::string &value) {
		boost::uint32_t pos = 0;
		pos += writeInt32(value.size());
		pos += write(value.c_str(), value.size());
		return pos;
	}

	boost::uint32_t writeString32List(const stringlist_t &list) {
		boost::uint32_t pos = 0;
		pos += writeInt32((boost::uint32_t)list.size());
		for(size_t i = 0; i < list.size(); i++) {
			pos += writeString32(list[i]);
		}
		return pos;
	}

	// Reading helper method
public:
	boost::uint32_t readInt32(boost::uint32_t &value) {
		union bytes {
			boost::uint8_t b[4];
			boost::uint32_t all;
		} theBytes;

		if(read((void *)theBytes.b, 4) != 4)
			throw CPacketException("readInt32 failed..");
		else {
			if(littleEndianMode)
				value = (boost::uint32_t)theBytes.all;
			else
				value = (boost::uint32_t)ntohl(theBytes.all);
		}
		return 4;
	}

	boost::uint32_t readInt32() {
		union bytes {
			boost::uint8_t b[4];
			boost::uint32_t all;
		} theBytes;

		if(read((void *)theBytes.b, 4) != 4)
			throw CPacketException("readInt32 failed..");
		if(littleEndianMode) {
			return (boost::uint32_t)theBytes.all;
		} else {
			return (boost::uint32_t)ntohl(theBytes.all);
		}
	}

	boost::uint16_t readInt16(boost::uint16_t &value) {
		union bytes {
			boost::uint8_t b[2];
			boost::uint16_t all;
		} theBytes;

		if(read((void *)theBytes.b, 2) != 2)
			throw CPacketException("readInt16 failed..");
		else {
			if(littleEndianMode)
				value = (boost::uint16_t)theBytes.all;
			else
				value = (boost::uint16_t)ntohs(theBytes.all);
		}
		return 2;
	}

	boost::uint16_t readInt16() {
		union bytes {
			boost::uint8_t b[2];
			boost::uint16_t all;
		} theBytes;

		if(read((void *)theBytes.b, 2) != 2)
			throw CPacketException("readInt16 failed..");
		if(littleEndianMode) {
			return (boost::uint16_t)theBytes.all;
		} else {
			return (boost::uint16_t)ntohs(theBytes.all);
		}
	}

	boost::uint8_t readInt8(boost::uint8_t &value) {
		boost::uint8_t b[1];
		int res = read((void *)b, 1);
		if(res != 1)
			throw CPacketException("readInt8 failed..");
		else
			value = b[0];
		return 1;
	}

	boost::uint8_t readInt8() {
		boost::uint8_t b[1];
		int res = read((void *)b, 1);
		if(res != 1)
			throw CPacketException("readInt8 failed..");
		return b[0];
	}

	boost::uint32_t readString32(std::string &value) {
		boost::uint32_t len = 0;
		boost::uint32_t pos = 0;
		pos += readInt32(len);
		int nread = read(value, len);
		if(nread != (int)len)
			throw CPacketException("readString32 failed..");
		return pos + len;
	}

	std::string readString32() {
		std::string str;
		size_t len = readInt8();
		int nread = read(str, len);
		if(nread != (int)len)
			throw CPacketException("readString32 failed..");
		return str;
	}

	boost::uint32_t readString16(std::string &value) {
		boost::uint16_t len = 0;
		boost::uint16_t pos = 0;
		pos += readInt16(len);
		int nread = read(value, len);
		if(nread != len)
			throw CPacketException("readString16 failed..");
		return pos + len;
	}

	std::string readString16() {
		std::string str;
		size_t len = readInt16();
		int nread = read(str, len);
		if(nread != (int)len)
			throw CPacketException("readString16 failed..");
		return str;
	}

	boost::uint32_t readString8(std::string &value) {
		boost::uint8_t len = 0;
		boost::uint32_t pos = 0;
		pos += readInt8(len);
		int nread = read(value, len);
		if(nread != len)
			throw CPacketException("readString8 failed..");
		return pos + len;
	}

	std::string readString8() {
		std::string str;
		size_t len = readInt8();
		int nread = read(str, len);
		if(nread != (int)len)
			throw CPacketException("readString8 failed..");
		return str;
	}

	std::string readString(size_t size);

public:
	size_t totalSize();
	size_t remainingSize();
	size_t readPos();
	void setReadPos(size_t pos);

	const void * basePtr();
	const void * currentPtr();
	std::string toStringFromBasePtr();
	std::string toStringFromCurrentPtr();

	void clear();
	void rewind(size_t size);
	void fastforward(size_t size);

private:
	std::string buffer_;
	int readPos_;
	/*int writePos_;*/	// write position is not necessary by using std::string
	boost::recursive_mutex lock_;

	static bool littleEndianMode;
};
