#include "stdafx.h"
#include "ffmpegwrapper.h"

#include "stdint.h"

#define VSYNC_AUTO       -1                                                                                                                                  
#define VSYNC_PASSTHROUGH 0
#define VSYNC_CFR         1
#define VSYNC_VFR         2
#define VSYNC_DROP        0xff

#define DEFAULT_MUX_MAX_DELAY	0.7

const AVIOInterruptCB int_cb = { decode_interrupt_cb, NULL };

// http://nerdlogger.com/2011/11/03/stream-your-windows-desktop-using-ffmpeg/
// ffmpeg -f dshow  -i video="UScreenCapture"  -r 30 -vcodec mpeg4 -q 12 a.avi
// ffmpeg -list_devices true -f dshow -i dummy
// >>> libavdevice/openal-dec.c
//------------------------------------------------------------------------------------------

ffmpegwrapper::ffmpegwrapper()
{
	stop_ = false;
	do_deinterlace_ = false;
	extra_size_ = 0;
	input_frame_rate_ = "30";
	output_frame_rate_ = "30";
	quality_ = 12;
	video_sync_method_ = VSYNC_AUTO;
	outputFileName_ = "c:\\keh.avi";

	codec_opts_ = NULL;
}


void ffmpegwrapper::init( void )
{
	swr_opts_ = swr_alloc();
	sws_opts_ = sws_getContext(16, 16, (PixelFormat)0, 16, 16, (PixelFormat)0, SWS_BICUBIC,
		NULL, NULL, NULL);

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

	choose_encoder( "mpeg4", oc, ost );

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
					exit(1);
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


int ffmpegwrapper::_transcode_step( void )
{
	// TODOMUST
	return -1;
}

             
void ffmpegwrapper::_close_output_stream( boost::shared_ptr<OutputStream> ost )                                                                                                           
{            
    boost::shared_ptr<OutputFile> of = output_file_list_[ost->file_index];
             
    ost->finished = 1;
    if (of->shortest) {
        int i;
        for (i = 0; i < of->ctx->nb_streams; i++)
            output_stream_list_[of->ost_index + i]->finished = 1;
    }        
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
	// TODOMUST
	return -1;
}

void ffmpegwrapper::_flush_encoders(void)
{
	// TODOMUST
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
			exit(1);
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
			exit(1);
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
			exit(1);
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
				exit(1);
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
		//outputFile->recording_time = o->recording_time;
		//if (o->recording_time != INT64_MAX)
		//	oc->duration = o->recording_time;
		//outputFile->start_time     = o->start_time;
		//outputFile->limit_filesize = o->limit_filesize;
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
		// TODO:copy global metadata by default
    //if (!o->metadata_global_manual && nb_input_files){
    //    av_dict_copy(&oc->metadata, input_files[0]->ctx->metadata,
    //                 AV_DICT_DONT_OVERWRITE);
    //    if(o->recording_time != INT64_MAX)
    //        av_dict_set(&oc->metadata, "duration", NULL, 0);
    //    av_dict_set(&oc->metadata, "creation_time", NULL, 0);
    //}
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

	return true;
}


void ffmpegwrapper::clear( void )
{
	stop_ = true;

	input_stream_list_.clear();
	input_file_list_.clear();
	output_stream_list_.clear();
	output_file_list_.clear();

	swr_free(&swr_opts_);
	sws_freeContext(sws_opts_);
}
