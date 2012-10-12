#include "stdafx.h"
#include "ffmpegwrapper.h"

#include "stdint.h"

#define DEFAULT_MUX_MAX_DELAY	0.7

AVRational _AV_TIME_BASE_Q = {1, AV_TIME_BASE};
const AVIOInterruptCB int_cb = { decode_interrupt_cb, NULL };

// http://nerdlogger.com/2011/11/03/stream-your-windows-desktop-using-ffmpeg/
// ffmpeg -f dshow  -i video="UScreenCapture"  -r 30 -vcodec mpeg4 -q 12 a.avi
// ffmpeg -f dshow  -i video="UScreenCapture"  -r 30 -vcodec mpeg4 -q 12 -f mpegts udp://1127.0.0.1:1234?pkt_size=188?buffer_size=65535
// ffmpeg -list_devices true -f dshow -i dummy
// >>> libavdevice/openal-dec.c
//------------------------------------------------------------------------------------------

ffmpegwrapper::ffmpegwrapper()
{
	metadata_global_manual_ = 0;
	input_ts_offset_ = 0;
	rate_emu_ = 0;
	start_time_ = 0;
	recording_time_ = INT64_MAX;
	limit_filesize_ = UINT64_MAX;
	copy_ts_ = false;
	nb_frames_dup_ = 0;
	dts_delta_threshold_ = 10.f;
	dts_error_threshold_ = 3600*30;
	nb_frames_drop_ = 0;
	same_quant_ = 0;
	audio_size_ = 0;
	video_size_ = 0;
	subtitle_size_ = 0;
	stop_ = false;
	do_deinterlace_ = false;
	extra_size_ = 0;
	input_frame_rate_ = "30";
	output_frame_rate_ = "30";
	quality_ = 12;
	video_sync_method_ = VSYNC_AUTO;
	audio_sync_method_ = 0;
	outputFileName_ = "";

	codec_opts_ = NULL;
}


// "video=UScreenCapture"
void ffmpegwrapper::setInputFileName( const std::string &name )
{
	inputFileName_ = "video=" + name;
}

// dshow
void ffmpegwrapper::setInputFormatName( const std::string &name )
{
	inputFormatName_ = name;
}


void ffmpegwrapper::setOutputCodecName( const std::string &name )
{
	outputCodecName_ = name;
}

void ffmpegwrapper::setOutputFileName( const std::string &name )
{
	outputFileName_ = name;
}

void ffmpegwrapper::setOutputQuality( int quality )
{
	quality_ = quality;
}

void ffmpegwrapper::setRecordTime( int second ) 
{
	if( recording_time_ != UINT64_MAX )
	{
		char str[100] = {0,};
		sprintf(str, "%d", second);
		int64_t us;
		if (av_parse_time(&us, str, 1) < 0)
			recording_time_ = UINT64_MAX;
		else
			recording_time_ = us;
	}
}

