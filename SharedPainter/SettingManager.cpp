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

#include "StdAfx.h"
#include "SettingManager.h"

#define DEFAULT_FILE_NAME	"SharedPainter.ini"

CSettingManager::CSettingManager(void)
{
	iniFile = qApp->applicationDirPath() + QDir::separator() + DEFAULT_FILE_NAME;

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
	QSettings settings( iniFile, QSettings::IniFormat );

	settings.beginGroup( "private" );
	if( false == _multi_instance_mode )
		myId_ = Util::toUtf8StdString( settings.value( "myId" ).toString() );
	settings.endGroup();

	settings.beginGroup( "personal" );
	nickName_ = Util::toUtf8StdString( settings.value( "nickName" ).toString() );
	settings.endGroup();

	settings.beginGroup( "general" );
	syncWindowSize_ = settings.value( "syncWindowSize", true ).toBool();
	serverConnectOnStart_ = settings.value( "serverConnectOnStart", true ).toBool();
	blinkLastItem_ = settings.value( "blinkLastItem", true ).toBool();
	autoSaveData_ = settings.value( "autoSaveData", true ).toBool();
	hiqhQualityMoveItemMode_ = settings.value( "highQualityMoveItem", false ).toBool();
	settings.endGroup();

	settings.beginGroup( "network" );
	peerAddress_		= settings.value( "peerAddress" ).toString().toStdString();
	relayServerAddress_ = settings.value( "relayServerAddress" ).toString().toStdString();
	paintChannel_		= settings.value( "paintChannel" ).toString().toStdString();
	settings.endGroup();
}


void CSettingManager::save( void )
{
	QSettings settings( iniFile, QSettings::IniFormat );

	settings.beginGroup( "private" );
	if( false == _multi_instance_mode )
		settings.setValue( "myId", Util::toStringFromUtf8(myId_) );
	settings.endGroup();

	settings.beginGroup( "personal" );
	settings.setValue( "nickName", Util::toStringFromUtf8(nickName_) );
	settings.endGroup();

	settings.beginGroup( "general" );
	settings.setValue( "syncWindowSize", syncWindowSize_ );
	settings.setValue( "serverConnectOnStart", serverConnectOnStart_ );
	settings.setValue( "blinkLastItem", blinkLastItem_ );
	settings.setValue( "autoSaveData", autoSaveData_ );
	settings.setValue( "highQualityMoveItem", hiqhQualityMoveItemMode_ );
	settings.endGroup();

	settings.beginGroup( "network" );
	settings.setValue( "peerAddress", peerAddress_.c_str() );
	settings.setValue( "relayServerAddress", relayServerAddress_.c_str() );
	settings.setValue( "paintChannel", paintChannel_.c_str() );
	settings.endGroup();
}
