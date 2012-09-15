#pragma once

#define USE_LITTLE_ENDIAN_MODE 1

typedef std::vector<std::string> stringlist_t;

class PacketBufferException : public std::exception {
public:
	PacketBufferException(const char *message) {
		setMessage(message);
	}

	virtual ~PacketBufferException() throw() {}

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


class PacketBufferUtil
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

	static size_t writeInt32( std::string &buf, size_t pos, boost::int32_t value, bool LE ) {
		boost::int32_t net;
		if(LE) {
			net = (boost::int32_t)value;
		} else {
			net = (boost::int32_t)htonl(value);
		}
		buf.insert( pos, (const char*)&net, 4 );
		return 4;
	}

	static size_t writeInt16( std::string &buf, size_t pos, boost::int16_t value, bool LE ) {
		boost::int16_t net;
		if(LE) {
			net = (boost::int16_t)value;
		} else {
			net = (boost::int16_t)htons(value);
		}
		buf.insert( pos, (const char*)&net, 2 );
		return 2;
	}

	static size_t writeInt8( std::string &buf, size_t pos, boost::int8_t value ) {
		buf.insert( pos, (const char*)&value, 1 );
		return 1;
	}

	static size_t writeBinary( std::string &buf, size_t pos, const void *data, size_t size ) {
		buf.insert( pos, (const char*)data, size );
		return size;
	}

	static size_t writeString8( std::string &buf, size_t pos, const std::string &value) {
		if( value.size() > 0xff )
			throw PacketBufferException("the string size is bigger than 1byte length..");

		pos += writeInt8( buf, pos, value.size() );
		buf.insert( pos, value );
		return 1 + value.size();
	}

	static size_t writeString16( std::string &buf, size_t pos, const std::string &value, bool LE ) {
		if( value.size() > 0xffff )
			throw PacketBufferException("the string size is bigger than 2byte length..");

		pos += writeInt16( buf, pos, value.size(), LE );
		buf.insert( pos, value );
		return 2 + value.size();
	}

	static size_t writeString32( std::string &buf, size_t pos, const std::string &value, bool LE ) {
		if( value.size() > 0xffffffff )
			throw PacketBufferException("the string size is bigger than 4byte length..");

		pos += writeInt32( buf, pos, value.size(), LE );
		buf.insert( pos, value );
		return 4 + value.size();
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
			throw PacketBufferException("readInt32 failed..");

		value = bitwise_cast<double>(theBytes.all);
		return 8;                             
	}

	static boost::int32_t readInt32( const std::string &buf, size_t pos, boost::int32_t &value, bool LE ) {
		union bytes {
			boost::int8_t b[4];
			boost::int32_t all;
		} theBytes;

		if( read( buf, pos, (void *)theBytes.b, 4 ) == 4 )
		{
			if( LE )
				value = (boost::int32_t)theBytes.all;
			else
				value = (boost::int32_t)ntohl(theBytes.all);
		}
		else
			throw PacketBufferException("readInt32 failed..");

		return 4;
	}

	static boost::int16_t readInt16( const std::string &buf, size_t pos, boost::int16_t &value, bool LE ) {
		union bytes {
			boost::int8_t b[2];
			boost::int16_t all;
		} theBytes;

		if( read( buf, pos, (void *)theBytes.b, 2 ) == 2 )
		{
			if( LE )
				value = (boost::int16_t)theBytes.all;
			else
				value = (boost::int16_t)ntohs(theBytes.all);
		}
		else
			throw PacketBufferException("readInt16 failed..");

		return 2;
	}

	static boost::int8_t readInt8( const std::string &buf, size_t pos, boost::int8_t &value ) {
		boost::int8_t b[1];
		if( read( buf, pos, (void *)b, 1) == 1 )
			value = b[0];
		else
			throw PacketBufferException("readInt8 failed..");
			
		return 1;
	}

	static boost::int32_t readString32( const std::string &buf, size_t pos, std::string &value, bool LE ) {
		boost::int32_t len = 0;
		pos += readInt32( buf, pos, len, LE );
		int nread = read( buf, pos, value, len );
		if(nread != len)
			throw PacketBufferException("readString32 failed..");
		return 4 + len;
	}

	static boost::int32_t readString16( const std::string &buf, size_t pos, std::string &value, bool LE ) {
		boost::int16_t len = 0;
		pos += readInt16( buf, pos, len, LE );
		int nread = read( buf, pos, value, len );
		if(nread != len)
			throw PacketBufferException("readString16 failed..");
		return 2 + len;
	}

	static boost::int32_t readString8( const std::string &buf, size_t pos, std::string &value ) {
		boost::int8_t len = 0;
		pos += readInt8( buf, pos, len );
		int nread = read( buf, pos, value, len );
		if(nread != len)
			throw PacketBufferException("readString8 failed..");
		return 1 + len;
	}
};