void ffmpegwrapper::init( void )
{
	swr_opts_ = swr_alloc();
	sws_opts_ = sws_getContext(16, 16, (PixelFormat)0, 16, 16, (PixelFormat)0, SWS_BICUBIC,
		NULL, NULL, NULL);

	av_log_set_callback( log_callback_null );
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


boost::shared_ptr<OutputStream> ffmpegwrapper::_add_output_streams( AVFormatContext *oc, enum AVMediaType type, int source_index )
{
	char *bsf = NULL, *next, *codec_tag = NULL;
	AVBitStreamFilterContext *bsfc, *bsfc_prev = NULL;
	double qscale = -1;
	char *buf = NULL, *arg = NULL, *preset = NULL;
	AVIOContext *s = NULL;

	AVStream *st = avformat_new_stream( oc, NULL );
	int idx = oc->nb_streams - 1, ret = 0;

	if (!st)
	{
		return boost::shared_ptr<OutputStream>();
	}

	//if (oc->nb_streams - 1 < o->nb_streamid_map)
	//	st->id = o->streamid_map[oc->nb_streams - 1];

	boost::shared_ptr<OutputStream> ost( new OutputStream );
	memset( ost.get(), 0, sizeof(OutputStream) );

	ost->file_index = output_file_list_.size();
	ost->index      = idx;
	ost->st         = st;
	st->codec->codec_type = type;

	choose_encoder( outputCodecName_.c_str(), oc, ost );

	if (ost->enc) 
	{
		ost->opts  = filter_codec_opts( codec_opts_, ost->enc->id, oc, st, ost->enc);
	}

	avcodec_get_context_defaults3( st->codec, ost->enc );   
	st->codec->codec_type = type; // XXX hack, avcodec_get_context_defaults2() sets type to unknown for stream copy
	
	// TODO:preset
	// ...

	ost->max_frames = INT64_MAX;
	ost->copy_prior_start = -1;
	// TODO:bitstream_filters
	// TODO:codec_tags

	if( quality_ >= 0 )
	{
		st->codec->flags |= CODEC_FLAG_QSCALE;
		st->codec->global_quality = FF_QP2LAMBDA * quality_;
	}       

	if( oc->oformat->flags & AVFMT_GLOBALHEADER )
		st->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

    av_opt_get_int(sws_opts_, "sws_flags", 0, &ost->sws_flags);
    av_opt_get_int(swr_opts_, "dither_method", 0, &ost->swr_dither_method);
    av_opt_get_double(swr_opts_, "dither_scale" , 0, &ost->swr_dither_scale);

	ost->source_index = source_index;

	if( source_index >= 0 && input_stream_list_.size() > source_index) 
	{
		ost->sync_ist = input_stream_list_[source_index];
		input_stream_list_[source_index]->discard = 0;
		input_stream_list_[source_index]->st->discard = AVDISCARD_NONE;
	}  

	output_stream_list_.push_back( ost );

	return ost;
}


boost::shared_ptr<OutputStream> ffmpegwrapper::_add_video_streams( AVFormatContext *oc, int source_index )
{
	AVStream *st;
	AVCodecContext *video_enc;
	char *frame_rate = NULL;

	boost::shared_ptr<OutputStream> ost = _add_output_streams( oc, AVMEDIA_TYPE_VIDEO, source_index );

	st  = ost->st;                                                                                                                                           
	video_enc = st->codec;

	if( av_parse_video_rate( &ost->frame_rate, output_frame_rate_.c_str() ) < 0 ) 
	{
		output_stream_list_.clear();
		return boost::shared_ptr<OutputStream>();
	}

	if( !ost->stream_copy ) 
	{
		const char *p = NULL;                                                                                                                                
		char *frame_size = NULL;
		char *frame_aspect_ratio = NULL, *frame_pix_fmt = NULL;
		char *intra_matrix = NULL, *inter_matrix = NULL;
		const char *filters = "null";
		int do_pass = 0;
		int i;

		if( !frame_size_.empty() && av_parse_video_size(&video_enc->width, &video_enc->height, frame_size_.c_str() ) < 0) 
		{
			output_stream_list_.clear();
			return boost::shared_ptr<OutputStream>();
		} 

		// TODO:frame_aspect_ratio
		// TODO:frame_bits_per_raw_sample
		video_enc->bits_per_raw_sample = 0;
		// TODO:frame_pix_fmts

		st->sample_aspect_ratio = video_enc->sample_aspect_ratio;
		// TODO:intra_matrices
		// TODO:inter_matrices
		// TODO:rc_overrides
		video_enc->rc_override_count = 0;
		if (!video_enc->rc_initial_buffer_occupancy)
			video_enc->rc_initial_buffer_occupancy = video_enc->rc_buffer_size * 3 / 4;
		// TODO:intra_dc_precision
		video_enc->intra_dc_precision = 0;
		// TODO:do_psnr
		// TODO:do_pass, two pass mode
		// TODO:forced_key_frames
		// TODO:force_fps
		ost->top_field_first = -1;
		// TODO:top_field_first
		// TODO:filter
		ost->avfilter = av_strdup(filters);
	}
	else
	{
		// TODO:copy_initial_nonkeyframes
	}
	return ost;
}

int ffmpegwrapper::_init_input_stream( boost::shared_ptr<InputStream> ist, char *error, int error_len )
{
    if (ist->decoding_needed) 
	{
        AVCodec *codec = ist->dec;
        if (!codec)
		{
            snprintf(error, error_len, "Decoder (codec %s) not found for input stream #%d:%d",
                    avcodec_get_name(ist->st->codec->codec_id), ist->file_index, ist->st->index);
            return AVERROR(EINVAL);
        }
 
        ist->dr1 = (codec->capabilities & CODEC_CAP_DR1) && !do_deinterlace_;
        if (codec->type == AVMEDIA_TYPE_VIDEO && ist->dr1) 
		{
            ist->st->codec->get_buffer     = codec_get_buffer;
            ist->st->codec->release_buffer = codec_release_buffer;
            ist->st->codec->opaque         = &ist->buffer_pool;
        }
 
        if (!av_dict_get(ist->opts, "threads", NULL, 0))
            av_dict_set(&ist->opts, "threads", "auto", 0);
        if (avcodec_open2(ist->st->codec, codec, &ist->opts) < 0) 
		{
            snprintf(error, error_len, "Error while opening decoder for input stream #%d:%d",
                    ist->file_index, ist->st->index);
            return AVERROR(EINVAL);
        }
        // TODO:assert_codec_experimental(ist->st->codec, 0);
        // TODO:assert_avoptions(ist->opts);
    }
 
    ist->next_pts = AV_NOPTS_VALUE;
    ist->next_dts = AV_NOPTS_VALUE;
    ist->is_start = 1;
 
    return 0;
}


int ffmpegwrapper::_transcode_init( void )
{
	int ret = 0;
	size_t i, j, k;
	AVFormatContext *oc;
	AVCodecContext *codec;
	boost::shared_ptr<OutputStream> ost;
	boost::shared_ptr<InputStream> ist;
	char error[1024];
	int want_sdp = 1;


	// init framerate emulation
	for( i = 0; i < input_file_list_.size(); i++ ) 
	{
		boost::shared_ptr<InputFile> ifile = input_file_list_[i];
		if (ifile->rate_emu)
			for (j = 0; j < ifile->nb_streams; j++)
				input_stream_list_[ j + ifile->ist_index ]->start = av_gettime();
	}    

	// output stream init
	for( i = 0; i < output_file_list_.size(); i++ ) 
	{
		oc = output_file_list_[i]->ctx;
		if( !oc->nb_streams && !(oc->oformat->flags & AVFMT_NOSTREAMS))
		{
			av_dump_format(oc, i, oc->filename, 1);
			av_log(NULL, AV_LOG_ERROR, "Output file #%d does not contain any stream\n", i);
			return AVERROR(EINVAL);
		}  
	}      

	// init complex filtergraphs
	//for (i = 0; i < nb_filtergraphs; i++)
	//    if ((ret = avfilter_graph_config(filtergraphs[i]->graph, NULL)) < 0)
	//        return ret;

	// for each output stream, we compute the right encoding parameters
	for (i = 0; i < output_stream_list_.size(); i++) 
	{
		AVCodecContext *icodec = NULL;
		ost = output_stream_list_[i];
		oc  = output_file_list_[ost->file_index]->ctx;
		ist = _get_input_stream(ost);

		if (ost->attachment_filename)
			continue;

		codec  = ost->st->codec;

		if (ist) 
		{
			icodec = ist->st->codec;

			ost->st->disposition          = ist->st->disposition;
			codec->bits_per_raw_sample    = icodec->bits_per_raw_sample;
			codec->chroma_sample_location = icodec->chroma_sample_location;
		}

		if (ost->stream_copy)
		{
			// TODO:ost:_transcode_init, stream_copy logic
		}
		else
		{
			if (!ost->enc)
                ost->enc = avcodec_find_encoder(codec->codec_id);

			if (!ost->enc) {
                /* should only happen when a default codec is not present. */
                snprintf(error, sizeof(error), "Encoder (codec %s) not found for output stream #%d:%d",
                         avcodec_get_name(ost->st->codec->codec_id), ost->file_index, ost->index);
				ret = AVERROR(EINVAL);
				goto dump_format;
			}

			if (ist)
				ist->decoding_needed++;
			ost->encoding_needed = 1;

			if (!ost->filter &&        
				(codec->codec_type == AVMEDIA_TYPE_VIDEO ||
				codec->codec_type == AVMEDIA_TYPE_AUDIO)) 
			{
				boost::shared_ptr<FilterGraph> fg;
				fg = _init_simple_filtergraph(ist, ost);
				if( _configure_filtergraph( fg ) ) 
				{                                                                                                         
					av_log(NULL, AV_LOG_FATAL, "Error opening filters!\n");
					continue;	// exit(1);
				}
			}

			if (codec->codec_type == AVMEDIA_TYPE_VIDEO) 
			{
                if (ost->filter && !ost->frame_rate.num)
                    ost->frame_rate = av_buffersink_get_frame_rate(ost->filter->filter);
                if (ist && !ost->frame_rate.num)
                    ost->frame_rate = ist->framerate;
                if (ist && !ost->frame_rate.num)
				{
					AVRational temp = {25, 1};
                    ost->frame_rate = ist->st->r_frame_rate.num ? ist->st->r_frame_rate : temp;
				}
                if (ost->enc && ost->enc->supported_framerates && !ost->force_fps) {
                    int idx = av_find_nearest_q_idx(ost->frame_rate, ost->enc->supported_framerates);
                    ost->frame_rate = ost->enc->supported_framerates[idx];
                }          
            } 

			switch (codec->codec_type) 
			{
			case AVMEDIA_TYPE_AUDIO:
				// TODO: audio check
				break;
			case AVMEDIA_TYPE_VIDEO:
				{
					codec->time_base = av_inv_q(ost->frame_rate);
					if( ost->filter && !(codec->time_base.num && codec->time_base.den))
						codec->time_base = ost->filter->filter->inputs[0]->time_base;

					if( av_q2d(codec->time_base) < 0.001 && video_sync_method_ != VSYNC_PASSTHROUGH
						&& (video_sync_method_ == VSYNC_CFR || (video_sync_method_ == VSYNC_AUTO && !(oc->oformat->flags & AVFMT_VARIABLE_FPS))))
					{
						av_log(oc, AV_LOG_WARNING, "Frame rate very high for a muxer not efficiently supporting it.\n"
							"Please consider specifying a lower framerate, a different muxer or -vsync 2\n");
					}     
					for (j = 0; j < ost->forced_kf_count; j++)
					{
						AVRational base = {1, AV_TIME_BASE};
						ost->forced_kf_pts[j] = av_rescale_q( ost->forced_kf_pts[j], base, codec->time_base );
					}

					codec->width  = ost->filter->filter->inputs[0]->w;
					codec->height = ost->filter->filter->inputs[0]->h;
					codec->sample_aspect_ratio = ost->st->sample_aspect_ratio =
						ost->frame_aspect_ratio ? // overridden by the -aspect cli option
						av_d2q(ost->frame_aspect_ratio * codec->height/codec->width, 255) :
					ost->filter->filter->inputs[0]->sample_aspect_ratio;
					codec->pix_fmt = (PixelFormat)ost->filter->filter->inputs[0]->format;

					if (!icodec ||
						codec->width   != icodec->width  ||
						codec->height  != icodec->height ||
						codec->pix_fmt != icodec->pix_fmt) 
					{
						codec->bits_per_raw_sample = 0;	// TODO:frame_bits_per_raw_sample;                                                                                  
					}

					// TODO:forced_keyframes
				}
				break;
			case AVMEDIA_TYPE_SUBTITLE:
				// TODO: subtitle check
				break;
			}

			// TODO: two pass mode
		}
	}

	/* open each encoder */
	for( i = 0; i < output_stream_list_.size(); i++ ) 
	{
		ost = output_stream_list_[i];

		if( ost->encoding_needed )
		{
			AVCodec      *codec = ost->enc;
			AVCodecContext *dec = NULL;

			if( (ist = _get_input_stream(ost)) )
				dec = ist->st->codec;

			// TODO:subtitle_header
			if( !av_dict_get(ost->opts, "threads", NULL, 0))
				av_dict_set(&ost->opts, "threads", "auto", 0);
			if( avcodec_open2(ost->st->codec, codec, &ost->opts) < 0) 
			{
				snprintf(error, sizeof(error), "Error while opening encoder for output stream #%d:%d - maybe incorrect parameters such as bit_rate, rate, width or height",
					ost->file_index, ost->index);
				ret = AVERROR(EINVAL);
				goto dump_format;
			}
			if( ost->enc->type == AVMEDIA_TYPE_AUDIO &&
				!(ost->enc->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE) )
				av_buffersink_set_frame_size( ost->filter->filter, ost->st->codec->frame_size );
			
			// TODO:assert_codec_experimental(ost->st->codec, 1);
			// TODO:assert_avoptions(ost->opts);

			if( ost->st->codec->bit_rate && ost->st->codec->bit_rate < 1000 )
				av_log(NULL, AV_LOG_WARNING, "The bitrate parameter is set too low."
											 " It takes bits/s as argument, not kbits/s\n");
			extra_size_ += ost->st->codec->extradata_size;

			if (ost->st->codec->me_threshold)
				input_stream_list_[ost->source_index]->st->codec->debug |= FF_DEBUG_MV;
		}
	}

	// init input streams
	for( i = 0; i < input_stream_list_.size(); i++ )
		if( (ret = _init_input_stream( input_stream_list_[i], error, sizeof(error) )) < 0 )
			goto dump_format;

	// discard unused programs
	for (i = 0; i < input_file_list_.size(); i++) 
	{
		boost::shared_ptr<InputFile> ifile = input_file_list_[i];
		for (j = 0; j < ifile->ctx->nb_programs; j++) 
		{
			AVProgram *p = ifile->ctx->programs[j];
			int discard  = AVDISCARD_ALL;

			for (k = 0; k < p->nb_stream_indexes; k++)
			{
				if (!input_stream_list_[ifile->ist_index + p->stream_index[k]]->discard) 
				{
					discard = AVDISCARD_DEFAULT;
					break;
				}  
			}
			p->discard = (AVDiscard)discard;
		}          
	}

	// open files and write file headers
	for( i = 0; i < output_file_list_.size(); i++ ) 
	{
		oc = output_file_list_[i]->ctx;
		oc->interrupt_callback = int_cb;
		if ((ret = avformat_write_header(oc, &output_file_list_[i]->opts)) < 0) 
		{
			char errbuf[128];
			const char *errbuf_ptr = errbuf;
			if (av_strerror(ret, errbuf, sizeof(errbuf)) < 0)
				errbuf_ptr = strerror(AVUNERROR(ret));
			snprintf(error, sizeof(error), "Could not write header for output file #%d (incorrect codec parameters ?): %s", i, errbuf_ptr);
			ret = AVERROR(EINVAL);
			goto dump_format;
		}         

		if (strcmp(oc->oformat->name, "rtp")) 
		{
			want_sdp = 0;
		}         
	}             


dump_format:

#ifdef __PRINT_DUMP__
	for (i = 0; i < output_file_list_.size(); i++)
	{
		av_dump_format(output_file_list_[i]->ctx, i, output_file_list_[i]->ctx->filename, 1);
	}     

	// TODO:dump the stream mapping 
	// ..

	for( i = 0; i < output_stream_list_.size(); i++ ) 
	{
		ost = output_stream_list_[i];

		if (ost->attachment_filename) 
		{
			/* an attached file */
			av_log(NULL, AV_LOG_INFO, "  File %s -> Stream #%d:%d\n",
				ost->attachment_filename, ost->file_index, ost->index);
			continue;
		}        

		// TODO:filter graph dump
		//if (ost->filter && ost->filter->graph->graph_desc) 
		//{
		//	/* output from a complex graph */
		//	av_log(NULL, AV_LOG_INFO, "  %s", ost->filter->name);
		//	if (nb_filtergraphs > 1)
		//		av_log(NULL, AV_LOG_INFO, " (graph %d)", ost->filter->graph->index);

		//	av_log(NULL, AV_LOG_INFO, " -> Stream #%d:%d (%s)\n", ost->file_index,
		//		ost->index, ost->enc ? ost->enc->name : "?");
		//	continue;
		//}        

		av_log(NULL, AV_LOG_INFO, "  Stream #%d:%d -> #%d:%d",
			input_stream_list_[ost->source_index]->file_index,
			input_stream_list_[ost->source_index]->st->index,
			ost->file_index,
			ost->index);

		if (ost->sync_ist != input_stream_list_[ost->source_index])
			av_log(NULL, AV_LOG_INFO, " [sync #%d:%d]",
			ost->sync_ist->file_index,
			ost->sync_ist->st->index);

		if (ost->stream_copy)
			av_log(NULL, AV_LOG_INFO, " (copy)");
		else     
			av_log(NULL, AV_LOG_INFO, " (%s -> %s)", input_stream_list_[ost->source_index]->dec ?
			input_stream_list_[ost->source_index]->dec->name : "?",
			ost->enc ? ost->enc->name : "?");
		av_log(NULL, AV_LOG_INFO, "\n");
	}
#endif

	return ret;
}

boost::shared_ptr<OutputStream> ffmpegwrapper::_choose_output( void )
{
    int i;
    int64_t opts_min = INT64_MAX;
    boost::shared_ptr<OutputStream> ost_min;

    for (i = 0; i < output_stream_list_.size(); i++) 
	{
		boost::shared_ptr<OutputStream> ost = output_stream_list_[i];
        int64_t opts = av_rescale_q(ost->st->cur_dts, ost->st->time_base,
                                    _AV_TIME_BASE_Q);
        if (!ost->unavailable && !ost->finished && opts < opts_min)
		{
            opts_min = opts;
            ost_min  = ost;
        }
    }
    return ost_min;
}

bool ffmpegwrapper::_got_eagain(void)
{
    int i;
    for (i = 0; i < output_stream_list_.size(); i++)
        if (output_stream_list_[i]->unavailable)
            return true;
    return false;
}


void ffmpegwrapper::_reset_eagain(void)
{
    size_t i;
    for (i = 0; i < input_file_list_.size(); i++)
        input_file_list_[i]->eagain = 0;
    for (i = 0; i < output_stream_list_.size(); i++)
        output_stream_list_[i]->unavailable = 0;
}

InputStream * ffmpegwrapper::_transcode_from_filter(FilterGraph *graph, int *resultCode)
{
    int i, ret;
    int nb_requests, nb_requests_max = 0;
    InputFilter *ifilter;
	InputStream* best_ist = NULL;

    ret = avfilter_graph_request_oldest(graph->graph);
    if (ret >= 0)
	{
        ret = _reap_filters();
		if(resultCode) *resultCode = ret;
		return NULL;
	}

	if (ret == AVERROR_EOF) 
	{
		ret = _reap_filters();
		for (i = 0; i < graph->nb_outputs; i++)
			_close_output_stream(graph->outputs[i]->ost);
		if(resultCode) *resultCode = ret;
		return NULL;
	}
	if (ret != AVERROR(EAGAIN)) 
	{
		if(resultCode) *resultCode = ret;
		return NULL;
	}

    for (i = 0; i < graph->nb_inputs; i++) 
	{
        ifilter = graph->inputs[i];
        InputStream* ist = ifilter->ist;
        if (input_file_list_[ist->file_index]->eagain ||
            input_file_list_[ist->file_index]->eof_reached)
            continue;
        nb_requests = av_buffersrc_get_nb_failed_requests(ifilter->filter);
        if (nb_requests > nb_requests_max) 
		{
            nb_requests_max = nb_requests;
            best_ist = ist;
        }
    }

    if (!best_ist)
        for (i = 0; i < graph->nb_outputs; i++)
            graph->outputs[i]->ost->unavailable = 1;

	if(resultCode) *resultCode = 0;
    return best_ist;
}


int ffmpegwrapper::_transcode_step( void )
{
    boost::shared_ptr<OutputStream> ost;
    InputStream *ist;

    int ret = 0;

    ost = _choose_output();
    if (!ost) 
	{
       if (_got_eagain()) 
		{
            _reset_eagain();
            av_usleep(10000);
            return 0;
        }
        av_log(NULL, AV_LOG_VERBOSE, "No more inputs to read from, finishing.\n");
        return AVERROR_EOF;
    }

    if (ost->filter) 
	{
		ist = _transcode_from_filter(ost->filter->graph, &ret);
		if( ret < 0 )
			return ret;
        if( !ist )
            return 0;
    } 
	else 
	{
        av_assert0(ost->source_index >= 0);
        ist = input_stream_list_[ost->source_index].get();
    }

    ret = _process_input(ist->file_index);
    if (ret == AVERROR(EAGAIN)) 
	{
        if (input_file_list_[ist->file_index]->eagain)
            ost->unavailable = 1;
        return 0;
    }
	
	if (ret < 0)
		return ret == AVERROR_EOF ? 0 : ret;

	return _reap_filters();
}

int ffmpegwrapper::_get_input_packet(boost::shared_ptr<InputFile> f, AVPacket *pkt)
{
#if HAVE_PTHREADS
    if (nb_input_files > 1)
        return get_input_packet_mt(f, pkt);
#endif
    return av_read_frame(f->ctx, pkt);
}

int ffmpegwrapper::_process_input(int file_index)
{
	boost::shared_ptr<InputFile> ifile = input_file_list_[file_index];
	AVFormatContext *is;
	boost::shared_ptr<InputStream> ist;
	AVPacket pkt;
	int ret, i, j;

	is  = ifile->ctx;
	ret = _get_input_packet(ifile, &pkt);

	if (ret == AVERROR(EAGAIN)) 
	{
		ifile->eagain = 1;
		return ret;
	}
	if (ret < 0) 
	{
		if (ret != AVERROR_EOF) 
		{
			return ret;
			//print_error(is->filename, ret);
			//if (exit_on_error)
			//    exit(1);
		}
		ifile->eof_reached = 1;

		for (i = 0; i < ifile->nb_streams; i++) 
		{
			ist = input_stream_list_[ifile->ist_index + i];
			if (ist->decoding_needed)
				_output_packet(ist, NULL);

			/* mark all outputs that don't go through lavfi as finished */
			for (j = 0; j < output_stream_list_.size(); j++)
			{
				boost::shared_ptr<OutputStream> ost = output_stream_list_[j];

				if (ost->source_index == ifile->ist_index + i &&
					(ost->stream_copy || ost->enc->type == AVMEDIA_TYPE_SUBTITLE))
					_close_output_stream(ost);
			}
		}

		return AVERROR(EAGAIN);
	}

	_reset_eagain();

	// TODO:dump do_pkt_dump
	//if (do_pkt_dump) 
	//{
	//	av_pkt_dump_log2(NULL, AV_LOG_DEBUG, &pkt, do_hex_dump,
	//		is->streams[pkt.stream_index]);
	//}

	// the following test is needed in case new streams appear
	// dynamically in stream : we ignore them
	if (pkt.stream_index >= ifile->nb_streams) {
		_report_new_stream(file_index, &pkt);
		goto discard_packet;
	}

	ist = input_stream_list_[ifile->ist_index + pkt.stream_index];
	if (ist->discard)
		goto discard_packet;

	if(!ist->wrap_correction_done && input_file_list_[file_index]->ctx->start_time != AV_NOPTS_VALUE && ist->st->pts_wrap_bits < 64)
	{
		int64_t stime = av_rescale_q(input_file_list_[file_index]->ctx->start_time, _AV_TIME_BASE_Q, ist->st->time_base);
		int64_t stime2= stime + (1ULL<<ist->st->pts_wrap_bits);
		ist->wrap_correction_done = 1;

		if(stime2 > stime && pkt.dts != AV_NOPTS_VALUE && pkt.dts > stime + (1LL<<(ist->st->pts_wrap_bits-1))) 
		{
			pkt.dts -= 1ULL<<ist->st->pts_wrap_bits;
			ist->wrap_correction_done = 0;
		}
		if(stime2 > stime && pkt.pts != AV_NOPTS_VALUE && pkt.pts > stime + (1LL<<(ist->st->pts_wrap_bits-1))) 
		{
			pkt.pts -= 1ULL<<ist->st->pts_wrap_bits;
			ist->wrap_correction_done = 0;
		}
	}

	if (pkt.dts != AV_NOPTS_VALUE)
		pkt.dts += av_rescale_q(ifile->ts_offset, _AV_TIME_BASE_Q, ist->st->time_base);
	if (pkt.pts != AV_NOPTS_VALUE)
		pkt.pts += av_rescale_q(ifile->ts_offset, _AV_TIME_BASE_Q, ist->st->time_base);

	if (pkt.pts != AV_NOPTS_VALUE)
		pkt.pts *= ist->ts_scale;
	if (pkt.dts != AV_NOPTS_VALUE)
		pkt.dts *= ist->ts_scale;

	//if (debug_ts) {
	//	av_log(NULL, AV_LOG_INFO, "demuxer -> ist_index:%d type:%s "
	//		"next_dts:%s next_dts_time:%s next_pts:%s next_pts_time:%s  pkt_pts:%s pkt_pts_time:%s pkt_dts:%s pkt_dts_time:%s off:%"PRId64"\n",
	//		ifile->ist_index + pkt.stream_index, av_get_media_type_string(ist->st->codec->codec_type),
	//		_av_ts2str(ist->next_dts), _av_ts2timestr(ist->next_dts, &_AV_TIME_BASE_Q),
	//		_av_ts2str(ist->next_pts), _av_ts2timestr(ist->next_pts, &_AV_TIME_BASE_Q),
	//		_av_ts2str(pkt.pts), _av_ts2timestr(pkt.pts, &ist->st->time_base),
	//		_av_ts2str(pkt.dts), _av_ts2timestr(pkt.dts, &ist->st->time_base),
	//		input_files[ist->file_index]->ts_offset);
	//}

	if (pkt.dts != AV_NOPTS_VALUE && ist->next_dts != AV_NOPTS_VALUE && !copy_ts_) 
	{
		int64_t pkt_dts = av_rescale_q(pkt.dts, ist->st->time_base, _AV_TIME_BASE_Q);
		int64_t delta   = pkt_dts - ist->next_dts;
		if (is->iformat->flags & AVFMT_TS_DISCONT)
		{
			if(delta < -1LL*dts_delta_threshold_*AV_TIME_BASE ||
				(delta > 1LL*dts_delta_threshold_*AV_TIME_BASE &&
				ist->st->codec->codec_type != AVMEDIA_TYPE_SUBTITLE) ||
				pkt_dts+1<ist->pts)
			{
				ifile->ts_offset -= delta;
				av_log(NULL, AV_LOG_DEBUG,
					"timestamp discontinuity %"PRId64", new offset= %"PRId64"\n",
					delta, ifile->ts_offset);
				pkt.dts -= av_rescale_q(delta, _AV_TIME_BASE_Q, ist->st->time_base);
				if (pkt.pts != AV_NOPTS_VALUE)
					pkt.pts -= av_rescale_q(delta, _AV_TIME_BASE_Q, ist->st->time_base);
			}
		} 
		else 
		{
			if ( delta < -1LL*dts_error_threshold_*AV_TIME_BASE ||
				(delta > 1LL*dts_error_threshold_*AV_TIME_BASE && ist->st->codec->codec_type != AVMEDIA_TYPE_SUBTITLE)
				) 
			{
				av_log(NULL, AV_LOG_WARNING, "DTS %"PRId64", next:%"PRId64" st:%d invalid dropping\n", pkt.dts, ist->next_dts, pkt.stream_index);
				pkt.dts = AV_NOPTS_VALUE;
			}
			if (pkt.pts != AV_NOPTS_VALUE)
			{
				int64_t pkt_pts = av_rescale_q(pkt.pts, ist->st->time_base, _AV_TIME_BASE_Q);
				delta   = pkt_pts - ist->next_dts;
				if ( delta < -1LL*dts_error_threshold_*AV_TIME_BASE ||
					(delta > 1LL*dts_error_threshold_*AV_TIME_BASE && ist->st->codec->codec_type != AVMEDIA_TYPE_SUBTITLE)
					)
				{
					av_log(NULL, AV_LOG_WARNING, "PTS %"PRId64", next:%"PRId64" invalid dropping st:%d\n", pkt.pts, ist->next_dts, pkt.stream_index);
					pkt.pts = AV_NOPTS_VALUE;
				}
			}
		}
	}

	_sub2video_heartbeat(ist, pkt.pts);

	ret = _output_packet(ist, &pkt);
	if (ret < 0) 
	{
		char buf[128];
		av_strerror(ret, buf, sizeof(buf));
		av_log(NULL, AV_LOG_ERROR, "Error while decoding stream #%d:%d: %s\n",
			ist->file_index, ist->st->index, buf);

		return ret;
	}

discard_packet:
	av_free_packet(&pkt);

	return 0;
}

void ffmpegwrapper::_sub2video_heartbeat(boost::shared_ptr<InputStream> ist, int64_t pts)
{
	boost::shared_ptr<InputFile> infile = input_file_list_[ist->file_index];
    int i, j, nb_reqs;
    int64_t pts2;

    /* When a frame is read from a file, examine all sub2video streams in
       the same file and send the sub2video frame again. Otherwise, decoded
       video frames could be accumulating in the filter graph while a filter
       (possibly overlay) is desperately waiting for a subtitle frame. */
    for (i = 0; i < infile->nb_streams; i++)
	{
        boost::shared_ptr<InputStream> ist2 = input_stream_list_[infile->ist_index + i];
        if (!ist2->sub2video.ref)
            continue;
        /* subtitles seem to be usually muxed ahead of other streams;
           if not, substracting a larger time here is necessary */
        pts2 = av_rescale_q(pts, ist->st->time_base, ist2->st->time_base) - 1;
        /* do not send the heartbeat frame if the subtitle is already ahead */
        if (pts2 <= ist2->sub2video.last_pts)
            continue;
        for (j = 0, nb_reqs = 0; j < ist2->nb_filters; j++)
            nb_reqs += av_buffersrc_get_nb_failed_requests(ist2->filters[j]->filter);
        if (nb_reqs)
            sub2video_push_ref(ist2.get(), pts2);
    }
}

void ffmpegwrapper::_report_new_stream(int input_index, AVPacket *pkt)
{
	boost::shared_ptr<InputFile> file = input_file_list_[input_index];
    AVStream *st = file->ctx->streams[pkt->stream_index];

    if (pkt->stream_index < file->nb_streams_warn)
        return;

    //av_log(file->ctx, AV_LOG_WARNING,
    //       "New %s stream %d:%d at pos:%"PRId64" and DTS:%ss\n",
    //       av_get_media_type_string(st->codec->codec_type),
    //       input_index, pkt->stream_index,
    //       pkt->pos, _av_ts2timestr(pkt->dts, &st->time_base));
    file->nb_streams_warn = pkt->stream_index + 1;
}

int ffmpegwrapper::_reap_filters(void)
{
	AVFilterBufferRef *picref;
	AVFrame *filtered_frame = NULL;
	size_t i;
	int64_t frame_pts;

	/* Reap all buffers present in the buffer sinks */
	for (i = 0; i < output_stream_list_.size(); i++) 
	{
		boost::shared_ptr<OutputStream> ost = output_stream_list_[i];
		boost::shared_ptr<OutputFile> of = output_file_list_[ost->file_index];
		int ret = 0;

		if (!ost->filter)
			continue;

		if (!ost->filtered_frame && !(ost->filtered_frame = avcodec_alloc_frame())) 
		{
			return AVERROR(ENOMEM);
		} 
		else
			avcodec_get_frame_defaults(ost->filtered_frame);

		filtered_frame = ost->filtered_frame;

		while (1) 
		{
			ret = av_buffersink_get_buffer_ref(ost->filter->filter, &picref,
				AV_BUFFERSINK_FLAG_NO_REQUEST);
			if (ret < 0) 
			{
				if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) 
				{
					char buf[256];
					av_strerror(ret, buf, sizeof(buf));
					av_log(NULL, AV_LOG_WARNING,
						"Error in av_buffersink_get_buffer_ref(): %s\n", buf);
				}
				break;
			}
			frame_pts = AV_NOPTS_VALUE;
			if (picref->pts != AV_NOPTS_VALUE) 
			{
				filtered_frame->pts = frame_pts = av_rescale_q(picref->pts,
					ost->filter->filter->inputs[0]->time_base,
					ost->st->codec->time_base) -
					av_rescale_q(of->start_time,
					_AV_TIME_BASE_Q,
					ost->st->codec->time_base);

				if (of->start_time && filtered_frame->pts < 0) 
				{
					avfilter_unref_buffer(picref);
					continue;
				}
			}
			//if (ost->source_index >= 0)
			//    *filtered_frame= *input_streams[ost->source_index]->decoded_frame; //for me_threshold


			switch (ost->filter->filter->inputs[0]->type)
			{
			case AVMEDIA_TYPE_VIDEO:
				avfilter_copy_buf_props(filtered_frame, picref);
				filtered_frame->pts = frame_pts;
				if (!ost->frame_aspect_ratio)
					ost->st->codec->sample_aspect_ratio = picref->video->sample_aspect_ratio;

				_do_video_out(of->ctx, ost, filtered_frame,
					same_quant_ ? ost->last_quality :
					ost->st->codec->global_quality);
				break;
			case AVMEDIA_TYPE_AUDIO:
				// TODO:do_audio_out
				/*avfilter_copy_buf_props(filtered_frame, picref);
				filtered_frame->pts = frame_pts;
				do_audio_out(of->ctx, ost, filtered_frame);*/
				break;
			default:
				// TODO support subtitle filters
				av_assert0(0);
			}

			avfilter_unref_buffer(picref);
		}
	}

	return 0;
}


