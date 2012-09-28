#include "StdAfx.h"
#include "UpgradeManager.h"

CUpgradeManager::CUpgradeManager(void) : upgradeState_(FirstRunState), currentTimerSecond_(0)
{
	timer_ = new QTimer(this);
	timer_->start(1000);
	connect( timer_, SIGNAL(timeout()), this, SLOT(onTimer()) );

	nam_ = new QNetworkAccessManager(this);
	connect( nam_, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)) );
}

CUpgradeManager::~CUpgradeManager(void)
{
}

void CUpgradeManager::doUpgradeNow( void )
{
	timer_->stop();

	// Run Upgrader
	Util::executeProgram( PROGRAM_UPGRADER_FILE_NAME );

	// Program exit..
	qApp->exit(0);
}

void CUpgradeManager::stopVersionCheck( void )
{
	timer_->stop();
	currentTimerSecond_ = 0;
	upgradeState_ = InitState;
}

void CUpgradeManager::doJob( void )
{
	static int prevState;

	if( prevState != upgradeState_ )
	{
		qDebug() << "CUpgradeManager::doJob >> STATE CHANGED : " << upgradeState_;
		prevState = upgradeState_;
	}

	switch( upgradeState_ )
	{
	case FirstRunState:
		upgradeState_ = DownloadVersionInfo;	// at the first, run right now once!
		break;
	case InitState:
		if( currentTimerSecond_ >= DEFAULT_UPGRADE_CHECK_SECOND )
		{
			upgradeState_ = DownloadVersionInfo;
			// go through next state
		}
		else
			break;
	case DownloadVersionInfo:
		currentReply_ = nam_->get(QNetworkRequest(QUrl("https://raw.github.com/gunoodaddy/SharedPainter/master/release/version.txt")));
		connect( currentReply_, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)) );
		upgradeState_ = DownloadingVersionInfo;
		break;
	case DownloadingVersionInfo:
		// waiting for finishing download
		break;
	case DownloadCompleteVersionInfo:
		processVersionInfoFile();
		break;
	case DownloadPatchFile:
		currentReply_ = nam_->get(QNetworkRequest(QUrl("https://raw.github.com/gunoodaddy/SharedPainter/master/release/SharedPainter.zip")));
		connect( currentReply_, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)) );
		upgradeState_ = DownloadingPatchFile;
		break;
	case DownloadingPatchFile:
		// waiting for finishing download
		break;
	case DownloadCompletePatchFile:
		processPatchFile();
		break;
	case WaitConfirmFromUser:
		// waiting for user confirm ( User must call doUpgradeNow or stopVersionCheck )
		break;
	case ErrorState:
		// TODO : UpgradeManager need more detail error handling?
		upgradeState_ = InitState;
		break;
	}
}

void CUpgradeManager::onTimer( void )
{
	currentTimerSecond_++;
	doJob();
}

void CUpgradeManager::processVersionInfoFile( void )
{
	int lineCnt = 0;
	while( 1 )
	{
		char buf[1024] = {0, };
		qint64 lineLength = currentReply_->readLine(buf, sizeof(buf));
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

	if( std::string(VERSION_TEXT) < remoteVersion_ )
		upgradeState_++;	// go to next stage!
	else
	{
		currentTimerSecond_ = 0;
		upgradeState_ = InitState;
	}
}


void CUpgradeManager::processPatchFile( void )
{
	QByteArray res = currentReply_->readAll();

	QFile f( DEFAULT_UPGRADE_FILE_NAME );
	if( !f.open( QIODevice::WriteOnly ) )
	{
		upgradeState_ = ErrorState;
		return;
	}

	QDataStream out(&f);
	int ret = out.writeRawData( res.data(), res.size() );
	if( ret != res.size() )
	{
		upgradeState_ = ErrorState;
		return;
	}
	f.close();

	fireObserver_NewVersion( remoteVersion_, patchContents_ );

	upgradeState_++;	// go to next stage!
}


void CUpgradeManager::finished(QNetworkReply *reply)
{
	// error handling
	if(reply->error() != QNetworkReply::NoError)  
	{
		currentTimerSecond_ = 0;
		upgradeState_ = ErrorState;
		
		qDebug() << reply->errorString();
		return;
	}

	upgradeState_++;	// go to next stage!
}


void CUpgradeManager::downloadProgress(qint64 done, qint64 total)
{
	qDebug() << "dataReadProgress" << done << total;
}
