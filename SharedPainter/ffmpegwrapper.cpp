#include "stdafx.h"
#include "ffmpegwrapper.h"


// http://nerdlogger.com/2011/11/03/stream-your-windows-desktop-using-ffmpeg/
// ffmpeg -f dshow  -i video="UScreenCapture"  -r 30 -vcodec mpeg4 -q 12 a.avi
// ffmpeg -list_devices true -f dshow -i dummy
// >>> libavdevice/openal-dec.c

ffmpegwrapper::ffmpegwrapper()
{
	input_frame_rate_ = "30";
	output_frame_rate_ = "30";
}


static volatile int received_nb_signals = 0;

static int decode_interrupt_cb(void *ctx)
{
	return received_nb_signals > 1;
}

const AVIOInterruptCB int_cb = { decode_interrupt_cb, NULL };

static void log_callback_null(void *ptr, int level, const char *fmt, va_list vl)
{
	char log[4096];
	va_start(vl, fmt);
	vsprintf(log, fmt, vl);
	va_end(vl);

	qDebug() << log;
}

void ffmpegwrapper::init( void )
{
	av_log_set_callback( log_callback_null );
	avcodec_register_all();
	avdevice_register_all();
	avfilter_register_all();
	av_register_all();
	avformat_network_init();
}

bool ffmpegwrapper::_add_input_streams( AVFormatContext *ic )
{
	input_stream_list_.clear();

	for ( size_t i = 0; i < ic->nb_streams; i++) {
		AVStream *st = ic->streams[i];
		AVCodecContext *dec = st->codec;
		boost::shared_ptr<InputStream> ist( new InputStream );
		memset( ist.get(), 0, sizeof(InputStream) );

		if (!ist)
			return false;

		ist->st = st;
		ist->file_index = 0;
		ist->discard = 1;
		st->discard  = AVDISCARD_ALL;
		//ist->opts = filter_codec_opts(codec_opts, ist->st->codec->codec_id, ic, st, choose_decoder(o, ic, st));
		ist->ts_scale = 1.0;
		// codec_tag
		//st->codec->codec_tag = tag;

		// video_type setting
		//ist->dec = choose_decoder(o, ic, st);
		ist->dec = avcodec_find_decoder(st->codec->codec_id);

		if(!ist->dec)
			ist->dec = avcodec_find_decoder(dec->codec_id);

		if (dec->lowres)
			dec->flags |= CODEC_FLAG_EMU_EDGE;

		ist->resample_height  = dec->height;
		ist->resample_width   = dec->width;
		ist->resample_pix_fmt = dec->pix_fmt;
		//		if (av_parse_video_rate(&ist->framerate, input_frame_rate_.c_str()) < 0) {
		//			av_log(NULL, AV_LOG_ERROR, "Error parsing framerate %s.\n",
		//				   framerate);
		//			exit(1);
		//		}

		ist->top_field_first = -1;

		input_stream_list_.push_back( ist );
	}

	return true;
}


bool ffmpegwrapper::openInputDevice( void )
{
	AVFormatContext *ic = NULL;
	AVDictionary* options = NULL;
	AVDictionary **input_opts = NULL;
	//av_dict_set( &options, "list_devices", "true", 0 );

	int err = 0;
	int ret = 0;
	int orig_nb_streams = 0;
	int i;

	bool res = false;
	do
	{
		ic = avformat_alloc_context();
		if( !ic )
			break;

		AVInputFormat *iformat = av_find_input_format( "dshow" );
		if( !iformat )
			break;

		err = avformat_open_input( &ic, "video=UScreenCapture", iformat, &options );
		if( err != 0 )
			break;

		// TODO:apply forced codec ids
		//		for (int i = 0; i < ic->nb_streams; i++)
		//			choose_decoder(o, ic, ic->streams[i]);

		// setting stream option

		orig_nb_streams = ic->nb_streams;

		input_opts = (AVDictionary **)av_mallocz( ic->nb_streams * sizeof(*input_opts) );
		if( !input_opts )
			break;

		// TODO:CODEC OPTION SETTING
		//		for ( i = 0; i < ic->nb_streams; i++)
		//			  opts[i] = filter_codec_opts(codec_opts, ic->streams[i]->codec->codec_id,
		//										  s, s->streams[i], NULL);
		ret = avformat_find_stream_info( ic, input_opts );
		if( ret < 0 )
			break;

		_add_input_streams( ic );

		boost::shared_ptr<InputFile> inputFile( new InputFile );
		memset( inputFile.get(), 0, sizeof(InputFile) );

		inputFile->ctx        = ic;
		inputFile->ist_index  = input_stream_list_.size() - ic->nb_streams;
		//inputFile->ts_offset  = o->input_ts_offset - (copy_ts ? 0 : timestamp);
		inputFile->nb_streams = ic->nb_streams;
		//inputFile->rate_emu   = o->rate_emu;
		input_file_list_.push_back( inputFile );

		res = true;
	}while( false );

	for (i = 0; i < orig_nb_streams; i++)
		av_dict_free(&input_opts[i]);
	av_freep(&input_opts);

	return res;
}


