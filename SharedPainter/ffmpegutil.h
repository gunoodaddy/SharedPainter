#pragma once

#include "compat_win32.h"

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
#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"
	//#include "libavutil/timestamp.h"
#include "libavutil/bprint.h"
#include "libavutil/time.h"
#include "libavutil/pixdesc.h"
#include "libavformat/avformat.h"
#include "libavfilter/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
}

#define DESCRIBE_FILTER_LINK(f, inout, in)                         \
	{                                                                  \
	AVFilterContext *ctx = inout->filter_ctx;                      \
	AVFilterPad *pads = in ? ctx->input_pads  : ctx->output_pads;  \
	int       nb_pads = in ? ctx->input_count : ctx->output_count; \
	AVIOContext *pb;                                               \
	\
	if (avio_open_dyn_buf(&pb) < 0)                                \
	return -1;                                                 \
	\
	avio_printf(pb, "%s", ctx->filter->name);                      \
	if (nb_pads > 1)                                               \
	avio_printf(pb, ":%s", avfilter_pad_get_name(pads, inout->pad_idx));\
	avio_w8(pb, 0);                                                \
	avio_close_dyn_buf(pb, &f->name);                              \
	}

namespace ffmpegutil {

	struct FilterGraph;

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

	typedef struct OutputFilter {                                                                                                                                
		AVFilterContext     *filter;
		struct OutputStream *ost;
		struct FilterGraph  *graph;
		uint8_t             *name;

		/* temporary storage until stream maps are processed */
		AVFilterInOut       *out_tmp;
	} OutputFilter;

	typedef struct OutputStream {
		int file_index;          /* file index */
		int index;               /* stream index in the output file */
		int source_index;        /* InputStream index */
		AVStream *st;            /* stream in the output file */
		int encoding_needed;     /* true if encoding needed for this stream */
		int frame_number;
		/* input pts and corresponding output pts
		for A/V sync */
		boost::shared_ptr<InputStream> sync_ist; /* input stream to sync against */
		int64_t sync_opts;       /* output frame counter, could be changed to some true timestamp */ // FIXME look at frame_number
		/* pts of the first frame encoded for this stream, used for limiting
		* recording time */
		int64_t first_pts;
		AVBitStreamFilterContext *bitstream_filters;
		AVCodec *enc;
		int64_t max_frames;
		AVFrame *filtered_frame;

		/* video only */
		AVRational frame_rate;
		int force_fps;
		int top_field_first;

		float frame_aspect_ratio;
		float last_quality;

		/* forced key frames */
		int64_t *forced_kf_pts;
		int forced_kf_count;
		int forced_kf_index;
		char *forced_keyframes;

		/* audio only */
		int audio_channels_map[SWR_CH_MAX];  /* list of the channels id to pick from the source stream */
		int audio_channels_mapped;           /* number of channels in audio_channels_map */

		/*
		char *logfile_prefix;
		FILE *logfile;
		*/

		OutputFilter *filter;
		char *avfilter;

		int64_t sws_flags;
		int64_t swr_dither_method;
		double swr_dither_scale;
		AVDictionary *opts;
		int finished;        /* no more packets should be written for this stream */
		int unavailable;                     /* true if the steram is unavailable (possibly temporarily) */
		int stream_copy;
		const char *attachment_filename;
		int copy_initial_nonkeyframes;                    
		int copy_prior_start;

		int keep_pix_fmt;
	} OutputStream;

	typedef struct OutputFile {
		AVFormatContext *ctx;
		AVDictionary *opts;
		int ost_index;       /* index of the first stream in output_streams */
		int64_t recording_time;  ///< desired length of the resulting file in microseconds == AV_TIME_BASE units
		int64_t start_time;      ///< start time in microseconds == AV_TIME_BASE units
		uint64_t limit_filesize; /* filesize limit expressed in bytes */

		int shortest;
	} OutputFile;

	typedef struct FilterGraph {
		int            index;                                                                                                                                    
		const char    *graph_desc;

		AVFilterGraph *graph;

		InputFilter   **inputs; 
		int          nb_inputs;
		OutputFilter **outputs;
		int         nb_outputs;
	} FilterGraph;


	static volatile int received_nb_signals = 0;

	static int decode_interrupt_cb(void *ctx)
	{
		return received_nb_signals > 1;
	}

	static void log_callback_null(void *ptr, int level, const char *fmt, va_list vl)
	{
		//char log[4096];
		//va_start(vl, fmt);
		//vsprintf(log, fmt, vl);
		//va_end(vl);

		if( level >= AV_LOG_VERBOSE )
			return;

		qDebug() << QString(fmt).trimmed();
	}


