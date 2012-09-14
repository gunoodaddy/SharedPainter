#pragma once

struct SPaintUserInfoData
{
	std::string channel;
	std::string userId;
};

class CPaintUser;

typedef std::vector<boost::shared_ptr<CPaintUser> > USER_LIST;

class CPaintUser
{
public:
	CPaintUser( void ) { }
	~CPaintUser( void ) { }

	void setSessionId( int sessionId ) { sessionId_ = sessionId; }
	int sessionId( void ) { return sessionId_; }
	void setData( const struct SPaintUserInfoData &info ) { data_ = info; }
	void setChannel( const std::string & channel ) { data_.channel = channel; }
	const std::string &channel( void ) { return data_.channel; }
	const std::string &userId( void ) { return data_.userId; }

private:
	int sessionId_;
	SPaintUserInfoData data_;
};