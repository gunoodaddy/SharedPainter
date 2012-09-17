#pragma once

class SharedPaintRoom;
class SharedPaintProtocol;
class SharedPaintClient;

typedef std::map<std::string, boost::shared_ptr<SharedPaintClient> > CLIENT_MAP;
typedef std::map<std::string, boost::shared_ptr<SharedPaintRoom> > ROOM_MAP;

class SharedPaintRoom
{
public:
	SharedPaintRoom( const std::string & roomid ) : roomId_(roomid), lastSyncRunnerIndex_(0) { }

	void addJoiner( boost::shared_ptr<SharedPaintClient> joiner );

	void removeJoiner( boost::shared_ptr<SharedPaintClient> joiner );

	void roomCast( const std::string &fromid, boost::shared_ptr<SharedPaintProtocol> prot, bool sendMySelf = false );

	void uniCast( boost::shared_ptr<SharedPaintProtocol> prot );

	void setSuperPeerSession( boost::shared_ptr<SharedPaintClient> client );

	boost::shared_ptr<SharedPaintClient> currentSuperPeerSession( void );

	std::string generateJoinerInfoPacket( void );

	void syncStart( const std::string &tartgetId );

	size_t userCount( void ) { return clientMap_.size(); }

private:
	void tossSuperPeerRightToCandidates( void );

private:
	std::string roomId_;
	boost::shared_ptr<SharedPaintClient> superPeerSession_;
	CLIENT_MAP clientMap_;
	int lastSyncRunnerIndex_;
};

