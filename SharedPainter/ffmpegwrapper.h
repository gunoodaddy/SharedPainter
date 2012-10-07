#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H

#include "CompatableAPI.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/audioconvert.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/fifo.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/dict.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
//#include "libavutil/avstring.h"
#include "libavutil/imgutils.h"
//#include "libavutil/timestamp.h"
//#include "libavutil/bprint.h"
#include "libavutil/time.h"
 #include "libavformat/avformat.h"
# include "libavfilter/avcodec.h"
# include "libavfilter/avfilter.h"
# include "libavfilter/avfiltergraph.h"
# include "libavfilter/buffersrc.h"
# include "libavfilter/buffersink.h"

extern AVInputFormat ff_dshow_demuxer;
}

typedef std::vector<std::string> stringlist_t;

class ffmpegwrapper
{
public:
	ffmpegwrapper();

	void init( void );
	stringlist_t getDeviceList( const std::string &type );
};

#endif // FFMPEGWRAPPER_H
