#include "stdafx.h"
#include "ffmpegwrapper.h"

ffmpegwrapper::ffmpegwrapper()
{
}

void ffmpegwrapper::init( void )
{
	avcodec_register_all();
}
