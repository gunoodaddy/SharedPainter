#include "stdafx.h"
#include "ScreenRecoder.h"
#include "ffmpegwrapper.h"
#include "DefferedCaller.h"
#include "atlconv.h"

static bool ffmpegLibInitialized = false;

class ScreenRecoderThread : public QThread
{
public:
	ScreenRecoderThread( ScreenRecoder *owner ) : owner_(owner) { }
	virtual ~ScreenRecoderThread() 
	{
		qDebug() << "~ScreenRecoderThread()";
	}

	void stop( void )
	{
		wrapper_.stop();
	}

	QString outputFilePath( void ) 
	{
		return outputPath_;
	}

private:

	static QString getFilePath( void )
	{
		QString outputPath = qApp->applicationDirPath() + QDir::separator() + DEFAULT_RECORD_FILE_PATH + QDir::separator();
		QDir dir( outputPath );
		if ( !dir.exists() )
			dir.mkpath( outputPath );
		outputPath += QDateTime::currentDateTime().toString( "yyMMddhhmmss");
		outputPath += ".avi";
		return outputPath;
	}

	void run( void )
	{
		outputPath_ = getFilePath();

		wrapper_.setInputFormatName("dshow");
		wrapper_.setInputFileName("UScreenCapture");
		wrapper_.setOutputCodecName("mpeg4");
		wrapper_.setOutputFileName(outputPath_.toStdString());

		qDebug() << "ffmpeg start : " << outputPath_;

		wrapper_.videoCapture();

		CDefferedCaller::singleShot( boost::bind(&ScreenRecoder::onThreadExit, owner_, outputPath_) );

		qDebug() << "ScreenRecoderThread task finished..";
	}
private:
//	boost::shared_ptr<CNetUdpSession> udpSocket_;
	ScreenRecoder *owner_;
	ffmpegwrapper wrapper_;
	QString outputPath_;

};

ScreenRecoder::ScreenRecoder() : lock_(QMutex::Recursive), thread_(NULL)
{
	if( !ffmpegLibInitialized )
	{
		ffmpegwrapper::initialize();
		ffmpegLibInitialized = true;
	}
}

bool ScreenRecoder::isRecording( void )
{
	lock();
	if( thread_ )
	{
		unlock();
		return true;
	}
	unlock();
	return false;
}

bool ScreenRecoder::recordStart( void )
{
	qDebug() << "recordStart called";

	if( isRecording() )
		return false;

	lock();
	if( !thread_ )
	{
		thread_ = new ScreenRecoderThread( this );
		thread_->setPriority( QThread::LowestPriority );
	}
	thread_->start();
	unlock();

	return true;
}

void ScreenRecoder::recordStop( void )
{
	qDebug() << "recordStop called";

	lock_.lock();
	if( thread_ )
	{
		qDebug() << "recordStop";

		ScreenRecoderThread *impl = (ScreenRecoderThread*)thread_;
		impl->stop();
		waitForFinished();
	}
	lock_.unlock();
}

void ScreenRecoder::waitForFinished( void )
{
	if( NULL == thread_ )
		return;

	thread_->wait();
	delete thread_;
	thread_ = NULL;
}

void ScreenRecoder::onThreadExit( const QString &path )
{
	waitForFinished();

	fireObserver_RecordStop( path, "" );

	qDebug() << "onThreadExit called and thread clear";
}