int ffmpegwrapper::_do_video_out(AVFormatContext *s, boost::shared_ptr<OutputStream> ost, AVFrame *in_picture, float quality)
{
	int ret, format_video_sync;
	AVPacket pkt;
	AVCodecContext *enc = ost->st->codec;
	int nb_frames, i;
	double sync_ipts, delta;
	double duration = 0;
	int frame_size = 0;
	boost::shared_ptr<InputStream> ist;

	if (ost->source_index >= 0)
		ist = input_stream_list_[ost->source_index];

	if(ist && ist->st->start_time != AV_NOPTS_VALUE && ist->st->first_dts != AV_NOPTS_VALUE && ost->frame_rate.num)
		duration = 1/(av_q2d(ost->frame_rate) * av_q2d(enc->time_base));

	sync_ipts = in_picture->pts;
	delta = sync_ipts - ost->sync_opts + duration;

	/* by default, we output a single frame */
	nb_frames = 1;

	format_video_sync = video_sync_method_;
	if (format_video_sync == VSYNC_AUTO)
		format_video_sync = (s->oformat->flags & AVFMT_VARIABLE_FPS) ? ((s->oformat->flags & AVFMT_NOTIMESTAMPS) ? VSYNC_PASSTHROUGH : VSYNC_VFR) : 1;

	switch (format_video_sync)
	{
	case VSYNC_CFR:
		// FIXME set to 0.5 after we fix some dts/pts bugs like in avidec.c
		if (delta < -1.1)
			nb_frames = 0;
		else if (delta > 1.1)
			nb_frames = lrintf(delta);
		break;
	case VSYNC_VFR:
		if (delta <= -0.6)
			nb_frames = 0;
		else if (delta > 0.6)
			ost->sync_opts = lrint(sync_ipts);
		break;
	case VSYNC_DROP:
	case VSYNC_PASSTHROUGH:
		ost->sync_opts = lrint(sync_ipts);
		break;
	default:
		av_assert0(0);
	}

	nb_frames = FFMIN(nb_frames, ost->max_frames - ost->frame_number);
	if (nb_frames == 0) 
	{
		nb_frames_drop_++;
		av_log(NULL, AV_LOG_VERBOSE, "*** drop!\n");
		return 1;
	} 
	else if (nb_frames > 1) 
	{
		if (nb_frames > dts_error_threshold_ * 30) 
		{
			av_log(NULL, AV_LOG_ERROR, "%d frame duplication too large, skipping\n", nb_frames - 1);
			nb_frames_drop_++;
			return 1;
		}
		nb_frames_dup_ += nb_frames - 1;
		av_log(NULL, AV_LOG_VERBOSE, "*** %d dup!\n", nb_frames - 1);
	}

	/* duplicates frame if needed */
	for (i = 0; i < nb_frames; i++) 
	{
		av_init_packet(&pkt);
		pkt.data = NULL;
		pkt.size = 0;

		in_picture->pts = ost->sync_opts;

		if (!_check_recording_time(ost))
			return 2;

		if (s->oformat->flags & AVFMT_RAWPICTURE &&
			enc->codec->id == AV_CODEC_ID_RAWVIDEO) 
		{
			/* raw pictures are written as AVPicture structure to
			avoid any copies. We support temporarily the older
			method. */
			enc->coded_frame->interlaced_frame = in_picture->interlaced_frame;
			enc->coded_frame->top_field_first  = in_picture->top_field_first;
			pkt.data   = (uint8_t *)in_picture;
			pkt.size   =  sizeof(AVPicture);
			pkt.pts    = av_rescale_q(in_picture->pts, enc->time_base, ost->st->time_base);
			pkt.flags |= AV_PKT_FLAG_KEY;

			video_size_ += pkt.size;
			write_frame(s, &pkt, ost.get(), video_sync_method_, audio_sync_method_);
		} 
		else 
		{
			int got_packet;
			AVFrame big_picture;

			big_picture = *in_picture;
			/* better than nothing: use input picture interlaced
			settings */
			big_picture.interlaced_frame = in_picture->interlaced_frame;
			if (ost->st->codec->flags & (CODEC_FLAG_INTERLACED_DCT|CODEC_FLAG_INTERLACED_ME)) 
			{
				if (ost->top_field_first == -1)
					big_picture.top_field_first = in_picture->top_field_first;
				else
					big_picture.top_field_first = !!ost->top_field_first;
			}

			/* handles same_quant here. This is not correct because it may
			not be a global option */
			big_picture.quality = quality;
			if (!enc->me_threshold)
				big_picture.pict_type = (AVPictureType)0;
			if (ost->forced_kf_index < ost->forced_kf_count &&
				big_picture.pts >= ost->forced_kf_pts[ost->forced_kf_index]) 
			{
				big_picture.pict_type = AV_PICTURE_TYPE_I;
				ost->forced_kf_index++;
			}
			update_benchmark(NULL);
			ret = avcodec_encode_video2(enc, &pkt, &big_picture, &got_packet);
			update_benchmark("encode_video %d.%d", ost->file_index, ost->index);
			if (ret < 0) 
			{
				//av_log(NULL, AV_LOG_FATAL, "Video encoding failed\n");
				//exit(1);
				return -1;
			}

			if (got_packet) {
				if (pkt.pts == AV_NOPTS_VALUE && !(enc->codec->capabilities & CODEC_CAP_DELAY))
					pkt.pts = ost->sync_opts;

				if (pkt.pts != AV_NOPTS_VALUE)
					pkt.pts = av_rescale_q(pkt.pts, enc->time_base, ost->st->time_base);
				if (pkt.dts != AV_NOPTS_VALUE)
					pkt.dts = av_rescale_q(pkt.dts, enc->time_base, ost->st->time_base);

				//if (debug_ts) {
				//	av_log(NULL, AV_LOG_INFO, "encoder -> type:video "
				//		"pkt_pts:%s pkt_pts_time:%s pkt_dts:%s pkt_dts_time:%s\n",
				//		_av_ts2str(pkt.pts), _av_ts2timestr(pkt.pts, &ost->st->time_base),
				//		_av_ts2str(pkt.dts), _av_ts2timestr(pkt.dts, &ost->st->time_base));
				//}

				frame_size = pkt.size;
				video_size_ += pkt.size;
				write_frame(s, &pkt, ost.get(), video_sync_method_, audio_sync_method_);
				av_free_packet(&pkt);

				///* if two pass, output log */
				//if (ost->logfile && enc->stats_out) {
				//	fprintf(ost->logfile, "%s", enc->stats_out);
				//}
			}
		}
		ost->sync_opts++;
		/*
		* For video, number of frames in == number of packets out.
		* But there may be reordering, so we can't throw away frames on encoder
		* flush, we need to limit them here, before they go into encoder.
		*/
		ost->frame_number++;
	}

	// TODO:do_video_stats
	//if (vstats_filename && frame_size)
	//	do_video_stats(output_file_list_[ost->file_index]->ctx, ost, frame_size);

	return 0;
}


