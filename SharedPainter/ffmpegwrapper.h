#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H

#include "ffmpegutil.h"
typedef std::vector<std::string> stringlist_t;

using namespace ffmpegutil;

class ffmpegwrapper
{
public:
	static void initialize()
	{
		initializeLib();
	}

	ffmpegwrapper();

	void setRecordTime( int second );
	void setOutputQuality( int quality );
	void setInputFormatName( const std::string &name );
	void setInputFileName( const std::string &name );
	void setOutputCodecName( const std::string &name );
	void setOutputFileName( const std::string &name );

	bool videoCapture( void )
	{
		init();

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

		clear();
		return true;
	}

	void stop( void )
	{
		stop_ = true;
	}

private:
	void init( void );
	bool openInputDevice( void );
	bool openOutputDevice( void );
	bool transcode( void );
	void clear( void );

	int _transcode_init( void );
	int _transcode_step( void );
	int _output_packet(boost::shared_ptr<InputStream> ist, const AVPacket *pkt);
	void _flush_encoders(void);
	bool _add_input_streams( AVFormatContext *ic );
	int _init_input_stream( boost::shared_ptr<InputStream> ist, char *error, int error_len );
	int _need_output( void );
	void _close_output_stream( OutputStream * ost );
	void _close_output_stream( boost::shared_ptr<OutputStream> ost );
	boost::shared_ptr<FilterGraph> _init_simple_filtergraph( boost::shared_ptr<InputStream> ist, boost::shared_ptr<OutputStream> ost );
	int _configure_filtergraph( boost::shared_ptr<FilterGraph> fg );
	bool _init_input_filter( boost::shared_ptr<FilterGraph> fg, AVFilterInOut *in );
	int _sub2video_prepare( InputStream *ist );
	int _configure_input_video_filter( FilterGraph *fg, InputFilter *ifilter, AVFilterInOut *in );
	int _configure_input_filter( FilterGraph *fg, InputFilter *ifilter, AVFilterInOut *in );
	int _configure_output_filter( FilterGraph *fg, OutputFilter *ofilter, AVFilterInOut *out );
	int _configure_output_video_filter( FilterGraph *fg, OutputFilter *ofilter, AVFilterInOut *out );

	boost::shared_ptr<OutputStream> _add_output_streams( AVFormatContext *oc, enum AVMediaType type, int source_index );
	boost::shared_ptr<OutputStream> _add_video_streams( AVFormatContext *oc, int source_index );
	boost::shared_ptr<InputStream> _get_input_stream( boost::shared_ptr<OutputStream> ost );

	int _decode_video(boost::shared_ptr<InputStream> ist, AVPacket *pkt, int *got_output);
	void _do_streamcopy(boost::shared_ptr<InputStream> ist, boost::shared_ptr<OutputStream> ost, const AVPacket *pkt);
	void _rate_emu_sleep(boost::shared_ptr<InputStream> ist);
	int _check_output_constraints(boost::shared_ptr<InputStream> ist, boost::shared_ptr<OutputStream> ost);

	boost::shared_ptr<OutputStream> _choose_output( void );
	int _do_video_out(AVFormatContext *s, boost::shared_ptr<OutputStream> ost, AVFrame *in_picture, float quality);
	int _reap_filters(void);
	int _check_recording_time(boost::shared_ptr<OutputStream> ost);
	bool _got_eagain(void);
	void _reset_eagain(void);
	InputStream * _transcode_from_filter(FilterGraph *graph, int *resultCode);
	int _process_input(int file_index);
	int _get_input_packet(boost::shared_ptr<InputFile> f, AVPacket *pkt);
	void _report_new_stream(int input_index, AVPacket *pkt);
	void _sub2video_heartbeat(boost::shared_ptr<InputStream> ist, int64_t pts);
	void _clearAllResource();

private:
	bool stop_;

	std::vector< boost::shared_ptr<InputStream> > input_stream_list_;
	std::vector< boost::shared_ptr<InputFile> > input_file_list_;
	std::vector< boost::shared_ptr<OutputStream> > output_stream_list_;
	std::vector< boost::shared_ptr<OutputFile> > output_file_list_;
	std::vector< boost::shared_ptr<FilterGraph> > filter_list_;

	// option
	std::string input_frame_rate_;
	std::string output_frame_rate_;
	std::string frame_size_;
	double quality_;
	int video_sync_method_;
	int audio_sync_method_;
	std::string inputFormatName_;
	std::string inputFileName_;
	std::string outputFileName_;
	std::string outputCodecName_;
	int64_t extra_size_;
	bool do_deinterlace_;
	AVDictionary *codec_opts_;
	struct SwsContext *sws_opts_;
	SwrContext *swr_opts_;
	int64_t video_size_;
	int64_t audio_size_;
	int64_t subtitle_size_;
	int same_quant_;
	int nb_frames_drop_;
	float dts_error_threshold_;
	int nb_frames_dup_;
	float dts_delta_threshold_;
	bool copy_ts_;
	uint64_t limit_filesize_;
	int64_t recording_time_;
	int64_t start_time_;
	int rate_emu_;
	int64_t input_ts_offset_;
	bool metadata_global_manual_;
};

#endif // FFMPEGWRAPPER_H
