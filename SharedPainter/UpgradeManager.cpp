#include "StdAfx.h"
#include "UpgradeManager.h"

CUpgradeManager::CUpgradeManager(void) : upgradeState_(FirstRunState), currentTimerSecond_(0), patchFileDownloaded_(false)
{
	start();
}

CUpgradeManager::~CUpgradeManager(void)
{
}

void CUpgradeManager::run()
{
	QTimer timer;
	timer.setInterval( 1000 );
	timer.moveToThread( this );
	connect( &timer, SIGNAL(timeout()), this, SLOT(onTimer()), Qt::DirectConnection );
	timer.start();

	nam_ = new QNetworkAccessManager();
	nam_->moveToThread( this );
	connect( nam_, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)), Qt::DirectConnection );

	exec();
	
	delete nam_;
}

void CUpgradeManager::close( void )
{
	quit();
}

bool CUpgradeManager::isAvailableUpgrade( void )
{
	if( ! patchFileDownloaded_ )
		return false;

	if( std::string(VERSION_TEXT) >= remoteVersion_ )
		return false;

	return true;
}

void CUpgradeManager::doUpgradeNow( void )
{
	if( ! patchFileDownloaded_ )
		return;

	gotoState( WaitForUnlimited );

	// Run Upgrader
	Util::executeProgram( PROGRAM_UPGRADER_FILE_NAME );

	// Program exit..
	_exit_flag = true;
	qApp->exit(0);
}

void CUpgradeManager::stopVersionCheck( void )
{
	mutex_.lock();
	upgradeState_ = true;
	currentTimerSecond_ = 0;
	upgradeState_ = WaitForUnlimited;
	mutex_.unlock();
}

void CUpgradeManager::gotoState( int state )
{
	mutex_.lock();
	upgradeState_ = state;
	mutex_.unlock();
}

void CUpgradeManager::gotoNextState( void )
{
	mutex_.lock();
	upgradeState_++;
	mutex_.unlock();
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
		gotoState( DownloadVersionInfo );	// at the first, run right now once!
		break;
	case InitState:
		if( currentTimerSecond_ >= DEFAULT_UPGRADE_CHECK_SECOND )
		{
			gotoNextState();
			// go through next state
		}
		else
			break;
	case DownloadVersionInfo:
		currentReply_ = nam_->get(QNetworkRequest(QUrl("https://raw.github.com/gunoodaddy/SharedPainter/master/release/version_win.txt")));
		connect( currentReply_, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)), Qt::DirectConnection );
		gotoNextState();
		break;
	case DownloadingVersionInfo:
		// waiting for finishing download
		break;
	case DownloadCompleteVersionInfo:
		processVersionInfoFile();
		break;
	case DownloadPatchFile:
		currentReply_ = nam_->get(QNetworkRequest(QUrl("https://raw.github.com/gunoodaddy/SharedPainter/master/release/patch_win.zip")));
		connect( currentReply_, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)), Qt::DirectConnection );
		gotoNextState();
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
	case WaitForUnlimited:
		// waiting for unlimited... 
		break;
	case ErrorState:
		// TODO : UpgradeManager need more handling???
		mutex_.lock();
		currentTimerSecond_ = 0;
		mutex_.unlock();
		gotoState( InitState );
		break;
	}
}

void CUpgradeManager::onTimer( void )
{
	mutex_.lock();
	currentTimerSecond_++;
	mutex_.unlock();
	doJob();
}

void CUpgradeManager::processVersionInfoFile( void )
{
	mutex_.lock();

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

	mutex_.unlock();

	if( std::string(VERSION_TEXT) < remoteVersion_ )
		gotoNextState();
	else
	{
		gotoState( ErrorState );
	}
}


void CUpgradeManager::processPatchFile( void )
{
	QByteArray res = currentReply_->readAll();

	QFile f( DEFAULT_UPGRADE_FILE_NAME );
	if( !f.open( QIODevice::WriteOnly ) )
	{
		gotoState( ErrorState );
		return;
	}

	QDataStream out(&f);
	int ret = out.writeRawData( res.data(), res.size() );
	if( ret != res.size() )
	{
		gotoState( ErrorState );
		return;
	}
	f.close();

	patchFileDownloaded_ = true;

	CDefferedCaller::singleShot( boost::bind( &CUpgradeManager::fireObserver_NewVersion, this, remoteVersion_, patchContents_) );

	gotoNextState();
}


void CUpgradeManager::finished(QNetworkReply *reply)
{
	// error handling
	if(reply->error() != QNetworkReply::NoError)  
	{
		gotoState( ErrorState );
		
		qDebug() << reply->errorString();
		return;
	}

	gotoNextState();
}


void CUpgradeManager::downloadProgress(qint64 done, qint64 total)
{
	qDebug() << "dataReadProgress" << done << total;
}
