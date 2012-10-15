#include "stdafx.h"
#include "ScreenRecoder.h"
#include "DefferedCaller.h"
#include "atlconv.h"

static bool vlcLibInitialized = false;

#define DEFAULT_LOCAL_STREAM_UDP_PORT 1234


class UdpStreamingThread : public QThread, public INetUdpSessionEvent
{
public:
	UdpStreamingThread( ScreenRecoder *owner ) : owner_(owner) { }

	boost::asio::io_service& io_service() { return io_service_; }
	
	void stop()
	{
		udpSession_->close();
	}
	
private:
	virtual void onINetUdpSessionEvent_Received( CNetUdpSession *session, const std::string buffer )
	{
		owner_->onReadyStreamData( buffer );
	}


private:
	void run( void )
	{
		udpSession_ = boost::shared_ptr<CNetUdpSession>(new CNetUdpSession(io_service_));
		udpSession_->setEvent(this);
		udpSession_->listen(DEFAULT_LOCAL_STREAM_UDP_PORT);

		qDebug() << "UdpStreamingThread run start";
		io_service_.run();
		qDebug() << "UdpStreamingThread run finished";
		
		CDefferedCaller::singleShot( boost::bind(&ScreenRecoder::onThreadExit, owner_, "") );
	}

private:
	ScreenRecoder *owner_;
	boost::shared_ptr<CNetUdpSession> udpSession_;
	boost::asio::io_service io_service_;
};

//
//class ScreenRecoderThread : public QThread
//{
//public:
//	ScreenRecoderThread( ScreenRecoder *owner ) : owner_(owner) { }
//	virtual ~ScreenRecoderThread() 
//	{
//		qDebug() << "~ScreenRecoderThread()";
//	}
//
//	void stop( void )
//	{
//		wrapper_.stop();
//	}
//
//	QString outputFilePath( void ) 
//	{
//		return outputPath_;
//	}
//
//private:
//
//	static QString getFilePath( void )
//	{
//		QString outputPath = qApp->applicationDirPath() + QDir::separator() + DEFAULT_RECORD_FILE_PATH + QDir::separator();
//		QDir dir( outputPath );
//		if ( !dir.exists() )
//			dir.mkpath( outputPath );
//		outputPath += QDateTime::currentDateTime().toString( "yyMMddhhmmss");
//		outputPath += ".avi";
//		return outputPath;
//	}
//
//	void run( void )
//	{
//		outputPath_ = getFilePath();
//
//		wrapper_.setInputFormatName("dshow");
//		wrapper_.setInputFileName("UScreenCapture");
//		wrapper_.setOutputCodecName("mpeg4");
//		wrapper_.setOutputFileName(Util::toUtf8StdString(outputPath_).c_str());
//
//		qDebug() << "ffmpeg start : " << outputPath_;
//
//		wrapper_.videoCapture();
//
//		CDefferedCaller::singleShot( boost::bind(&ScreenRecoder::onThreadExit, owner_, outputPath_) );
//
//		qDebug() << "ScreenRecoderThread task finished..";
//	}
//private:
//	ScreenRecoder *owner_;
//	ffmpegwrapper wrapper_;
//	QString outputPath_;
//
//};

ScreenRecoder::ScreenRecoder() : lock_(QMutex::Recursive), threadUdpStream_(NULL), streaming_(false), recording_(false)
#ifdef __USE_VLC__
	, vlcInstance_(NULL), vlcPlayer_(NULL)
#endif
{
	qDebug() << "ScreenRecoder() called";
#ifdef __USE_VLC__
	if( !vlcLibInitialized )
	{
		//ffmpegwrapper::initialize();
		vlcLibInitialized = true;

		const char * const vlc_args[] = {
			"-I", "dummy", /* Don't use any interface */
			"--ignore-config", /* Don't use VLC's config */
			//"--extraintf=logger", //log anything
			"--verbose=2", //be much more verbose then normal for debugging purpose
		};

		vlcInstance_ = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
	}
#endif
}

ScreenRecoder::~ScreenRecoder()
{
	qDebug() << "~ScreenRecoder() called";
	stopRecord();
	stopStream();
	waitForFinished();

	// TODO: vlcInstance_ remove
}

bool ScreenRecoder::isRecording( void )
{
#ifdef __USE_VLC__
	if( vlcPlayer_ && recording_) 
		return true;
	return false;
#else
	return false;
#endif
}

bool ScreenRecoder::startRecord( void )
{
#ifdef __USE_VLC__
	stopStream();
	stopRecord();

	libvlc_media_t *m;

	m = libvlc_media_new_location(vlcInstance_, "dshow://");

	outputPath_ = getRecordFilePath();
	QString option;
	option = ":sout=#duplicate{dst=display,dst=\"transcode{vcodec=h264,fps=25,acodec=none}:std{access=file,mux=mp4,dst='%1'}\"}";
	option = option.arg(outputPath_);

	libvlc_media_add_option(m, ":dshow-vdev=UScreenCapture");
	libvlc_media_add_option(m,  Util::toUtf8StdString(option).c_str());
	//libvlc_media_add_option(m, ":sout=#duplicate{dst=display,dst=\"transcode{vcodec=h264,fps=25,acodec=none}:std{access=file,mux=mp4,dst='c:\\a.mp4'}\"}");
	//libvlc_media_add_option(m, ":sout=#duplicate{dst=display,dst=std{access=file,mux=avi,dst='c:\\test.avi'}}");
	//libvlc_media_add_option(m, ":sout=#duplicate{dst=display,dst=\"transcode{vcodec=h264,fps=25,acodec=none}:std{access=file,mux=mp4,dst='c:\\a.mp4'}\"}");
		
	if(libvlc_errmsg()) {                                                                                      
		qDebug() << "libvlc" << "Error:" << libvlc_errmsg();
		libvlc_clearerr(); 
	}
	vlcPlayer_ = libvlc_media_player_new_from_media(m);

	libvlc_media_player_set_hwnd (vlcPlayer_, windowId_);

	libvlc_media_release (m);
	libvlc_media_player_play (vlcPlayer_);

	recording_ = true;
	return true;
#else
	return false;
#endif
}

