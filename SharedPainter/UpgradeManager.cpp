#include "StdAfx.h"
#include "UpgradeManager.h"

CUpgradeManager::CUpgradeManager(void) : enable_(true)
{
	timer_ = new QTimer(this);
	timer_->start(5000);
	connect( timer_, SIGNAL(timeout()), this, SLOT(onTimer()) );

	nam_ = new QNetworkAccessManager(this);
	connect( nam_, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)) );
}

CUpgradeManager::~CUpgradeManager(void)
{
}

void CUpgradeManager::stopVersionCheck( void )
{
	enable_ = false;
	timer_->stop();
}

void CUpgradeManager::checkVersion( void )
{
	if( std::string(VERSION_TEXT) < remoteVersion_ )
	{
		timer_->stop();
		fireObserver_NewVersion( remoteVersion_, patchContents_ );

		if( enable_ )
			timer_->start();
	}
}

void CUpgradeManager::onTimer( void )
{
	nam_->get(QNetworkRequest(QUrl("https://raw.github.com/gunoodaddy/SharedPainter/master/release/version.txt")));
}

void CUpgradeManager::finished(QNetworkReply *reply)
{
	if(reply->error() == QNetworkReply::NoError)   
	{      
		int lineCnt = 0;
		while( 1 )
		{
			char buf[1024] = {0, };
			qint64 lineLength = reply->readLine(buf, sizeof(buf));
			if( lineLength < 0 )
				break;

			if( lineCnt == 0 ) 
			{
				remoteVersion_.assign( buf, lineLength );
				QString tempStr(remoteVersion_.c_str());
				remoteVersion_ = tempStr.trimmed().toStdString();
				patchContents_ = "";
			}
			else 
			{
				std::string temp( buf, lineLength );
				patchContents_ += temp;
			}
			lineCnt++;
		}

		QString tempStr(patchContents_.c_str());
		patchContents_ = tempStr.trimmed().toStdString();

		checkVersion();
	}  
	else  
	{    
		qDebug() << reply->errorString();
	}

	qDebug() << remoteVersion_.c_str();
	qDebug() << patchContents_.c_str();
}


void CUpgradeManager::downloadProgress(qint64 done, qint64 total)
{
	qDebug() << "dataReadProgress" << done << total;
}
