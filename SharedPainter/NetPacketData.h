#pragma once

class CNetPacketData
{
public:
	CNetPacketData( boost::int32_t packetId, const std::string &body ) : packetId_(packetId)
	{
		writeBuffer_.write( body.c_str(), body.size() );
	}

	int packetId( void ) { return packetId_; }
		
	CPacketBuffer &buffer( void ) { return writeBuffer_; }

private:
	boost::int32_t packetId_;
	CPacketBuffer writeBuffer_;
};