	static AVCodec *find_codec( const std::string &name, enum AVMediaType type, int encoder )
	{               
		const AVCodecDescriptor *desc;
		const char *codec_string = encoder ? "encoder" : "decoder";
		AVCodec *codec;

		codec = encoder ?
			avcodec_find_encoder_by_name(name.c_str()) :
		avcodec_find_decoder_by_name(name.c_str());

		if (!codec && (desc = avcodec_descriptor_get_by_name(name.c_str()))) 
		{
			codec = encoder ? avcodec_find_encoder(desc->id) : avcodec_find_decoder(desc->id);
		}           

		if (!codec) 
			return NULL;     

		if (codec->type != type) 
			return NULL;   

		return codec;
	}   

	static void choose_encoder( const std::string &codec_name, AVFormatContext *s, boost::shared_ptr<OutputStream> ost )
	{
		if ( codec_name.empty() ) 
		{
			ost->st->codec->codec_id = av_guess_codec(s->oformat, NULL, s->filename,
				NULL, ost->st->codec->codec_type);
			ost->enc = avcodec_find_encoder(ost->st->codec->codec_id);
		} 
		else if ( codec_name == "copy" )
			ost->stream_copy = 1;
		else 
		{
			ost->enc = find_codec( codec_name, ost->st->codec->codec_type, 1 );
			ost->st->codec->codec_id = ost->enc->id;
		}
	}

	static int alloc_buffer(FrameBuffer **pool, AVCodecContext *s, FrameBuffer **pbuf)
	{
		FrameBuffer *buf = (FrameBuffer*)av_mallocz(sizeof(*buf));
		int i, ret;
		const int pixel_size = av_pix_fmt_descriptors[s->pix_fmt].comp[0].step_minus1+1;
		assert(false);

		int h_chroma_shift, v_chroma_shift;
		int edge = 32; // XXX should be avcodec_get_edge_width(), but that fails on svq1
		int w = s->width, h = s->height;

		if (!buf)
			return AVERROR(ENOMEM);

		avcodec_align_dimensions(s, &w, &h);

		if (!(s->flags & CODEC_FLAG_EMU_EDGE)) {
			w += 2*edge;
			h += 2*edge;
		}

		if ((ret = av_image_alloc(buf->base, buf->linesize, w, h,
			s->pix_fmt, 32)) < 0) {
				av_freep(&buf);
				av_log(s, AV_LOG_ERROR, "alloc_buffer: av_image_alloc() failed\n");
				return ret;
		}
		/* XXX this shouldn't be needed, but some tests break without this line
		* those decoders are buggy and need to be fixed.
		* the following tests fail:
		* cdgraphics, ansi, aasc, fraps-v1, qtrle-1bit
		*/
		memset(buf->base[0], 128, ret);

		avcodec_get_chroma_sub_sample(s->pix_fmt, &h_chroma_shift, &v_chroma_shift);
		for (i = 0; i < FF_ARRAY_ELEMS(buf->data); i++) {
			const int h_shift = i==0 ? 0 : h_chroma_shift;
			const int v_shift = i==0 ? 0 : v_chroma_shift;
			if ((s->flags & CODEC_FLAG_EMU_EDGE) || !buf->linesize[i] || !buf->base[i])
				buf->data[i] = buf->base[i];
			else
				buf->data[i] = buf->base[i] +
				FFALIGN((buf->linesize[i]*edge >> v_shift) +
				(pixel_size*edge >> h_shift), 32);
		}
		buf->w       = s->width;
		buf->h       = s->height;
		buf->pix_fmt = s->pix_fmt;
		buf->pool    = pool;

		*pbuf = buf;
		return 0;
	}

	static void unref_buffer(FrameBuffer *buf)
	{     
		FrameBuffer **pool = buf->pool;

		av_assert0(buf->refcount > 0);
		buf->refcount--;    
		if (!buf->refcount) {
			FrameBuffer *tmp;
			for(tmp= *pool; tmp; tmp= tmp->next)
				av_assert1(tmp != buf);

			buf->next = *pool;
			*pool = buf;
		} 
	}  

