#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H

#include "ffmpegutil.h"
typedef std::vector<std::string> stringlist_t;

using namespace ffmpegutil;

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
	void clear( void );

	int _transcode_init( void );
	int _transcode_step( void );
	int _output_packet(boost::shared_ptr<InputStream> ist, const AVPacket *pkt);
	void _flush_encoders(void);
	bool _add_input_streams( AVFormatContext *ic );
	int _init_input_stream( boost::shared_ptr<InputStream> ist, char *error, int error_len );
	int _need_output( void );
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

private:
	std::vector< boost::shared_ptr<InputStream> > input_stream_list_;
	std::vector< boost::shared_ptr<InputFile> > input_file_list_;
	std::vector< boost::shared_ptr<OutputStream> > output_stream_list_;
	std::vector< boost::shared_ptr<OutputFile> > output_file_list_;
	std::vector< boost::shared_ptr<FilterGraph> > filter_list_;
	std::string input_frame_rate_;
	std::string output_frame_rate_;
	std::string frame_size_;
	double quality_;
	int video_sync_method_;
	std::string outputFileName_;
	int64_t extra_size_;
	bool do_deinterlace_;
	bool stop_;
	AVDictionary *codec_opts_;
	struct SwsContext *sws_opts_;
	SwrContext *swr_opts_;
};

#endif // FFMPEGWRAPPER_H