int ffmpegwrapper::_check_recording_time(boost::shared_ptr<OutputStream> ost)
{
	boost::shared_ptr<OutputFile> of = output_file_list_[ost->file_index];

    if (of->recording_time != INT64_MAX &&
        av_compare_ts(ost->sync_opts - ost->first_pts, ost->st->codec->time_base, of->recording_time,
                      _AV_TIME_BASE_Q) >= 0) 
	{
        _close_output_stream(ost);
        return 0;
    }
    return 1;
}


void ffmpegwrapper::_do_streamcopy(boost::shared_ptr<InputStream> ist, boost::shared_ptr<OutputStream> ost, const AVPacket *pkt)
{
	boost::shared_ptr<OutputFile> of = output_file_list_[ost->file_index];
	int64_t ost_tb_start_time = av_rescale_q(of->start_time, _AV_TIME_BASE_Q, ost->st->time_base);
	AVPicture pict;
	AVPacket opkt;

	av_init_packet(&opkt);

	if ((!ost->frame_number && !(pkt->flags & AV_PKT_FLAG_KEY)) &&
		!ost->copy_initial_nonkeyframes)
		return;

	if (!ost->frame_number && ist->pts < of->start_time &&
		!ost->copy_prior_start)
		return;

	if (of->recording_time != INT64_MAX &&
		ist->pts >= of->recording_time + of->start_time) 
	{
			_close_output_stream(ost);
			return;
	}

	/* force the input stream PTS */
	if (ost->st->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		audio_size_ += pkt->size;
	else if (ost->st->codec->codec_type == AVMEDIA_TYPE_VIDEO) 
	{
		video_size_ += pkt->size;
		ost->sync_opts++;
	} else if (ost->st->codec->codec_type == AVMEDIA_TYPE_SUBTITLE) 
	{
		subtitle_size_ += pkt->size;
	}

	if (pkt->pts != AV_NOPTS_VALUE)
		opkt.pts = av_rescale_q(pkt->pts, ist->st->time_base, ost->st->time_base) - ost_tb_start_time;
	else
		opkt.pts = AV_NOPTS_VALUE;

	if (pkt->dts == AV_NOPTS_VALUE)
		opkt.dts = av_rescale_q(ist->dts, _AV_TIME_BASE_Q, ost->st->time_base);
	else
		opkt.dts = av_rescale_q(pkt->dts, ist->st->time_base, ost->st->time_base);
	opkt.dts -= ost_tb_start_time;

	opkt.duration = av_rescale_q(pkt->duration, ist->st->time_base, ost->st->time_base);
	opkt.flags    = pkt->flags;

	// FIXME remove the following 2 lines they shall be replaced by the bitstream filters
	if (  ost->st->codec->codec_id != AV_CODEC_ID_H264
		&& ost->st->codec->codec_id != AV_CODEC_ID_MPEG1VIDEO
		&& ost->st->codec->codec_id != AV_CODEC_ID_MPEG2VIDEO
		&& ost->st->codec->codec_id != AV_CODEC_ID_VC1
		) 
	{
			if (av_parser_change(ist->st->parser, ost->st->codec, &opkt.data, &opkt.size, pkt->data, pkt->size, pkt->flags & AV_PKT_FLAG_KEY))
				opkt.destruct = av_destruct_packet;
	} 
	else 
	{
		opkt.data = pkt->data;
		opkt.size = pkt->size;
	}

	if (ost->st->codec->codec_type == AVMEDIA_TYPE_VIDEO && (of->ctx->oformat->flags & AVFMT_RAWPICTURE)) 
	{
		/* store AVPicture in AVPacket, as expected by the output format */
		avpicture_fill(&pict, opkt.data, ost->st->codec->pix_fmt, ost->st->codec->width, ost->st->codec->height);
		opkt.data = (uint8_t *)&pict;
		opkt.size = sizeof(AVPicture);
		opkt.flags |= AV_PKT_FLAG_KEY;
	}

	write_frame(of->ctx, &opkt, ost.get(), video_sync_method_, audio_sync_method_);
	ost->st->codec->frame_number++;
	av_free_packet(&opkt);
}

void ffmpegwrapper::_close_output_stream( OutputStream * ost )                                                                                                           
{            
	boost::shared_ptr<OutputFile> of = output_file_list_[ost->file_index];

	ost->finished = 1;
	if (of->shortest) {
		int i;
		for (i = 0; i < of->ctx->nb_streams; i++)
			output_stream_list_[of->ost_index + i]->finished = 1;
	}        
} 


void ffmpegwrapper::_close_output_stream( boost::shared_ptr<OutputStream> ost )                                                                                                           
{            
	_close_output_stream(ost.get());
}   

int ffmpegwrapper::_need_output( void )                                                                                                                                 
{           
	size_t i;  

	for( i = 0; i < output_stream_list_.size(); i++ )
	{
		boost::shared_ptr<OutputStream> ost    = output_stream_list_[i];
		boost::shared_ptr<OutputFile> of       = output_file_list_[ost->file_index];
		AVFormatContext *os  = output_file_list_[ost->file_index]->ctx;

		if (ost->finished ||
			(os->pb && avio_tell(os->pb) >= of->limit_filesize))
			continue;
		if (ost->frame_number >= ost->max_frames) 
		{
			size_t j;
			for (j = 0; j < of->ctx->nb_streams; j++)
				_close_output_stream( output_stream_list_[of->ost_index + j] );
			continue;
		}   

		return 1;
	}       

	return 0;
}  

boost::shared_ptr<FilterGraph> ffmpegwrapper::_init_simple_filtergraph( boost::shared_ptr<InputStream> ist, boost::shared_ptr<OutputStream> ost )
{
	boost::shared_ptr<FilterGraph> fg( new FilterGraph );
	memset( fg.get(), 0, sizeof(FilterGraph) );
	if (!fg)
		return boost::shared_ptr<FilterGraph>();

	fg->index = filter_list_.size();

	fg->outputs = (OutputFilter **)grow_array(fg->outputs, sizeof(*fg->outputs), &fg->nb_outputs,                                                                             
		fg->nb_outputs + 1);
	if (!(fg->outputs[0] = (OutputFilter *)av_mallocz(sizeof(*fg->outputs[0]))))
	{
		av_free( fg->outputs );
		return boost::shared_ptr<FilterGraph>();
	}

	fg->outputs[0]->ost   = ost.get();
	fg->outputs[0]->graph = fg.get();

	ost->filter = fg->outputs[0];

	fg->inputs = (InputFilter **)grow_array(fg->inputs, sizeof(*fg->inputs), &fg->nb_inputs,
		fg->nb_inputs + 1);
	if (!(fg->inputs[0] = (InputFilter *)av_mallocz(sizeof(*fg->inputs[0]))))
	{
		av_free( fg->inputs );
		return boost::shared_ptr<FilterGraph>();
	}
	fg->inputs[0]->ist   = ist.get();
	fg->inputs[0]->graph = fg.get();

	ist->filters = (InputFilter **)grow_array(ist->filters, sizeof(*ist->filters),
		&ist->nb_filters, ist->nb_filters + 1);
	ist->filters[ist->nb_filters - 1] = fg->inputs[0];

	filter_list_.push_back( fg );
	return fg;
}

// pkt = NULL means EOF (needed to flush decoder buffers)
int ffmpegwrapper::_output_packet(boost::shared_ptr<InputStream> ist, const AVPacket *pkt) 
{
	int ret = 0, i;
	int got_output;

	AVPacket avpkt;
	if (!ist->saw_first_ts) 
	{
		ist->dts = ist->st->avg_frame_rate.num ? - ist->st->codec->has_b_frames * AV_TIME_BASE / av_q2d(ist->st->avg_frame_rate) : 0;
		ist->pts = 0;
		if (pkt != NULL && pkt->pts != AV_NOPTS_VALUE && !ist->decoding_needed) {
			ist->dts += av_rescale_q(pkt->pts, ist->st->time_base, _AV_TIME_BASE_Q);
			ist->pts = ist->dts; //unused but better to set it to a value thats not totally wrong
		}
		ist->saw_first_ts = 1;
	}

	if (ist->next_dts == AV_NOPTS_VALUE)
		ist->next_dts = ist->dts;
	if (ist->next_pts == AV_NOPTS_VALUE)
		ist->next_pts = ist->pts;

	if (pkt == NULL) 
	{
		/* EOF handling */
		av_init_packet(&avpkt);
		avpkt.data = NULL;
		avpkt.size = 0;
		goto handle_eof;
	} 
	else 
	{
		avpkt = *pkt;
	}

	if (pkt->dts != AV_NOPTS_VALUE)
	{
		ist->next_dts = ist->dts = av_rescale_q(pkt->dts, ist->st->time_base, _AV_TIME_BASE_Q);
		if (ist->st->codec->codec_type != AVMEDIA_TYPE_VIDEO || !ist->decoding_needed)
			ist->next_pts = ist->pts = av_rescale_q(pkt->dts, ist->st->time_base, _AV_TIME_BASE_Q);
	}

	// while we have more to decode or while the decoder did output something on EOF
	while (ist->decoding_needed && (avpkt.size > 0 || (!pkt && got_output))) 
	{
		int duration;
handle_eof:

		ist->pts = ist->next_pts;
		ist->dts = ist->next_dts;

		if (avpkt.size && avpkt.size != pkt->size) 
		{
			av_log(NULL, ist->showed_multi_packet_warning ? AV_LOG_VERBOSE : AV_LOG_WARNING,
				"Multiple frames in a packet from stream %d\n", pkt->stream_index);
			ist->showed_multi_packet_warning = 1;
		}

		switch (ist->st->codec->codec_type) 
		{
		case AVMEDIA_TYPE_AUDIO:
			// TODO:decode_audio //ret = decode_audio(ist, &avpkt, &got_output);
			break;
		case AVMEDIA_TYPE_VIDEO:
			ret = _decode_video(ist, &avpkt, &got_output);
			if (avpkt.duration) 
			{
				duration = av_rescale_q(avpkt.duration, ist->st->time_base, _AV_TIME_BASE_Q);
			} 
			else if(ist->st->codec->time_base.num != 0 && ist->st->codec->time_base.den != 0) 
			{
				int ticks= ist->st->parser ? ist->st->parser->repeat_pict+1 : ist->st->codec->ticks_per_frame;
				duration = ((int64_t)AV_TIME_BASE *
					ist->st->codec->time_base.num * ticks) /
					ist->st->codec->time_base.den;
			} 
			else
				duration = 0;

			if(ist->dts != AV_NOPTS_VALUE && duration) 
			{
				ist->next_dts += duration;
			}else
				ist->next_dts = AV_NOPTS_VALUE;

			if (got_output)
				ist->next_pts += duration; //FIXME the duration is not correct in some cases
			break;
		case AVMEDIA_TYPE_SUBTITLE:
			// TODO:transcode_subtitles //ret = transcode_subtitles(ist, &avpkt, &got_output);
			break;
		default:
			return -1;
		}

		if (ret < 0)
			return ret;

		avpkt.dts=
			avpkt.pts= AV_NOPTS_VALUE;

		// touch data and size only if not EOF
		if (pkt) 
		{
			if(ist->st->codec->codec_type != AVMEDIA_TYPE_AUDIO)
				ret = avpkt.size;
			avpkt.data += ret;
			avpkt.size -= ret;
		}
		if (!got_output) 
		{
			continue;
		}
	}

	/* handle stream copy */
	if (!ist->decoding_needed) 
	{
		_rate_emu_sleep(ist);
		ist->dts = ist->next_dts;
		switch (ist->st->codec->codec_type) 
		{
		case AVMEDIA_TYPE_AUDIO:
			ist->next_dts += ((int64_t)AV_TIME_BASE * ist->st->codec->frame_size) /
				ist->st->codec->sample_rate;
			break;
		case AVMEDIA_TYPE_VIDEO:
			if (pkt->duration) {
				ist->next_dts += av_rescale_q(pkt->duration, ist->st->time_base, _AV_TIME_BASE_Q);
			} else if(ist->st->codec->time_base.num != 0) {
				int ticks= ist->st->parser ? ist->st->parser->repeat_pict + 1 : ist->st->codec->ticks_per_frame;
				ist->next_dts += ((int64_t)AV_TIME_BASE *
					ist->st->codec->time_base.num * ticks) /
					ist->st->codec->time_base.den;
			}
			break;
		}
		ist->pts = ist->dts;
		ist->next_pts = ist->next_dts;
	}
	for (i = 0; pkt && i < output_stream_list_.size(); i++) 
	{
		boost::shared_ptr<OutputStream> ost = output_stream_list_[i];

		if (!_check_output_constraints(ist, ost) || ost->encoding_needed)
			continue;

		_do_streamcopy(ist, ost, pkt);
	}

	return 0;
}

int ffmpegwrapper::_check_output_constraints(boost::shared_ptr<InputStream> ist, boost::shared_ptr<OutputStream> ost)
{
	boost::shared_ptr<OutputFile> of = output_file_list_[ost->file_index];
    int ist_index  = input_file_list_[ist->file_index]->ist_index + ist->st->index;

    if (ost->source_index != ist_index)
        return 0;

    if (of->start_time && ist->pts < of->start_time)
        return 0;

    return 1;
}


void ffmpegwrapper::_flush_encoders(void)
{
	int i, ret;

	for (i = 0; i < output_stream_list_.size(); i++) 
	{
		boost::shared_ptr<OutputStream> ost = output_stream_list_[i];
		AVCodecContext *enc = ost->st->codec;
		AVFormatContext *os = output_file_list_[ost->file_index]->ctx;
		int stop_encoding = 0;

		if (!ost->encoding_needed)
			continue;

		if (ost->st->codec->codec_type == AVMEDIA_TYPE_AUDIO && enc->frame_size <= 1)
			continue;
		if (ost->st->codec->codec_type == AVMEDIA_TYPE_VIDEO && (os->oformat->flags & AVFMT_RAWPICTURE) && enc->codec->id == AV_CODEC_ID_RAWVIDEO)
			continue;

		for (;;) 
		{
			int (*encode)(AVCodecContext*, AVPacket*, const AVFrame*, int*) = NULL;
			const char *desc;
			int64_t *size;

			switch (ost->st->codec->codec_type) {
			case AVMEDIA_TYPE_AUDIO:
				encode = avcodec_encode_audio2;
				desc   = "Audio";
				size   = &audio_size_;
				break;
			case AVMEDIA_TYPE_VIDEO:
				encode = avcodec_encode_video2;
				desc   = "Video";
				size   = &video_size_;
				break;
			default:
				stop_encoding = 1;
			}

			if (encode) 
			{
				AVPacket pkt;
				int got_packet;
				av_init_packet(&pkt);
				pkt.data = NULL;
				pkt.size = 0;

				//update_benchmark(NULL);
				ret = encode(enc, &pkt, NULL, &got_packet);
				//update_benchmark("flush %s %d.%d", desc, ost->file_index, ost->index);
				if (ret < 0) {
					av_log(NULL, AV_LOG_FATAL, "%s encoding failed\n", desc);
					//exit(1);
					stop_encoding = 1;
					break;
				}
				*size += pkt.size;
				//if (ost->logfile && enc->stats_out) {
					//fprintf(ost->logfile, "%s", enc->stats_out);
				//}
				if (!got_packet) {
					stop_encoding = 1;
					break;
				}
				if (pkt.pts != AV_NOPTS_VALUE)
					pkt.pts = av_rescale_q(pkt.pts, enc->time_base, ost->st->time_base);
				if (pkt.dts != AV_NOPTS_VALUE)
					pkt.dts = av_rescale_q(pkt.dts, enc->time_base, ost->st->time_base);
				write_frame(os, &pkt, ost.get(), video_sync_method_, audio_sync_method_);
			}

			if (stop_encoding)
				break;
		}
	}
}

bool ffmpegwrapper::_init_input_filter( boost::shared_ptr<FilterGraph> fg, AVFilterInOut *in )
{
	boost::shared_ptr<InputStream> ist;
    enum AVMediaType type = avfilter_pad_get_type(in->filter_ctx->input_pads, in->pad_idx);
    size_t i;
 
    // TODO: support other filter types
    if (type != AVMEDIA_TYPE_VIDEO && type != AVMEDIA_TYPE_AUDIO) {
        av_log(NULL, AV_LOG_FATAL, "Only video and audio filters supported "
               "currently.\n");
		return false;
	}

	if (in->name) 
	{
		AVFormatContext *s;
		AVStream       *st = NULL;
		char *p;
		int file_idx = strtol(in->name, &p, 0);

		if (file_idx < 0 || file_idx >= input_file_list_.size()) 
		{
			av_log(NULL, AV_LOG_FATAL, "Invalid file index %d in filtergraph description %s.\n",
				file_idx, fg->graph_desc);
			return false;	// exit(1);
		}
		s = input_file_list_[file_idx]->ctx;

		for (i = 0; i < s->nb_streams; i++)
		{
			enum AVMediaType stream_type = s->streams[i]->codec->codec_type;
			if (stream_type != type &&
				!(stream_type == AVMEDIA_TYPE_SUBTITLE &&
				type == AVMEDIA_TYPE_VIDEO /* sub2video hack */))
				continue;
			if (check_stream_specifier(s, s->streams[i], *p == ':' ? p + 1 : p) == 1) 
			{
				st = s->streams[i];
				break;
			}
		}
		if (!st) 
		{
			av_log(NULL, AV_LOG_FATAL, "Stream specifier '%s' in filtergraph description %s "
				"matches no streams.\n", p, fg->graph_desc);
			return false;	// exit(1);
		}
		ist = input_stream_list_[input_file_list_[file_idx]->ist_index + st->index];
	} 
	else 
	{
		/* find the first unused stream of corresponding type */
		for (i = 0; i < input_stream_list_.size(); i++) 
		{
			ist = input_stream_list_[i];
			if (ist->st->codec->codec_type == type && ist->discard)
				break;
		}
		if (i == input_stream_list_.size()) 
		{
			av_log(NULL, AV_LOG_FATAL, "Cannot find a matching stream for "
				"unlabeled input pad %d on filter %s\n", in->pad_idx,
				in->filter_ctx->name);
			return false;	//exit(1);
		}
	}
	return true;
}

int ffmpegwrapper::_configure_filtergraph( boost::shared_ptr<FilterGraph> fg )
{                             
	AVFilterInOut *inputs, *outputs, *cur;
	int ret, i, init = !fg->graph, simple = !fg->graph_desc;
	const char *graph_desc = simple ? fg->outputs[0]->ost->avfilter : fg->graph_desc;

	avfilter_graph_free(&fg->graph);
	if (!(fg->graph = avfilter_graph_alloc()))
		return AVERROR(ENOMEM);   

	if (simple) {
		OutputStream *ost = fg->outputs[0]->ost;
		char args[255];
		snprintf(args, sizeof(args), "flags=0x%X", (unsigned)ost->sws_flags);
		fg->graph->scale_sws_opts = av_strdup(args);
	}

	if ((ret = avfilter_graph_parse2(fg->graph, graph_desc, &inputs, &outputs)) < 0)
		return ret;

    if (simple && (!inputs || inputs->next || !outputs || outputs->next)) {
        av_log(NULL, AV_LOG_ERROR, "Simple filtergraph '%s' does not have "
               "exactly one input and output.\n", graph_desc);
        return AVERROR(EINVAL);
    }
 
	for (cur = inputs; !simple && init && cur; cur = cur->next)
		_init_input_filter( fg, cur );

	for (cur = inputs, i = 0; cur; cur = cur->next, i++)
		if ((ret = _configure_input_filter(fg.get(), fg->inputs[i], cur)) < 0)
			return ret;
	avfilter_inout_free(&inputs);

	if (!init || simple) 
	{
		/* we already know the mappings between lavfi outputs and output streams,
		* so we can finish the setup */
		for (cur = outputs, i = 0; cur; cur = cur->next, i++)
			_configure_output_filter( fg.get(), fg->outputs[i], cur );
		avfilter_inout_free(&outputs);

		if ((ret = avfilter_graph_config(fg->graph, NULL)) < 0)
			return ret;
	} 
	else 
	{
		/* wait until output mappings are processed */
		for (cur = outputs; cur;) 
		{
			fg->outputs = (OutputFilter **)grow_array(fg->outputs, sizeof(*fg->outputs),
				&fg->nb_outputs, fg->nb_outputs + 1);
			if (!(fg->outputs[fg->nb_outputs - 1] = (OutputFilter *)av_mallocz(sizeof(*fg->outputs[0]))))
				return -1;	// exit(1);
			fg->outputs[fg->nb_outputs - 1]->graph   = fg.get();
			fg->outputs[fg->nb_outputs - 1]->out_tmp = cur;
			cur = cur->next;
			fg->outputs[fg->nb_outputs - 1]->out_tmp->next = NULL;
		}
	}    
	return 0;
}

boost::shared_ptr<InputStream> ffmpegwrapper::_get_input_stream( boost::shared_ptr<OutputStream> ost )
{
	if (ost->source_index >= 0)                                                                                                                              
		return input_stream_list_[ost->source_index];
	return boost::shared_ptr<InputStream>();
}

int ffmpegwrapper::_sub2video_prepare( InputStream *ist )
{          
	AVFormatContext *avf = input_file_list_[ist->file_index]->ctx;
	int i, ret, w, h;
	uint8_t *image[4];
	int linesize[4];

	/* Compute the size of the canvas for the subtitles stream.
	If the subtitles codec has set a size, use it. Otherwise use the
	maximum dimensions of the video streams in the same file. */
	w = ist->st->codec->width;
	h = ist->st->codec->height;
	if (!(w && h)) 
	{
		for (i = 0; i < avf->nb_streams; i++) 
		{
			if (avf->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) 
			{
				w = FFMAX(w, avf->streams[i]->codec->width);
				h = FFMAX(h, avf->streams[i]->codec->height);
			}
		}  
		if (!(w && h)) 
		{
			w = FFMAX(w, 720);
			h = FFMAX(h, 576);
		}  
		av_log(avf, AV_LOG_INFO, "sub2video: using %dx%d canvas\n", w, h);
	}      
	ist->sub2video.w = ist->st->codec->width  = w;
	ist->sub2video.h = ist->st->codec->height = h;

	/* rectangles are PIX_FMT_PAL8, but we have no guarantee that the
	palettes for all rectangles are identical or compatible */
	ist->st->codec->pix_fmt = PIX_FMT_RGB32;

	ret = av_image_alloc(image, linesize, w, h, PIX_FMT_RGB32, 32);
	if (ret < 0)
		return ret;
	memset(image[0], 0, h * linesize[0]);
	ist->sub2video.ref = avfilter_get_video_buffer_ref_from_arrays(
		image, linesize, AV_PERM_READ | AV_PERM_PRESERVE,
		w, h, PIX_FMT_RGB32);
	if (!ist->sub2video.ref) 
	{
		av_free(image[0]);
		return AVERROR(ENOMEM);
	}      
	return 0;
} 

int ffmpegwrapper::_configure_input_video_filter(FilterGraph *fg, InputFilter *ifilter, AVFilterInOut *in)
{   
	AVFilterContext *first_filter = in->filter_ctx;
	AVFilter *filter = avfilter_get_by_name("buffer");
	InputStream *ist = ifilter->ist;
	AVRational tb = ist->framerate.num ? av_inv_q(ist->framerate) :
		ist->st->time_base;
	AVRational fr = ist->framerate.num ? ist->framerate :
		ist->st->r_frame_rate;
	AVRational sar;
	AVBPrint args;
	char name[255];
	int pad_idx = in->pad_idx;
	int ret = 0;

	if (ist->st->codec->codec_type == AVMEDIA_TYPE_SUBTITLE) 
	{
		ret = _sub2video_prepare(ist);
		if (ret < 0)
			return ret;
	}

	sar = ist->st->sample_aspect_ratio.num ?
		ist->st->sample_aspect_ratio :
	ist->st->codec->sample_aspect_ratio;
	if(!sar.den)
	{
		AVRational temp = {0, 1};
		sar = temp;
	}
	av_bprint_init(&args, 0, 1);
	av_bprintf(&args,
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:"
		"pixel_aspect=%d/%d:sws_param=flags=%d", ist->st->codec->width,
		ist->st->codec->height, ist->st->codec->pix_fmt,
		tb.num, tb.den, sar.num, sar.den,
		SWS_BILINEAR + ((ist->st->codec->flags&CODEC_FLAG_BITEXACT) ? SWS_BITEXACT:0));
	if (fr.num && fr.den)
		av_bprintf(&args, ":frame_rate=%d/%d", fr.num, fr.den);
	snprintf(name, sizeof(name), "graph %d input from stream %d:%d", fg->index,
		ist->file_index, ist->st->index);


	if ((ret = avfilter_graph_create_filter(&ifilter->filter, filter, name,
		args.str, NULL, fg->graph)) < 0)
		return ret;

	if (ist->framerate.num) 
	{
		AVFilterContext *setpts;

		snprintf(name, sizeof(name), "force CFR for input from stream %d:%d",
			ist->file_index, ist->st->index);
		if ((ret = avfilter_graph_create_filter(&setpts,
			avfilter_get_by_name("setpts"),
			name, "N", NULL,
			fg->graph)) < 0)
			return ret;

		if ((ret = avfilter_link(setpts, 0, first_filter, pad_idx)) < 0)
			return ret;

		first_filter = setpts;
		pad_idx = 0;
	}       

	if ((ret = avfilter_link(ifilter->filter, 0, first_filter, pad_idx)) < 0)
		return ret;
	return 0;
}


int ffmpegwrapper::_configure_output_video_filter( FilterGraph *fg, OutputFilter *ofilter, AVFilterInOut *out )
{
	char *pix_fmts;
	OutputStream *ost = ofilter->ost;
    AVCodecContext *codec = ost->st->codec;
    AVFilterContext *last_filter = out->filter_ctx;
    int pad_idx = out->pad_idx;
    int ret;
    char name[255];
    AVBufferSinkParams *buffersink_params = av_buffersink_params_alloc();
 
    snprintf(name, sizeof(name), "output stream %d:%d", ost->file_index, ost->index);
    ret = avfilter_graph_create_filter(&ofilter->filter,
                                       avfilter_get_by_name("ffbuffersink"),
                                       name, NULL, NULL, fg->graph);
    av_freep(&buffersink_params);
 
    if (ret < 0)
        return ret;

    if (codec->width || codec->height) 
	{
        char args[255];
        AVFilterContext *filter;
 
        snprintf(args, sizeof(args), "%d:%d:flags=0x%X",
                 codec->width,
                 codec->height,
                 (unsigned)ost->sws_flags);
        snprintf(name, sizeof(name), "scaler for output stream %d:%d",
                 ost->file_index, ost->index);
        if ((ret = avfilter_graph_create_filter(&filter, avfilter_get_by_name("scale"),
                                                name, args, NULL, fg->graph)) < 0)
            return ret;
        if ((ret = avfilter_link(last_filter, pad_idx, filter, 0)) < 0)
            return ret;
 
        last_filter = filter;
        pad_idx = 0;
	}

	if ((pix_fmts = choose_pix_fmts(ost))) 
	{
		AVFilterContext *filter;
		snprintf(name, sizeof(name), "pixel format for output stream %d:%d",
			ost->file_index, ost->index);
		if ((ret = avfilter_graph_create_filter(&filter,
			avfilter_get_by_name("format"),
			"format", pix_fmts, NULL,
			fg->graph)) < 0)
			return ret;
		if ((ret = avfilter_link(last_filter, pad_idx, filter, 0)) < 0)
			return ret;

		last_filter = filter;
		pad_idx     = 0;
		av_freep(&pix_fmts);
	}

	if (ost->frame_rate.num && 0) 
	{
		AVFilterContext *fps;
		char args[255];

		snprintf(args, sizeof(args), "fps=%d/%d", ost->frame_rate.num,
			ost->frame_rate.den);
		snprintf(name, sizeof(name), "fps for output stream %d:%d",
			ost->file_index, ost->index);
		ret = avfilter_graph_create_filter(&fps, avfilter_get_by_name("fps"),
			name, args, NULL, fg->graph);
		if (ret < 0)
			return ret;

		ret = avfilter_link(last_filter, pad_idx, fps, 0);
		if (ret < 0)
			return ret;
		last_filter = fps;
		pad_idx = 0;
	}       

	if ((ret = avfilter_link(last_filter, pad_idx, ofilter->filter, 0)) < 0)
		return ret;

	return ret;
}


int ffmpegwrapper::_configure_input_filter( FilterGraph *fg, InputFilter *ifilter, AVFilterInOut *in )
{
	av_freep(&ifilter->name);
	DESCRIBE_FILTER_LINK(ifilter, in, true);

	switch (avfilter_pad_get_type(in->filter_ctx->input_pads, in->pad_idx)) 
	{
	case AVMEDIA_TYPE_VIDEO: return _configure_input_video_filter(fg, ifilter, in);
	// TODO: case AVMEDIA_TYPE_AUDIO: return configure_input_audio_filter(fg, ifilter, in);
	default: av_assert0(0);
	}

	return -1;
}


int ffmpegwrapper::_configure_output_filter( FilterGraph *fg, OutputFilter *ofilter, AVFilterInOut *out )
{                                                                                                                                                            
	av_freep(&ofilter->name);
	DESCRIBE_FILTER_LINK(ofilter, out, 0);
	
	switch (avfilter_pad_get_type(out->filter_ctx->output_pads, out->pad_idx)) 
	{
	case AVMEDIA_TYPE_VIDEO: return _configure_output_video_filter( fg, ofilter, out );
		// TODO: case AVMEDIA_TYPE_AUDIO: return configure_output_audio_filter(fg, ofilter, out);
	default: av_assert0(0);
	}

	return -1;
}

void ffmpegwrapper::_rate_emu_sleep(boost::shared_ptr<InputStream> ist)
{
	if (input_file_list_[ist->file_index]->rate_emu) {
		int64_t pts = av_rescale(ist->dts, 1000000, AV_TIME_BASE);
		int64_t now = av_gettime() - ist->start;
		if (pts > now)
			av_usleep(pts - now);
	}
}

int ffmpegwrapper::_decode_video(boost::shared_ptr<InputStream> ist, AVPacket *pkt, int *got_output)
{
	AVFrame *decoded_frame;
	void *buffer_to_free = NULL;
	int i, ret = 0, resample_changed;
	int64_t best_effort_timestamp;
	AVRational *frame_sample_aspect;
	float quality;

	if (!ist->decoded_frame && !(ist->decoded_frame = avcodec_alloc_frame()))
		return AVERROR(ENOMEM);
	else
		avcodec_get_frame_defaults(ist->decoded_frame);

	decoded_frame = ist->decoded_frame;
	pkt->dts  = av_rescale_q(ist->dts, _AV_TIME_BASE_Q, ist->st->time_base);

	update_benchmark(NULL);
	ret = avcodec_decode_video2(ist->st->codec,
		decoded_frame, got_output, pkt);
	update_benchmark("decode_video %d.%d", ist->file_index, ist->st->index);
	if (!*got_output || ret < 0)
	{
		if (!pkt->size) 
		{
			for (i = 0; i < ist->nb_filters; i++)
				av_buffersrc_add_ref(ist->filters[i]->filter, NULL, 0);
		}
		return ret;
	}

	quality = same_quant_ ? decoded_frame->quality : 0;

	if(ist->top_field_first>=0)
		decoded_frame->top_field_first = ist->top_field_first;

	best_effort_timestamp= av_frame_get_best_effort_timestamp(decoded_frame);
	if(best_effort_timestamp != AV_NOPTS_VALUE)
		ist->next_pts = ist->pts = av_rescale_q(decoded_frame->pts = best_effort_timestamp, ist->st->time_base, _AV_TIME_BASE_Q);

	//if (debug_ts) 
	//{
	//	av_log(NULL, AV_LOG_INFO, "decoder -> ist_index:%d type:video "
	//		"frame_pts:%s frame_pts_time:%s best_effort_ts:%"PRId64" best_effort_ts_time:%s keyframe:%d frame_type:%d \n",
	//		ist->st->index, _av_ts2str(decoded_frame->pts),
	//		_av_ts2timestr(decoded_frame->pts, &ist->st->time_base),
	//		best_effort_timestamp,
	//		_av_ts2timestr(best_effort_timestamp, &ist->st->time_base),
	//		decoded_frame->key_frame, decoded_frame->pict_type);
	//}

	pkt->size = 0;
	pre_process_video_frame(ist.get(), (AVPicture *)decoded_frame, do_deinterlace_ ? 1 : 0, &buffer_to_free);

	_rate_emu_sleep(ist);

	if (ist->st->sample_aspect_ratio.num)
		decoded_frame->sample_aspect_ratio = ist->st->sample_aspect_ratio;

	resample_changed = ist->resample_width   != decoded_frame->width  ||
		ist->resample_height  != decoded_frame->height ||
		ist->resample_pix_fmt != decoded_frame->format;
	if (resample_changed) 
	{
		av_log(NULL, AV_LOG_INFO,
			"Input stream #%d:%d frame changed from size:%dx%d fmt:%s to size:%dx%d fmt:%s\n",
			ist->file_index, ist->st->index,
			ist->resample_width,  ist->resample_height,  av_get_pix_fmt_name((PixelFormat)ist->resample_pix_fmt),
			decoded_frame->width, decoded_frame->height, av_get_pix_fmt_name((PixelFormat)decoded_frame->format));

		ist->resample_width   = decoded_frame->width;
		ist->resample_height  = decoded_frame->height;
		ist->resample_pix_fmt = decoded_frame->format;

		for (i = 0; i < filter_list_.size(); i++)
		{
			if (ist_in_filtergraph(filter_list_[i].get(), ist.get()) &&
				_configure_filtergraph(filter_list_[i]) < 0) 
			{
				/*av_log(NULL, AV_LOG_FATAL, "Error reinitializing filters!\n");
				exit(1);*/
				if(buffer_to_free) 
					av_free(buffer_to_free);
				return -1;
			}
		}
	}

	frame_sample_aspect= (AVRational *)av_opt_ptr(avcodec_get_frame_class(), decoded_frame, "sample_aspect_ratio");
	for (i = 0; i < ist->nb_filters; i++) 
	{
		int changed = ist->st->codec->width   != ist->filters[i]->filter->outputs[0]->w
			|| ist->st->codec->height  != ist->filters[i]->filter->outputs[0]->h
			|| ist->st->codec->pix_fmt != ist->filters[i]->filter->outputs[0]->format;
		// XXX what an ugly hack
		if (ist->filters[i]->graph->nb_outputs == 1)
			ist->filters[i]->graph->outputs[0]->ost->last_quality = quality;

		if (!frame_sample_aspect->num)
			*frame_sample_aspect = ist->st->sample_aspect_ratio;
		if (ist->dr1 && decoded_frame->type==FF_BUFFER_TYPE_USER && !changed) 
		{
			FrameBuffer      *buf = (FrameBuffer *)decoded_frame->opaque;
			AVFilterBufferRef *fb = avfilter_get_video_buffer_ref_from_arrays(
				decoded_frame->data, decoded_frame->linesize,
				AV_PERM_READ | AV_PERM_PRESERVE,
				ist->st->codec->width, ist->st->codec->height,
				ist->st->codec->pix_fmt);

			avfilter_copy_frame_props(fb, decoded_frame);
			fb->buf->priv           = buf;
			fb->buf->free           = filter_release_buffer;

			av_assert0(buf->refcount>0);
			buf->refcount++;
			av_buffersrc_add_ref(ist->filters[i]->filter, fb,
				AV_BUFFERSRC_FLAG_NO_CHECK_FORMAT |
				AV_BUFFERSRC_FLAG_NO_COPY |
				AV_BUFFERSRC_FLAG_PUSH);
		} 
		else
		{
			if(av_buffersrc_add_frame(ist->filters[i]->filter, decoded_frame, AV_BUFFERSRC_FLAG_PUSH)<0) {
				/*av_log(NULL, AV_LOG_FATAL, "Failed to inject frame into filter network\n");
				exit(1);*/
				if(buffer_to_free) 
					av_free(buffer_to_free);
				return -1;
			}
		}
	}

	av_free(buffer_to_free);
	return ret;
}

bool ffmpegwrapper::openInputDevice( void )
{
	AVFormatContext *ic = NULL;
	AVDictionary* options = NULL;
	AVDictionary **input_opts = NULL;
	int64_t timestamp;
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

		AVInputFormat *iformat = av_find_input_format( inputFormatName_.c_str() );
		if( !iformat )
			break;

	/*		
		// TODO: openInputDevice codec_names
		MATCH_PER_TYPE_OPT(codec_names, str,    video_codec_name, ic, "v");
		MATCH_PER_TYPE_OPT(codec_names, str,    audio_codec_name, ic, "a");
		MATCH_PER_TYPE_OPT(codec_names, str, subtitle_codec_name, ic, "s");*/

		ic->video_codec_id   = /*video_codec_name ?
			find_codec_or_die(video_codec_name   , AVMEDIA_TYPE_VIDEO   , 0)->id :*/ AV_CODEC_ID_NONE;
		ic->audio_codec_id   = /*audio_codec_name ?
			find_codec_or_die(audio_codec_name   , AVMEDIA_TYPE_AUDIO   , 0)->id :*/ AV_CODEC_ID_NONE;
		ic->subtitle_codec_id= /*subtitle_codec_name ?
			find_codec_or_die(subtitle_codec_name, AVMEDIA_TYPE_SUBTITLE, 0)->id :*/ AV_CODEC_ID_NONE;
		ic->flags |= AVFMT_FLAG_NONBLOCK;
		ic->interrupt_callback = int_cb;

		err = avformat_open_input( &ic, inputFileName_.c_str(), iformat, &options );
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

		timestamp = start_time_;
		/* add the stream start time */
		if (ic->start_time != AV_NOPTS_VALUE)
			timestamp += ic->start_time;

		/* if seeking requested, we execute it */
		if (start_time_ != 0) {
			ret = av_seek_frame(ic, -1, timestamp, AVSEEK_FLAG_BACKWARD);
			/*if (ret < 0) {
				av_log(NULL, AV_LOG_WARNING, "%s: could not seek to position %0.3f\n",
					filename, (double)timestamp / AV_TIME_BASE);
			}*/
		}

		_add_input_streams( ic );

		boost::shared_ptr<InputFile> inputFile( new InputFile );
		memset( inputFile.get(), 0, sizeof(InputFile) );

		inputFile->ctx        = ic;
		inputFile->ist_index  = input_stream_list_.size() - ic->nb_streams;
		inputFile->ts_offset  = input_ts_offset_ - (copy_ts_ ? 0 : timestamp);
		inputFile->nb_streams = ic->nb_streams;
		inputFile->rate_emu   = rate_emu_;
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

	int err = 0;
	int i;
	bool res = false;
	do
	{
		oc = avformat_alloc_context();
		if( !oc )
			break;

		AVOutputFormat *oformat = av_guess_format( NULL, outputFileName_.c_str(), NULL );
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
		if (oc->oformat->video_codec != AV_CODEC_ID_NONE) 
		{
			int area = 0, idx = -1;
			int qcr = avformat_query_codec(oc->oformat, oc->oformat->video_codec, 0);

			for ( i = 0; i < input_stream_list_.size(); i++) 
			{
				int new_area;
				boost::shared_ptr<InputStream> ist = input_stream_list_[i];
				new_area = ist->st->codec->width * ist->st->codec->height;
				if((qcr!=MKTAG('A', 'P', 'I', 'C')) && (ist->st->disposition & AV_DISPOSITION_ATTACHED_PIC))
					new_area = 1;
				if (ist->st->codec->codec_type == AVMEDIA_TYPE_VIDEO &&
					new_area > area) 
				{
					if((qcr==MKTAG('A', 'P', 'I', 'C')) && !(ist->st->disposition & AV_DISPOSITION_ATTACHED_PIC))
						continue;
					area = new_area;
					idx = i;
				}
			}

			if (idx >= 0)
				_add_video_streams( oc, idx );

			// TODO:audio: most channels
			// TODO:subtitles: pick first
		}

		//for (i = nb_output_streams - oc->nb_streams; i < nb_output_streams; i++) { //for all streams of this output file
		//	AVDictionaryEntry *e;
		//	ost = output_streams[i];

		//	if (   ost->stream_copy
		//		&& (e = av_dict_get(codec_opts, "flags", NULL, AV_DICT_IGNORE_SUFFIX))
		//		&& (!e->key[5] || check_stream_specifier(oc, ost->st, e->key+6)))
		//		if (av_opt_set(ost->st->codec, "flags", e->value, 0) < 0)
		//			exit(1);
		//}   

		// TODO:handle attached file


		boost::shared_ptr<OutputFile> outputFile(new OutputFile);
		memset( outputFile.get(), 0, sizeof(OutputFile) );

		output_file_list_.push_back( outputFile );
		outputFile->ctx            = oc;
		outputFile->ost_index      = output_stream_list_.size() - oc->nb_streams;
		outputFile->recording_time = recording_time_;
		if (recording_time_ != INT64_MAX)
			oc->duration = recording_time_;
		outputFile->start_time     = start_time_;
		outputFile->limit_filesize = limit_filesize_;
		//outputFile->shortest       = o->shortest;
		//av_dict_copy(&outputFile->opts, format_opts, 0);

		if( oc->oformat->flags & AVFMT_NEEDNUMBER )
		{
			if( !av_filename_number_test(oc->filename) )
			{
				break;
			}
		}

		if (!(oc->oformat->flags & AVFMT_NOFILE)) 
		{
			av_strlcpy( oc->filename, outputFileName_.c_str(), sizeof(oc->filename) );

			// open the file
			if ((err = avio_open2(&oc->pb, outputFileName_.c_str(), AVIO_FLAG_WRITE,
				&oc->interrupt_callback,
				&options)) < 0)
			{
				break;
			}
		}

		// TODO:mux_preload
		// TODO:mux_max_delay
		oc->max_delay = (int)(DEFAULT_MUX_MAX_DELAY * AV_TIME_BASE);
		// TODO:copy metadata
		// TODO:copy chapters

		if (!metadata_global_manual_ && input_file_list_.size() > 0)
		{
			av_dict_copy(&oc->metadata, input_file_list_[0]->ctx->metadata,
				AV_DICT_DONT_OVERWRITE);
			if(recording_time_ != INT64_MAX)
				av_dict_set(&oc->metadata, "duration", NULL, 0);
			av_dict_set(&oc->metadata, "creation_time", NULL, 0);
		}

		// TODO:metadata_streams_manual
		if ( 1 )
		{
			assert( output_file_list_.size() > 0 );
			boost::shared_ptr<OutputFile> outputFile = output_file_list_[ output_file_list_.size() - 1 ];    
			for( i = outputFile->ost_index; i < output_stream_list_.size(); i++ ) 
			{
				boost::shared_ptr<InputStream> ist;    
				int idx = output_stream_list_[i]->source_index;

				if ( idx < 0)         /* this is true e.g. for attached files */
					continue;                         
				ist = input_stream_list_[ idx ];
				av_dict_copy( &output_stream_list_[i]->st->metadata, ist->st->metadata, AV_DICT_DONT_OVERWRITE );                                                       
			}    
		}

		// TODO:process manually set metadata

		res = true;

	}while( false );

	if( ! res )
		clear();

	return res;
}


bool ffmpegwrapper::transcode( void )
{
	size_t i = 0;
	int ret = _transcode_init();                                                                                                                                  
	if (ret < 0)
		return false;

	while( !stop_ )
	{
		if (!_need_output()) {
            av_log(NULL, AV_LOG_VERBOSE, "No more output streams to write to, finishing.\n");
            break;
        }  

		ret = _transcode_step();
        if (ret < 0) {
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                continue;

            av_log(NULL, AV_LOG_ERROR, "Error while filtering.\n");
            break;
        }
	}

	// at the end of stream, we must flush the decoder buffers
    for (i = 0; i < input_stream_list_.size(); i++) 
	{
       boost::shared_ptr<InputStream>  ist = input_stream_list_[i];
        if (!input_file_list_[ist->file_index]->eof_reached && ist->decoding_needed) 
		{
            _output_packet(ist, NULL);
        }
    }
    _flush_encoders();

	// write the trailer if needed and close file
	for (i = 0; i < output_file_list_.size(); i++) 
	{
		AVFormatContext *os;
		os = output_file_list_[i]->ctx;
		av_write_trailer(os);
	}

	// close each encoder
	for (i = 0; i < output_stream_list_.size(); i++) 
	{
		boost::shared_ptr<OutputStream> ost = output_stream_list_[i];
		if (ost->encoding_needed)
		{
			av_freep(&ost->st->codec->stats_in);
			avcodec_close(ost->st->codec);
		}
	}

	// close each decoder
	for (i = 0; i < input_stream_list_.size(); i++) 
	{
		boost::shared_ptr<InputStream>  ist = input_stream_list_[i];
		if (ist->decoding_needed) {
			avcodec_close(ist->st->codec);
		}
	}

	/* finished ! */
	ret = 0;

	if(output_stream_list_.size() > 0 ) 
	{
		for (i = 0; i < output_stream_list_.size(); i++) {
			boost::shared_ptr<OutputStream> ost = output_stream_list_[i];
			if (ost) 
			{
				if (ost->stream_copy)
					av_freep(&ost->st->codec->extradata);
				// TODO:LOGFILE
				//if (ost->logfile) 
				//{
				//	fclose(ost->logfile);
				//	ost->logfile = NULL;
				//}
				av_freep(&ost->st->codec->subtitle_header);
				av_free(ost->forced_kf_pts);
				av_dict_free(&ost->opts);
			}
		}
	}

	return true;
}

void ffmpegwrapper::_clearAllResource() 
{
	int i, j;

	for (i = 0; i < filter_list_.size(); i++) 
	{
		avfilter_graph_free(&filter_list_[i]->graph);
		for (j = 0; j < filter_list_[i]->nb_inputs; j++) 
		{
			av_freep(&filter_list_[i]->inputs[j]->name);
			av_freep(&filter_list_[i]->inputs[j]);
		}
		av_freep(&filter_list_[i]->inputs);
		for (j = 0; j < filter_list_[i]->nb_outputs; j++) 
		{
			av_freep(&filter_list_[i]->outputs[j]->name);
			av_freep(&filter_list_[i]->outputs[j]);
		}
		av_freep(&filter_list_[i]->outputs);
	}
	filter_list_.clear();

	// TODO:subtitle free
	//av_freep(&subtitle_out);

	// close files
	for (i = 0; i < output_file_list_.size(); i++) 
	{
		AVFormatContext *s = output_file_list_[i]->ctx;
		if (!(s->oformat->flags & AVFMT_NOFILE) && s->pb)
			avio_close(s->pb);
		avformat_free_context(s);
		av_dict_free(&output_file_list_[i]->opts);
	}
	for (i = 0; i < output_stream_list_.size(); i++) 
	{
		AVBitStreamFilterContext *bsfc = output_stream_list_[i]->bitstream_filters;
		while (bsfc) 
		{
			AVBitStreamFilterContext *next = bsfc->next;
			av_bitstream_filter_close(bsfc);
			bsfc = next;
		}
		output_stream_list_[i]->bitstream_filters = NULL;
		avcodec_free_frame(&output_stream_list_[i]->filtered_frame);

		av_freep(&output_stream_list_[i]->forced_keyframes);
		av_freep(&output_stream_list_[i]->avfilter);
		//av_freep(&output_stream_list_[i]->logfile_prefix);
	}
	for (i = 0; i < input_file_list_.size(); i++) 
	{
		avformat_close_input(&input_file_list_[i]->ctx);
	}
	for (i = 0; i < input_stream_list_.size(); i++) 
	{
		avcodec_free_frame(&input_stream_list_[i]->decoded_frame);
		av_dict_free(&input_stream_list_[i]->opts);
		free_buffer_pool(&input_stream_list_[i]->buffer_pool);
		avfilter_unref_bufferp(&input_stream_list_[i]->sub2video.ref);
		av_freep(&input_stream_list_[i]->filters);
	}

	// TODO:VSTAT
	//if (vstats_file)
	//    fclose(vstats_file);
	//av_free(vstats_filename);

//	avfilter_uninit();
//	avformat_network_deinit();
}


void ffmpegwrapper::clear( void )
{
	stop_ = true;

	_clearAllResource();

	input_stream_list_.clear();
	input_file_list_.clear();
	output_stream_list_.clear();
	output_file_list_.clear();

	swr_free(&swr_opts_);
	sws_freeContext(sws_opts_);
}
