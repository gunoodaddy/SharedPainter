#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H

extern "C" {
#include "libavcodec/avcodec.h"
}

class ffmpegwrapper
{
public:
	ffmpegwrapper();

	void init( void );

};

#endif // FFMPEGWRAPPER_H