	static int codec_get_buffer( AVCodecContext *s, AVFrame *frame )
	{
		FrameBuffer **pool = (FrameBuffer**)s->opaque;
		FrameBuffer *buf;
		int ret, i;

		if(av_image_check_size(s->width, s->height, 0, s) || s->pix_fmt<0) {
			av_log(s, AV_LOG_ERROR, "codec_get_buffer: image parameters invalid\n");
			return -1;
		}

		if (!*pool && (ret = alloc_buffer(pool, s, pool)) < 0)
			return ret;

		buf              = *pool;
		*pool            = buf->next;
		buf->next        = NULL;
		if (buf->w != s->width || buf->h != s->height || buf->pix_fmt != s->pix_fmt) {
			av_freep(&buf->base[0]);
			av_free(buf);
			if ((ret = alloc_buffer(pool, s, &buf)) < 0)
				return ret;
		}
		av_assert0(!buf->refcount);
		buf->refcount++;

		frame->opaque        = buf;
		frame->type          = FF_BUFFER_TYPE_USER;
		frame->extended_data = frame->data;
		frame->pkt_pts       = s->pkt ? s->pkt->pts : AV_NOPTS_VALUE;
		frame->width         = buf->w;
		frame->height        = buf->h;
		frame->format        = buf->pix_fmt;
		frame->sample_aspect_ratio = s->sample_aspect_ratio;

		for (i = 0; i < FF_ARRAY_ELEMS(buf->data); i++) {
			frame->base[i]     = buf->base[i];  // XXX h264.c uses base though it shouldn't
			frame->data[i]     = buf->data[i];
			frame->linesize[i] = buf->linesize[i];
		}

		return 0;
	}

	static void codec_release_buffer(AVCodecContext *s, AVFrame *frame)
	{     
		FrameBuffer *buf = (FrameBuffer*)frame->opaque;
		int i;

		if(frame->type!=FF_BUFFER_TYPE_USER) {
			avcodec_default_release_buffer(s, frame);
			return;
		} 

		for (i = 0; i < FF_ARRAY_ELEMS(frame->data); i++)
			frame->data[i] = NULL;

		unref_buffer(buf);
	}  

	static void *grow_array(void *array, int elem_size, int *size, int new_size)                                                                                        
	{                     
		if (new_size >= INT_MAX / elem_size) 
		{
			av_log(NULL, AV_LOG_ERROR, "Array too big.\n");
			exit(1);      
		}   
		if (*size < new_size) 
		{
			uint8_t *tmp = (uint8_t *)av_realloc(array, new_size*elem_size);
			if (!tmp) 
			{
				av_log(NULL, AV_LOG_ERROR, "Could not alloc buffer.\n");
				exit(1);
			}
			memset(tmp + *size*elem_size, 0, (new_size-*size) * elem_size);
			*size = new_size; 
			return tmp;   
		}
		return array;     
	}  