void ScreenRecoder::stopRecord( void )
{
#ifdef __USE_VLC__
	qDebug() << "stopRecord called";

	if( recording_ ) 
	{
		if( vlcPlayer_ ) 
		{
			libvlc_media_player_stop( vlcPlayer_ );
			vlcPlayer_ = NULL;
		}

		recording_ = false;
		waitForFinished();

		fireObserver_RecordStop( outputPath_, "" );
	}
#endif
}


QString ScreenRecoder::getRecordFilePath( void )
{
	QString outputPath = qApp->applicationDirPath() + QDir::separator() + DEFAULT_RECORD_FILE_PATH + QDir::separator();
	QDir dir( outputPath );
	if ( !dir.exists() )
		dir.mkpath( outputPath );
	outputPath += QDateTime::currentDateTime().toString( "yyMMddhhmmss");
	outputPath += ".avi";
#ifdef Q_WS_WIN
	outputPath.replace("/", "\\");
#endif
	return outputPath;
}

void ScreenRecoder::startStreamSend( void ) 
{
	stopRecord();
	stopStream();
#ifdef __USE_VLC__
	libvlc_media_t *m;

	m = libvlc_media_new_location(vlcInstance_, "dshow://");

	QString option;
	option = ":sout=#duplicate{dst=display,dst=\"transcode{vcodec=mpegts,fps=25,vb=4800,scale=1}:rtp{dst=%1,port=%2,mux=ts,ttl=1}\"}";
	option = option.arg("127.0.0.1", QString::number(DEFAULT_LOCAL_STREAM_UDP_PORT));

	libvlc_media_add_option(m, ":dshow-vdev=UScreenCapture");
	libvlc_media_add_option(m,  option.toAscii().data());
	
	if(libvlc_errmsg()) {                                                                                      
		qDebug() << "libvlc" << "Error:" << libvlc_errmsg();
		libvlc_clearerr(); 
	}
	vlcPlayer_ = libvlc_media_player_new_from_media(m);

	libvlc_media_player_set_hwnd (vlcPlayer_, windowId_);

	libvlc_media_release (m);
	libvlc_media_player_play (vlcPlayer_);

	if( !threadUdpStream_ )
	{
		threadUdpStream_ = new UdpStreamingThread( this );
		//threadUdpStream_->setPriority( QThread::LowestPriority );
	}
	threadUdpStream_->start();

	streaming_ = true;
#endif
}


void ScreenRecoder::playStream( const std::string &ip, int port )
{
	stopRecord();
	stopStream();
#ifdef __USE_VLC__
	libvlc_media_t *m;

	QString option;
	option = "rtp://@%1:%2";
	option = option.arg(QString::fromStdString(ip), QString::number(port));

	m = libvlc_media_new_location(vlcInstance_, option.toAscii().data());
	
	//char *opt = ":sout=#duplicate{dst=display,dst=std{access=file,mux=asf,dst=\"C:\\A.asf\"}}";

	//libvlc_media_add_option(m, opt);
	if(libvlc_errmsg()) {                                                                                      
		qDebug() << "libvlc" << "Error:" << libvlc_errmsg();
		libvlc_clearerr(); 
	}
	vlcPlayer_ = libvlc_media_player_new_from_media(m);

	libvlc_media_player_set_hwnd (vlcPlayer_, windowId_);

	libvlc_media_release (m);
	libvlc_media_player_play (vlcPlayer_);

	streaming_ = true;
#endif
}


void ScreenRecoder::stopStream( void )
{
#ifdef __USE_VLC__
	if( vlcPlayer_ ) 
	{
		libvlc_media_player_stop( vlcPlayer_ );
		vlcPlayer_ = NULL;
	}

	streaming_ = false;
	waitForFinished();
#endif
}

bool ScreenRecoder::isPlaying( void )
{
#ifdef __USE_VLC__
	if( vlcPlayer_ && !threadUdpStream_ && streaming_) 
		return true;
	return false;
#else
	return false;
#endif
}

bool ScreenRecoder::isStreaming( void )
{
#ifdef __USE_VLC__
	if( vlcPlayer_ && threadUdpStream_ && streaming_) 
		return true;
	return false;
#else
	return false;
#endif
}


void ScreenRecoder::waitForFinished( void )
{
	if( threadUdpStream_ )
	{
		threadUdpStream_->stop();
		threadUdpStream_->wait();
		delete threadUdpStream_;
		threadUdpStream_ = NULL;
	}
}

void ScreenRecoder::onThreadExit( const QString &path )
{
	waitForFinished();

	qDebug() << "onThreadExit called and thread clear";
}


void ScreenRecoder::onReadyStreamData( const std::string &data )
{
	fireObserver_ReadyStreamingData( data );
}	