bool ffmpegwrapper::openOutputDevice( void )
{
	AVFormatContext *oc = NULL;
	AVDictionary* options = NULL;
	av_dict_set( &options, "q", "12", 0 );

	const char *filename = "c:\\keh.avi";
	int err = 0;
	int i;
	bool res = false;
	do
	{
		oc = avformat_alloc_context();
		if( !oc )
			break;

		AVOutputFormat *oformat = av_guess_format( NULL, filename, NULL );
		if( !oformat )
			break;
		oc->oformat = oformat;

		/* set the format-level framerate option;
		 * this is important for video grabbers, e.g. x11 */
		if( oformat && oformat->priv_class &&
				av_opt_find( &oformat->priv_class, "framerate", NULL, 0, AV_OPT_SEARCH_FAKE_OBJ ) )
		{
			av_dict_set(&options, "framerate", output_frame_rate_.c_str(), 0);
		}

		// mpeg4 decoder setting
		//AVCodec *codec = avcodec_find_decoder_by_name( "mpeg4" );
		//if( !codec )
		//	break;
		//oc->video_codec_id = codec->id;
		//oc->flags |= AVFMT_FLAG_NONBLOCK;
		//oc->interrupt_callback = int_cb;

		if( oc->oformat->priv_data_size > 0 )
		{
			oc->priv_data = av_mallocz( oc->oformat->priv_data_size );
			if( !oc->priv_data )
				break;
			if( oc->oformat->priv_class )
			{
				*(const AVClass**)oc->priv_data = oc->oformat->priv_class;
				av_opt_set_defaults( oc->priv_data );
			}
		}
		else
			oc->priv_data = NULL;

		//char *subtitle_codec_name = NULL;

		// video: highest resolution
		if (oc->oformat->video_codec != AV_CODEC_ID_NONE) {
//			int area = 0, idx = -1;
//			int qcr = avformat_query_codec(oc->oformat, oc->oformat->video_codec, 0);
			/*
		  for ( i = 0; i < nb_input_streams; i++) {
			  int new_area;
			  ist = input_streams[i];
			  new_area = ist->st->codec->width * ist->st->codec->height;
			  if((qcr!=MKTAG('A', 'P', 'I', 'C')) && (ist->st->disposition & AV_DISPOSITION_ATTACHED_PIC))
				  new_area = 1;
			  if (ist->st->codec->codec_type == AVMEDIA_TYPE_VIDEO &&
					  new_area > area) {
				  if((qcr==MKTAG('A', 'P', 'I', 'C')) && !(ist->st->disposition & AV_DISPOSITION_ATTACHED_PIC))
					  continue;
				  area = new_area;
				  idx = i;
			  }
		  }

		  if (idx >= 0)
			  new_video_stream(o, oc, idx);
		  */
		}

		av_strlcpy( oc->filename, filename, sizeof(oc->filename) );

		// open the file
		if ((err = avio_open2(&oc->pb, filename, AVIO_FLAG_WRITE,
							  &oc->interrupt_callback,
							  &options)) < 0)
		{
			break;
		}

		res = true;

	}while( false );

	return res;
}

bool ffmpegwrapper::transcode( void )
{
	return true;
}
