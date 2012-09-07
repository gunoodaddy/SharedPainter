#pragma once

struct SUserInfoData
{
	std::string userId;
};

class CPaintUser
{
public:
	CPaintUser( void ) { }
	~CPaintUser( void ) { }

	void loadData( const struct SUserInfoData &info ) { data_ = info; }

	const std::string &userId( void ) { return data_.userId; }

private:
	SUserInfoData data_;
};