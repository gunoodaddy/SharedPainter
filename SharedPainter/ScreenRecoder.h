#pragma once

#include <QThread>

class ScreenRecoderThread;

class ScreenRecoder
{
public:
	ScreenRecoder( void );

	bool isRecording( void );
	bool recordStart( void );
	void recordStop( void );

	void lock( void ) { lock_.lock(); }
	void unlock( void ) { lock_.unlock(); }

private:
	void onThreadExit( void );

private:
	friend class ScreenRecoderThread;
	void run( void );

	QMutex lock_;
	QThread *thread_;
};
