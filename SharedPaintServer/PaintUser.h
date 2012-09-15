#pragma once

struct SPaintUserInfoData
{
	std::string roomId;
	std::string userId;
};

class CPaintUser
{
public:
	CPaintUser( void ) { }
	~CPaintUser( void ) { }

	void setSessionId( int sessionId ) { sessionId_ = sessionId; }
	int sessionId( void ) { return sessionId_; }
	void setData( const struct SPaintUserInfoData &info ) { data_ = info; }

	const std::string &roomId( void ) { return data_.roomId; }
	const std::string &userId( void ) { return data_.userId; }

private:
	int sessionId_;
	SPaintUserInfoData data_;
};
