#include "stdafx.h"
#include "ffmpegwrapper.h"

// http://nerdlogger.com/2011/11/03/stream-your-windows-desktop-using-ffmpeg/
// ffmpeg -f dshow  -i video="UScreenCapture"  -r 30 -vcodec mpeg4 -q 12 a.avi
// ffmpeg -list_devices true -f dshow -i dummy
ffmpegwrapper::ffmpegwrapper()
{
}

void ffmpegwrapper::init( void )
{
	avcodec_register_all();
	avfilter_register_all();
	av_register_all();
	avformat_network_init();
}

stringlist_t ffmpegwrapper::getDeviceList( const std::string &type )
{
	stringlist_t res;
	return res;
}
