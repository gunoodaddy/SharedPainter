#pragma once

#include "Singleton.h"

#define SettingManagerPtr()		CSingleton<CSettingManager>::Instance()

class CSettingManager : public QObject
{
	Q_OBJECT

public:
	CSettingManager(void);
	~CSettingManager(void);

	const std::string peerAddress( void ) { return peerAddress_; }
	void setPeerAddress( const std::string & addr )
	{
		peerAddress_ = addr;
	}

	const std::string broadCastChannel( void) { return broadCastChannel_; }
	void setBroadCastChannel( const std::string & channel )
	{
		broadCastChannel_ = channel;
	}

	void load( void );
	void save( void );

protected slots:
	void onTimer( void );

private:
	std::string broadCastChannel_;
	std::string peerAddress_;
	QTimer *timer_;
};
