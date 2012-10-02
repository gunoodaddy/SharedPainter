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

#pragma once

#include "Singleton.h"

#define SettingManagerPtr()		CSingleton<CSettingManager>::Instance()

class CSettingManager : public QObject
{
	Q_OBJECT

public:
	CSettingManager(void);
	~CSettingManager(void);

	const std::string &myId( void ) { return myId_; }
	void setMyId( const std::string & id ) { myId_ = id; }

	const std::string &peerAddress( void ) { return peerAddress_; }
	void setPeerAddress( const std::string & addr ) { peerAddress_ = addr; }

	const std::string &relayServerAddress( void ) { return relayServerAddress_; }
	void setRelayServerAddress( const std::string & addr ) { relayServerAddress_ = addr; }

	const std::string &paintChannel( void) { return paintChannel_; }
	void setPaintChannel( const std::string & channel ) { paintChannel_ = channel; }

	const std::string &nickName( void) { return nickName_; }
	void setNickName( const std::string & nickName ) { nickName_ = nickName; }

	bool isBlinkLastItem( void ) { return blinkLastItem_; }
	void setBlinkLastItem( bool enabled ) { blinkLastItem_ = enabled; }

	bool isSyncWindowSize( void ) { return syncWindowSize_; }
	void setSyncWindowSize( bool enabled ) { syncWindowSize_ = enabled; }

	bool isRelayServerConnectOnStarting( void ) { return serverConnectOnStart_; }
	void setRelayServerConnectOnStarting( bool enabled ) { serverConnectOnStart_ = enabled; }

	bool isAutoSaveData( void ) { return autoSaveData_; }
	void setAutoSaveData( bool enabled ) { autoSaveData_ = enabled; }

	bool isHighQualityMoveItemMode( void ) { return hiqhQualityMoveItemMode_; }
	void setHighQualityMoveItemMode( bool enabled ) { hiqhQualityMoveItemMode_ = enabled; }

	void load( void );
	void save( void );

protected slots:
	void onTimer( void );

private:
	QString iniFile;
	std::string myId_;
	std::string paintChannel_;
	std::string peerAddress_;
	std::string relayServerAddress_;
	std::string nickName_;
	bool syncWindowSize_;
	bool serverConnectOnStart_;
	bool blinkLastItem_;
	bool autoSaveData_;
	bool hiqhQualityMoveItemMode_;

	QTimer *timer_;
};
