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

	const std::string relayServerAddress( void ) { return relayServerAddress_; }
	void setRelayServerAddress( const std::string & addr )
	{
		relayServerAddress_ = addr;
	}

	const std::string paintChannel( void) { return paintChannel_; }
	void setPaintChannel( const std::string & channel )
	{
		paintChannel_ = channel;
	}

	void load( void );
	void save( void );

protected slots:
	void onTimer( void );

private:
	std::string paintChannel_;
	std::string peerAddress_;
	std::string relayServerAddress_;
	QTimer *timer_;
};