	static int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec)                                                                               
	{           
		int ret = avformat_match_stream_specifier(s, st, spec);
		if (ret < 0)
			av_log(s, AV_LOG_ERROR, "Invalid stream specifier: %s.\n", spec);
		return ret;
	}               

	static enum PixelFormat choose_pixel_fmt(AVStream *st, AVCodec *codec, enum PixelFormat target)
	{                   
		if (codec && codec->pix_fmts) 
		{
			const enum PixelFormat *p = codec->pix_fmts;
			const enum PixelFormat temp1[] = { PIX_FMT_YUVJ420P, PIX_FMT_YUVJ422P, PIX_FMT_YUV420P, PIX_FMT_YUV422P, PIX_FMT_NONE };
			const enum PixelFormat temp2[] = { PIX_FMT_YUVJ420P, PIX_FMT_YUVJ422P, PIX_FMT_YUVJ444P, PIX_FMT_YUV420P,
				PIX_FMT_YUV422P, PIX_FMT_YUV444P, PIX_FMT_BGRA, PIX_FMT_NONE };

			int has_alpha= av_pix_fmt_descriptors[target].nb_components % 2 == 0;
			enum PixelFormat best= PIX_FMT_NONE;
			if (st->codec->strict_std_compliance <= FF_COMPLIANCE_UNOFFICIAL) 
			{
				if (st->codec->codec_id == AV_CODEC_ID_MJPEG) 
				{
					p = temp1;
				} 
				else if (st->codec->codec_id == AV_CODEC_ID_LJPEG) 
				{
					p = temp2;
				}       
			}           
			for (; *p != PIX_FMT_NONE; p++) 
			{
				best= avcodec_find_best_pix_fmt_of_2(best, *p, target, has_alpha, NULL);
				if (*p == target)
					break;
			}           
			if (*p == PIX_FMT_NONE) 
			{
				//if (target != PIX_FMT_NONE)
				//    av_log(NULL, AV_LOG_WARNING,
				//           "Incompatible pixel format '%s' for codec '%s', auto-selecting format '%s'\n",
				//           av_pix_fmt_descriptors[target].name,
				//           codec->name,
				//           av_pix_fmt_descriptors[best].name);
				return best;
			}           
		}               
		return target;  
	}   

	static char *choose_pix_fmts(OutputStream *ost)
	{
		if (ost->keep_pix_fmt) 
		{
			if (ost->filter)
				avfilter_graph_set_auto_convert(ost->filter->graph->graph,
				AVFILTER_AUTO_CONVERT_NONE);
			if (ost->st->codec->pix_fmt == PIX_FMT_NONE)
				return NULL;
			return av_strdup(av_get_pix_fmt_name(ost->st->codec->pix_fmt));
		}
		if (ost->st->codec->pix_fmt != PIX_FMT_NONE) 
		{
			return av_strdup(av_get_pix_fmt_name(choose_pixel_fmt(ost->st, ost->enc, ost->st->codec->pix_fmt)));
		} 
		else if (ost->enc && ost->enc->pix_fmts) 
		{
			const enum PixelFormat *p;
			AVIOContext *s = NULL;
			uint8_t *ret;
			int len;

			if (avio_open_dyn_buf(&s) < 0)
				exit(1);

			const enum PixelFormat temp1[] = { PIX_FMT_YUVJ420P, PIX_FMT_YUVJ422P, PIX_FMT_YUV420P, PIX_FMT_YUV422P, PIX_FMT_NONE };
			const enum PixelFormat temp2[] = { PIX_FMT_YUVJ420P, PIX_FMT_YUVJ422P, PIX_FMT_YUVJ444P, PIX_FMT_YUV420P,
				PIX_FMT_YUV422P, PIX_FMT_YUV444P, PIX_FMT_BGRA, PIX_FMT_NONE };
			p = ost->enc->pix_fmts;
			if (ost->st->codec->strict_std_compliance <= FF_COMPLIANCE_UNOFFICIAL) 
			{
				if (ost->st->codec->codec_id == AV_CODEC_ID_MJPEG) 
				{
					p = temp1;
				} 
				else if (ost->st->codec->codec_id == AV_CODEC_ID_LJPEG) 
				{
					p = temp2;
				}
			}

			for (; *p != PIX_FMT_NONE; p++)
			{
				const char *name = av_get_pix_fmt_name(*p);
				avio_printf(s, "%s:", name);
			}
			len = avio_close_dyn_buf(s, &ret);
			ret[len - 1] = 0;
			return (char*)ret;
		} 
		else
			return NULL;
	}

	static AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
		AVFormatContext *s, AVStream *st, AVCodec *codec)
	{
		AVDictionary *ret = NULL;
		AVDictionaryEntry *t = NULL;
		int flags = s->oformat ? AV_OPT_FLAG_ENCODING_PARAM : AV_OPT_FLAG_DECODING_PARAM;
		char prefix = 0;
		const AVClass *cc = avcodec_get_class();

		if (!codec)
			codec = s->oformat ? avcodec_find_encoder(codec_id) : avcodec_find_decoder(codec_id);

		if (!codec)
			return NULL;

		switch (codec->type) 
		{
		case AVMEDIA_TYPE_VIDEO:
			prefix  = 'v';
			flags  |= AV_OPT_FLAG_VIDEO_PARAM;
			break;
		case AVMEDIA_TYPE_AUDIO:
			prefix  = 'a';
			flags  |= AV_OPT_FLAG_AUDIO_PARAM;
			break;
		case AVMEDIA_TYPE_SUBTITLE:
			prefix  = 's';
			flags  |= AV_OPT_FLAG_SUBTITLE_PARAM;
			break;
		}

		while (t = av_dict_get(opts, "", t, AV_DICT_IGNORE_SUFFIX)) {
			char *p = strchr(t->key, ':');

			// check stream specification in opt name
			if (p)
				switch (check_stream_specifier(s, st, p + 1)) 
			{
				case  1: *p = 0; break;
				case  0:         continue;
				default:         return NULL;
			}

			if (av_opt_find(&cc, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ) ||
				(codec && codec->priv_class &&
				av_opt_find(&codec->priv_class, t->key, NULL, flags,
				AV_OPT_SEARCH_FAKE_OBJ)))
				av_dict_set(&ret, t->key, t->value, 0);
			else if (t->key[0] == prefix &&
				av_opt_find(&cc, t->key + 1, NULL, flags,
				AV_OPT_SEARCH_FAKE_OBJ))
				av_dict_set(&ret, t->key + 1, t->value, 0);

			if (p)
				*p = ':';
		}
		return ret;
	}
}