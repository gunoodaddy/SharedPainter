#include "StdAfx.h"
#include "SettingManager.h"

#define DEFAULT_FILE_NAME	"SharedPainter.ini"

CSettingManager::CSettingManager(void)
{
	// Save timer
	timer_ = new QTimer(this);
	timer_->start(10000);	// 10 sec
	connect(timer_, SIGNAL(timeout()),this, SLOT(onTimer()));

	load();
}

CSettingManager::~CSettingManager(void)
{
	delete timer_;
}

void CSettingManager::onTimer( void )
{
	save();
}

void CSettingManager::load( void )
{
	QSettings settings( DEFAULT_FILE_NAME, QSettings::IniFormat );
	settings.beginGroup( "network" );
	peerAddress_ = settings.value( "peerAddress" ).toString().toStdString();
	broadCastChannel_ = settings.value( "broadCastChannel" ).toString().toStdString();
	settings.endGroup();
}


void CSettingManager::save( void )
{
	QSettings settings( DEFAULT_FILE_NAME, QSettings::IniFormat );
	settings.beginGroup( "network" );
	settings.setValue( "peerAddress", peerAddress_.c_str() );
	settings.setValue( "broadCastChannel", broadCastChannel_.c_str() );
	settings.endGroup();
}
