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
#include "libavutil/avstring.h"
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


typedef struct InputFilter {
	AVFilterContext    *filter;
	struct InputStream *ist;
	struct FilterGraph *graph;
	uint8_t            *name;
} InputFilter;

typedef struct FrameBuffer {
	uint8_t *base[4];
	uint8_t *data[4];
	int  linesize[4];

	int h, w;
	enum PixelFormat pix_fmt;

	int refcount;
	struct FrameBuffer **pool;  ///< head of the buffer pool
	struct FrameBuffer *next;
} FrameBuffer;

typedef struct InputStream {
	int file_index;
	AVStream *st;
	int discard;             /* true if stream data should be discarded */
	int decoding_needed;     /* true if the packets must be decoded in 'raw_fifo' */
	AVCodec *dec;
	AVFrame *decoded_frame;

	int64_t       start;     /* time when read started */
	/* predicted dts of the next packet read for this stream or (when there are
	 * several frames in a packet) of the next frame in current packet (in AV_TIME_BASE units) */
	int64_t       next_dts;
	int64_t       dts;       ///< dts of the last packet read for this stream (in AV_TIME_BASE units)

	int64_t       next_pts;  ///< synthetic pts for the next decode frame (in AV_TIME_BASE units)
	int64_t       pts;       ///< current pts of the decoded frame  (in AV_TIME_BASE units)
	int           wrap_correction_done;
	double ts_scale;
	int is_start;            /* is 1 at the start and after a discontinuity */
	int saw_first_ts;
	int showed_multi_packet_warning;
	AVDictionary *opts;
	AVRational framerate;               /* framerate forced with -r */
	int top_field_first;

	int resample_height;
	int resample_width;
	int resample_pix_fmt;

	int      resample_sample_fmt;
	int      resample_sample_rate;
	int      resample_channels;
	uint64_t resample_channel_layout;

	int fix_sub_duration;
	struct { /* previous decoded subtitle and related variables */
		int got_output;
		int ret;
		AVSubtitle subtitle;
	} prev_sub;

	struct sub2video {
		int64_t last_pts;
		AVFilterBufferRef *ref;
		int w, h;
	} sub2video;

	/* a pool of free buffers for decoded data */
	FrameBuffer *buffer_pool;
	int dr1;

	/* decoded data from this stream goes into all those filters
	 * currently video and audio only */
	InputFilter **filters;
	int        nb_filters;
} InputStream;


typedef struct InputFile {
	AVFormatContext *ctx;
	int eof_reached;      /* true if eof reached */
	int eagain;           /* true if last read attempt returned EAGAIN */
	int ist_index;        /* index of first stream in input_streams */
	int64_t ts_offset;
	int nb_streams;       /* number of stream that ffmpeg is aware of; may be different
							 from ctx.nb_streams if new streams appear during av_read_frame() */
	int nb_streams_warn;  /* number of streams that the user was warned of */
	int rate_emu;

//#if HAVE_PTHREADS
//	pthread_t thread;           /* thread reading from this file */
//	int finished;               /* the thread has exited */
//	int joined;                 /* the thread has been joined */
//	pthread_mutex_t fifo_lock;  /* lock for access to fifo */
//	pthread_cond_t  fifo_cond;  /* the main thread will signal on this cond after reading from fifo */
//	AVFifoBuffer *fifo;         /* demuxed packets are stored here; freed by the main thread */
//#endif
} InputFile;


typedef std::vector<std::string> stringlist_t;

class ffmpegwrapper
{
public:
	ffmpegwrapper();

	void init( void );

	bool videoCapture( const std::string &type )
	{
		if( !openInputDevice() )
		{
			return false;
		}

		if( !openOutputDevice() )
		{
			return false;
		}

		if( !transcode() )
		{
			return false;
		}
		return true;
	}

private:
	bool openInputDevice( void );
	bool openOutputDevice( void );
	bool transcode( void );
	bool _add_input_streams( AVFormatContext *ic );

private:
	std::vector< boost::shared_ptr<InputStream> > input_stream_list_;
	std::vector< boost::shared_ptr<InputFile> > input_file_list_;
	std::string input_frame_rate_;
	std::string output_frame_rate_;
};

#endif // FFMPEGWRAPPER_H
