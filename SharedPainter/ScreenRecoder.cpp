#include "stdafx.h"
#include "ScreenRecoder.h"

class ScreenRecoderThread : public QThread, public INetUdpSessionEvent
{
public:
	ScreenRecoderThread( ScreenRecoder *owner ) : stop_(false), owner_(owner), process_(NULL) { }

	void processStop( void )
	{
		qDebug() << "processStop";
		owner_->lock();
		qDebug() << "processStop1";
		stop_ = true;
		udpSocket_->close();
		QString program = qApp->applicationDirPath() + QDir::separator() +  "ffmpeg" + QDir::separator() + "kill_ffmpeg.bat";
		QProcess::execute( program );
		qDebug() << "processStop2";
		owner_->unlock();
		qDebug() << "processStop3";
	}

private:
	virtual void onINetUdpSessionEvent_Received( CNetUdpSession *session, const std::string buffer )
	{
		qDebug() << "onINetUdpSessionEvent_Received" << buffer.size();
		recordData_.append( buffer );
	}

	void run( void )
	{
		owner_->lock();
		process_ = new QProcess();
		owner_->unlock();

		QString program = qApp->applicationDirPath() + QDir::separator() +  "ffmpeg" + QDir::separator() + "ffmpeg.exe";
		QStringList arguments;

		qDebug() << program;

		udpSocket_ = boost::shared_ptr<CNetUdpSession>(new CNetUdpSession( NetServiceRunnerPtr()->io_service() ) );
		udpSocket_->setEvent( this );
		udpSocket_->listen( 5555 );

		while( !stop_ )
		{
			Sleep(1000);
			qDebug() << "------------------------------";
		}
		qDebug() << "------------------------------ STOPPED ---------------------------------";

		if(0)
		{
			//arguments << "-f" << "dshow";
			arguments << "-f" << "mpegts";
			arguments << "-i" << "video=UScreenCapture";
			arguments << "-r" << "30";
			arguments << "-vcodec" << "mpeg4";
			arguments << "-q" << "12";
			arguments << "udp://127.0.0.1:5555?pkt_size=188?buffer_size=65535";
			//arguments << "c:\\output.avi";

			process_->start(program, arguments);
			process_->waitForFinished();
			process_->close();
		}

		QFile f("c:/test.avi");
		if( f.open( QIODevice::WriteOnly ) )
		{
			qDebug() << "FILE OPEN";
			QDataStream out(&f);
			int ret = out.writeRawData( recordData_.c_str(), recordData_.size() );
			if( ret != (int)recordData_.size() )
			{
				qDebug() << "FILE WRITE FAILED";
			}
		}
		else
		{
			qDebug() << "FILE OPEN FAILED";
		}



		owner_->lock();
		delete process_;
		process_ = NULL;
		owner_->unlock();

		owner_->onThreadExit();
		//delete this;

		qDebug() << program;
		qDebug() << arguments;
	}
private:
	bool stop_;
	boost::shared_ptr<CNetUdpSession> udpSocket_;
	ScreenRecoder *owner_;
	QProcess *process_;
	std::string recordData_;

};

ScreenRecoder::ScreenRecoder() : lock_(QMutex::Recursive), thread_(NULL)
{
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
		impl->processStop();
	}
	lock_.unlock();
}

void ScreenRecoder::onThreadExit( void )
{
	thread_ = NULL;
}
