#pragma once

#include <QThread>
#define __USE_VLC__
#ifdef __USE_VLC__
#include <vlc/vlc.h>
#endif

class UdpStreamingThread;
class ScreenRecoder;

class IScreenRecorderEvent
{
public:
	virtual void onIScreenRecorderEvent_RecordStop( ScreenRecoder *self, const QString &filePath, const std::string &errorMsg ) = 0;	// called from another thread..
	virtual void onIScreenRecorderEvent_ReadyStreamingData( ScreenRecoder *self, const std::string &streamData ) = 0;
};


class ScreenRecoder
{
public:
	ScreenRecoder( void );
	~ScreenRecoder( void );

	void registerObserver( IScreenRecorderEvent *obs )
	{
		observers_.remove( obs );
		observers_.push_back( obs );
	}

	void unregisterObserver( IScreenRecorderEvent *obs )
	{
		observers_.remove( obs );
	}

	void setWindowId( WId id )
	{
		windowId_ = id;
	}

	bool isRecording( void );
	bool startRecord( void );
	void stopRecord( void );

	void startStreamSend( void );
	void playStream( const std::string &ip, int port );
	void stopStream( void );
	bool isPlaying( void );
	bool isStreaming( void );

	void lock( void ) { lock_.lock(); }
	void unlock( void ) { lock_.unlock(); }

private:
	static QString getRecordFilePath( void );
	void waitForFinished( void );
	void onReadyStreamData( const std::string &data );
	void onThreadExit( const QString &path );
	void run( void );

private:
	void fireObserver_ReadyStreamingData( const std::string &streamData )
	{
		std::list<IScreenRecorderEvent *> observers = observers_;
		for( std::list<IScreenRecorderEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onIScreenRecorderEvent_ReadyStreamingData( this, streamData );
		}
	}

	void fireObserver_RecordStop(  const QString &filePath, const std::string &errorMsg )
	{
		std::list<IScreenRecorderEvent *> observers = observers_;
		for( std::list<IScreenRecorderEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onIScreenRecorderEvent_RecordStop( this, filePath, errorMsg );
		}
	}

private:
	friend class UdpStreamingThread;

	// obsevers
	std::list<IScreenRecorderEvent *> observers_;
	QMutex lock_;
	UdpStreamingThread *threadUdpStream_;
	QString outputPath_;

	WId windowId_;

	volatile bool streaming_;
	volatile bool recording_;
#ifdef __USE_VLC__
	libvlc_instance_t *vlcInstance_;
	libvlc_media_player_t *vlcPlayer_;
#endif
};
