#pragma once

#include <QThread>

class ScreenRecoderThread;
class ScreenRecoder;

class IScreenRecorderEvent
{
public:
	virtual void onIScreenRecorderEvent_RecordStop( ScreenRecoder *self, const QString &filePath, const std::string &errorMsg ) = 0;
};


class ScreenRecoder
{
public:
	ScreenRecoder( void );
	
	void registerObserver( IScreenRecorderEvent *obs )
	{
		observers_.remove( obs );
		observers_.push_back( obs );
	}

	void unregisterObserver( IScreenRecorderEvent *obs )
	{
		observers_.remove( obs );
	}

	bool isRecording( void );
	bool recordStart( void );
	void recordStop( void );

	void lock( void ) { lock_.lock(); }
	void unlock( void ) { lock_.unlock(); }

private:
	void waitForFinished( void );
	void onThreadExit( const QString &path );
	void run( void );

private:
	void fireObserver_RecordStop(  const QString &filePath, const std::string &errorMsg )
	{
		std::list<IScreenRecorderEvent *> observers = observers_;
		for( std::list<IScreenRecorderEvent *>::iterator it = observers.begin(); it != observers.end(); it++ )
		{
			(*it)->onIScreenRecorderEvent_RecordStop( this, filePath, errorMsg );
		}
	}

private:
	friend class ScreenRecoderThread;

	// obsevers
	std::list<IScreenRecorderEvent *> observers_;
	QMutex lock_;
	QThread *thread_;
	QString outputPath_;
};
