#if defined(WIN32)

#include <stdio.h>
#include <string.h>
#include "libavutil/pixfmt.h"
#include "libavutil/pixdesc.h"

const AVPixFmtDescriptor av_pix_fmt_descriptors[PIX_FMT_NB] = {

    /*[PIX_FMT_YUV420P] = */{
        /* .name = */"yuv420p",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
		/* .flags = */PIX_FMT_PLANAR,
        /* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_YUYV422] = */{
        /* .name = */"yuyv422",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
		/* .flags = */0,
        /* .comp = */{
            { 0, 1, 1, 0, 7 },        /* Y */
            { 0, 3, 2, 0, 7 },        /* U */
            { 0, 3, 4, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_RGB24] = */{
        /* .name = */"rgb24",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
		/* .flags = */PIX_FMT_RGB,
        /* .comp = */{
            { 0, 2, 1, 0, 7 },        /* R */
            { 0, 2, 2, 0, 7 },        /* G */
            { 0, 2, 3, 0, 7 },        /* B */
        },
    },
    /*[PIX_FMT_BGR24] = */{
        /* .name = */"bgr24",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
		/* .flags = */PIX_FMT_RGB,
        /* .comp = */{
            { 0, 2, 3, 0, 7 },        /* R */
            { 0, 2, 2, 0, 7 },        /* G */
            { 0, 2, 1, 0, 7 },        /* B */
        },
    },
    /*[PIX_FMT_YUV422P] = */{
        /* .name = */"yuv422p",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
		/* .flags = */PIX_FMT_PLANAR,
        /* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_YUV444P] = */{
        /* .name = */"yuv444p",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_YUV410P] = */{
        /* .name = */"yuv410p",
        /* .nb_components = */3,
        /* .log2_chroma_w = */2,
        /* .log2_chroma_h = */2,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_YUV411P] = */{
        /* .name = */"yuv411p",
        /* .nb_components = */3,
        /* .log2_chroma_w = */2,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_GRAY8] = */{
        /* .name = */"gray",
        /* .nb_components = */1,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PSEUDOPAL,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
        },
    },
    /*[PIX_FMT_MONOWHITE] = */{
        /* .name = */"monow",
        /* .nb_components = */1,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BITSTREAM,
		/* .comp = */{
            { 0, 0, 1, 0, 0 },        /* Y */
        },
    },
    /*[PIX_FMT_MONOBLACK] = */{
        /* .name = */"monob",
        /* .nb_components = */1,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BITSTREAM,
		/* .comp = */{
            { 0, 0, 1, 7, 0 },        /* Y */
        },
    },
    /*[PIX_FMT_PAL8] = */{
        /* .name = */"pal8",
        /* .nb_components = */1,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PAL,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },
        },
    },
    /*[PIX_FMT_YUVJ420P] = */{
        /* .name = */"yuvj420p",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_YUVJ422P] = */{
        /* .name = */"yuvj422p",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_YUVJ444P] = */{
        /* .name = */"yuvj444p",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_XVMC_MPEG2_MC] = */{
        /* .name = */"xvmcmc",
        /* .nb_components = */0,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_HWACCEL,
    },
    /*[PIX_FMT_XVMC_MPEG2_IDCT] = */{
        /* .name = */"xvmcidct",
		/* .nb_components = */0,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_HWACCEL,
    },
    /*[PIX_FMT_UYVY422] = */{
        /* .name = */"uyvy422",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
		/* .flags = */0,
        /* .comp = */{
            { 0, 1, 2, 0, 7 },        /* Y */
            { 0, 3, 1, 0, 7 },        /* U */
            { 0, 3, 3, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_UYYVYY411] = */{
        /* .name = */"uyyvyy411",
        /* .nb_components = */3,
        /* .log2_chroma_w = */2,
        /* .log2_chroma_h = */0,
		/* .flags = */0,
        /* .comp = */{
            { 0, 3, 2, 0, 7 },        /* Y */
            { 0, 5, 1, 0, 7 },        /* U */
            { 0, 5, 4, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_BGR8] = */{
        /* .name = */"bgr8",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
		/* .flags = */PIX_FMT_RGB | PIX_FMT_PSEUDOPAL,
        /* .comp = */{
            { 0, 0, 1, 0, 2 },        /* R */
            { 0, 0, 1, 3, 2 },        /* G */
            { 0, 0, 1, 6, 1 },        /* B */
        },
    },
    /*[PIX_FMT_BGR4] = */{
        /* .name = */"bgr4",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
		/* .flags = */PIX_FMT_BITSTREAM | PIX_FMT_RGB,
        /* .comp = */{
            { 0, 3, 4, 0, 0 },        /* R */
            { 0, 3, 2, 0, 1 },        /* G */
            { 0, 3, 1, 0, 0 },        /* B */
        },
    },
    /*[PIX_FMT_BGR4_BYTE] = */{
        /* .name = */"bgr4_byte",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
		/* .flags = */PIX_FMT_RGB | PIX_FMT_PSEUDOPAL,
        /* .comp = */{
            { 0, 0, 1, 0, 0 },        /* R */
            { 0, 0, 1, 1, 1 },        /* G */
            { 0, 0, 1, 3, 0 },        /* B */
        },
    },
    /*[PIX_FMT_RGB8] = */{
        /* .name = */"rgb8",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
		/* .flags = */PIX_FMT_RGB | PIX_FMT_PSEUDOPAL,
        /* .comp = */{
            { 0, 0, 1, 6, 1 },        /* R */
            { 0, 0, 1, 3, 2 },        /* G */
            { 0, 0, 1, 0, 2 },        /* B */
        },
    },
    /*[PIX_FMT_RGB4] = */{
        /* .name = */"rgb4",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BITSTREAM | PIX_FMT_RGB,
		/* .comp = */{
            { 0, 3, 1, 0, 0 },        /* R */
            { 0, 3, 2, 0, 1 },        /* G */
            { 0, 3, 4, 0, 0 },        /* B */
        },
    },
    /*[PIX_FMT_RGB4_BYTE] = */{
        /* .name = */"rgb4_byte",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB | PIX_FMT_PSEUDOPAL,
		/* .comp = */{
            { 0, 0, 1, 3, 0 },        /* R */
            { 0, 0, 1, 1, 1 },        /* G */
            { 0, 0, 1, 0, 0 },        /* B */
        },
    },
    /*[PIX_FMT_NV12] = */{
        /* .name = */"nv12",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 1, 1, 0, 7 },        /* U */
            { 1, 1, 2, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_NV21] = */{
        /* .name = */"nv21",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 1, 2, 0, 7 },        /* U */
            { 1, 1, 1, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_ARGB] = */{
        /* .name = */"argb",
        /* .nb_components = */4,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB,
		/* .comp = */{
            { 0, 3, 2, 0, 7 },        /* R */
            { 0, 3, 3, 0, 7 },        /* G */
            { 0, 3, 4, 0, 7 },        /* B */
            { 0, 3, 1, 0, 7 },        /* A */
        },
    },
    /*[PIX_FMT_RGBA] = */{
        /* .name = */"rgba",
        /* .nb_components = */4,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB,
		/* .comp = */{
            { 0, 3, 1, 0, 7 },        /* R */
            { 0, 3, 2, 0, 7 },        /* G */
            { 0, 3, 3, 0, 7 },        /* B */
            { 0, 3, 4, 0, 7 },        /* A */
        },
    },
    /*[PIX_FMT_ABGR] = */{
        /* .name = */"abgr",
        /* .nb_components = */4,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB,
		/* .comp = */{
            { 0, 3, 4, 0, 7 },        /* R */
            { 0, 3, 3, 0, 7 },        /* G */
            { 0, 3, 2, 0, 7 },        /* B */
            { 0, 3, 1, 0, 7 },        /* A */
        },
    },
    /*[PIX_FMT_BGRA] = */{
        /* .name = */"bgra",
        /* .nb_components = */4,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB,
		/* .comp = */{
            { 0, 3, 3, 0, 7 },        /* R */
            { 0, 3, 2, 0, 7 },        /* G */
            { 0, 3, 1, 0, 7 },        /* B */
            { 0, 3, 4, 0, 7 },        /* A */
        },
    },
    /*[PIX_FMT_0RGB] = */{
        /* .name = */"0rgb",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB,
		/* .comp = */{
            { 0, 3, 2, 0, 7 },        /* R */
            { 0, 3, 3, 0, 7 },        /* G */
            { 0, 3, 4, 0, 7 },        /* B */
        },
    },
    /*[PIX_FMT_RGB0] = */{
        /* .name = */"rgb0",
        /* .nb_components= */3,
        /* .log2_chroma_w = */ 0,
        /* .log2_chroma_h = */ 0,
        /* .flags = */PIX_FMT_RGB,
		/* .comp = */{
            { 0, 3, 1, 0, 7 },        /* R */
            { 0, 3, 2, 0, 7 },        /* G */
            { 0, 3, 3, 0, 7 },        /* B */
            { 0, 3, 4, 0, 7 },        /* A */
        },
    },
    /*[PIX_FMT_0BGR] = */{
        /* .name = */"0bgr",
        /* .nb_components= */3,
        /* .log2_chroma_w = */ 0,
        /* .log2_chroma_h = */ 0,
        /* .flags = */PIX_FMT_RGB,
		/* .comp = */{
            { 0, 3, 4, 0, 7 },        /* R */
            { 0, 3, 3, 0, 7 },        /* G */
            { 0, 3, 2, 0, 7 },        /* B */
        },
    },
    /*[PIX_FMT_BGR0] = */{
        /* .name = */"bgr0",
        /* .nb_components= */3,
        /* .log2_chroma_w = */ 0,
        /* .log2_chroma_h = */ 0,
        /* .flags = */PIX_FMT_RGB,
		/* .comp = */{
            { 0, 3, 3, 0, 7 },        /* R */
            { 0, 3, 2, 0, 7 },        /* G */
            { 0, 3, 1, 0, 7 },        /* B */
            { 0, 3, 4, 0, 7 },        /* A */
        },
    },
    /*[PIX_FMT_GRAY16BE] = */{
        /* .name = */"gray16be",
        /* .nb_components = */1,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE,
		/* .comp = */{
            { 0, 1, 1, 0, 15 },       /* Y */
        },
    },
    /*[PIX_FMT_GRAY16LE] = */{
        /* .name = */"gray16le",
        /* .nb_components = */1,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
		/* .flags = */0,
        /* .comp = */{
            { 0, 1, 1, 0, 15 },       /* Y */
        },
    },
    /*[PIX_FMT_YUV440P] = */{
        /* .name = */"yuv440p",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_YUVJ440P] = */{
        /* .name = */"yuvj440p",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
        },
    },
    /*[PIX_FMT_YUVA420P] = */{
        /* .name = */"yuva420p",
        /* .nb_components = */4,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
            { 3, 0, 1, 0, 7 },        /* A */
        },
    },
    /*[PIX_FMT_YUVA422P] = */{
        /* .name = */"yuva422p",
        /* .nb_components = */4,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
            { 3, 0, 1, 0, 7 },        /* A */
        },
    },
    /*[PIX_FMT_YUVA444P] = */{
        /* .name = */"yuva444p",
        /* .nb_components = */4,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 0, 1, 0, 7 },        /* Y */
            { 1, 0, 1, 0, 7 },        /* U */
            { 2, 0, 1, 0, 7 },        /* V */
            { 3, 0, 1, 0, 7 },        /* A */
        },
    },
    /*[PIX_FMT_VDPAU_H264] = */{
        /* .name = */"vdpau_h264",
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_HWACCEL,
    },
    /*[PIX_FMT_VDPAU_MPEG1] = */{
        /* .name = */"vdpau_mpeg1",
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_HWACCEL,
    },
    /*[PIX_FMT_VDPAU_MPEG2] = */{
        /* .name = */"vdpau_mpeg2",
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_HWACCEL,
    },
    /*[PIX_FMT_VDPAU_WMV3] = */{
        /* .name = */"vdpau_wmv3",
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_HWACCEL,
    },
    /*[PIX_FMT_VDPAU_VC1] = */{
        /* .name = */"vdpau_vc1",
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_HWACCEL,
    },
    /*[PIX_FMT_VDPAU_MPEG4] = */{
        /* .name = */"vdpau_mpeg4",
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_HWACCEL,
    },
    /*[PIX_FMT_RGB48BE] = */{
        /* .name = */"rgb48be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB | PIX_FMT_BE,
		/* .comp = */{
            { 0, 5, 1, 0, 15 },       /* R */
            { 0, 5, 3, 0, 15 },       /* G */
            { 0, 5, 5, 0, 15 },       /* B */
        },
    },
    /*[PIX_FMT_RGB48LE] = */{
        /* .name = */"rgb48le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB,
		/* .comp = */{
            { 0, 5, 1, 0, 15 },       /* R */
            { 0, 5, 3, 0, 15 },       /* G */
            { 0, 5, 5, 0, 15 },       /* B */
        },
    },
    /*[PIX_FMT_RGBA64BE] = */{
        /* .name = */"rgba64be",
        /* .nb_components= */4,
        /* .log2_chroma_w = */ 0,
        /* .log2_chroma_h = */ 0,
        /* .flags = */PIX_FMT_RGB | PIX_FMT_BE,
		/* .comp = */{
            { 0, 7, 1, 0, 15 },       /* R */
            { 0, 7, 3, 0, 15 },       /* G */
            { 0, 7, 5, 0, 15 },       /* B */
            { 0, 7, 7, 0, 15 },       /* A */
        },
    },
    /*[PIX_FMT_RGBA64LE] = */{
        /* .name = */"rgba64le",
        /* .nb_components= */4,
        /* .log2_chroma_w = */ 0,
        /* .log2_chroma_h = */ 0,
        /* .flags = */PIX_FMT_RGB,
		/* .comp = */{
            { 0, 7, 1, 0, 15 },       /* R */
            { 0, 7, 3, 0, 15 },       /* G */
            { 0, 7, 5, 0, 15 },       /* B */
            { 0, 7, 7, 0, 15 },       /* A */
        },
    },
    /*[PIX_FMT_RGB565BE] = */{
        /* .name = */"rgb565be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_RGB,
		/* .comp = */{
            { 0, 1, 0, 3, 4 },        /* R */
            { 0, 1, 1, 5, 5 },        /* G */
            { 0, 1, 1, 0, 4 },        /* B */
        },
    },
    /*[PIX_FMT_RGB565LE] = */{
        /* .name = */"rgb565le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB,
		/* .comp = */{
            { 0, 1, 2, 3, 4 },        /* R */
            { 0, 1, 1, 5, 5 },        /* G */
            { 0, 1, 1, 0, 4 },        /* B */
        },
    },
    /*[PIX_FMT_RGB555BE] = */{
        /* .name = */"rgb555be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_RGB,
		/* .comp = */{
            { 0, 1, 0, 2, 4 },        /* R */
            { 0, 1, 1, 5, 4 },        /* G */
            { 0, 1, 1, 0, 4 },        /* B */
        },
    },
    /*[PIX_FMT_RGB555LE] = */{
        /* .name = */"rgb555le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB,
		/* .comp = */{
            { 0, 1, 2, 2, 4 },        /* R */
            { 0, 1, 1, 5, 4 },        /* G */
            { 0, 1, 1, 0, 4 },        /* B */
        },
    },
    /*[PIX_FMT_RGB444BE] = */{
        /* .name = */"rgb444be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_RGB,
 		/* .comp = */{
            { 0, 1, 0, 0, 3 },        /* R */
            { 0, 1, 1, 4, 3 },        /* G */
            { 0, 1, 1, 0, 3 },        /* B */
        },
    },
    /*[PIX_FMT_RGB444LE] = */{
        /* .name = */"rgb444le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB,
 		/* .comp = */{
            { 0, 1, 2, 0, 3 },        /* R */
            { 0, 1, 1, 4, 3 },        /* G */
            { 0, 1, 1, 0, 3 },        /* B */
        },
    },
    /*[PIX_FMT_BGR48BE] = */{
        /* .name = */"bgr48be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_RGB,
    	/* .comp = */{
            { 0, 5, 5, 0, 15 },       /* R */
            { 0, 5, 3, 0, 15 },       /* G */
            { 0, 5, 1, 0, 15 },       /* B */
        },
    },
    /*[PIX_FMT_BGR48LE] = */{
        /* .name = */"bgr48le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB,
  		/* .comp = */{
            { 0, 5, 5, 0, 15 },       /* R */
            { 0, 5, 3, 0, 15 },       /* G */
            { 0, 5, 1, 0, 15 },       /* B */
        },
    },
    /*[PIX_FMT_BGRA64BE] = */{
        /* .name = */"bgra64be",
        /* .nb_components= */4,
        /* .log2_chroma_w = */ 0,
        /* .log2_chroma_h = */ 0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_RGB,
   		/* .comp = */{
            { 0, 7, 5, 0, 15 },       /* R */
            { 0, 7, 3, 0, 15 },       /* G */
            { 0, 7, 1, 0, 15 },       /* B */
            { 0, 7, 7, 0, 15 },       /* A */
        },
    },
    /*[PIX_FMT_BGRA64LE] = */{
        /* .name = */"bgra64le",
        /* .nb_components= */4,
        /* .log2_chroma_w = */ 0,
        /* .log2_chroma_h = */ 0,
		/* .flags = */PIX_FMT_RGB,
   		/* .comp = */{
            { 0, 7, 5, 0, 15 },       /* R */
            { 0, 7, 3, 0, 15 },       /* G */
            { 0, 7, 1, 0, 15 },       /* B */
            { 0, 7, 7, 0, 15 },       /* A */
        },
    },
    /*[PIX_FMT_BGR565BE] = */{
        /* .name = */"bgr565be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_RGB,
  		/* .comp = */{
            { 0, 1, 1, 0, 4 },        /* R */
            { 0, 1, 1, 5, 5 },        /* G */
            { 0, 1, 0, 3, 4 },        /* B */
        },
    },
    /*[PIX_FMT_BGR565LE] = */{
        /* .name = */"bgr565le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB,
  		/* .comp = */{
            { 0, 1, 1, 0, 4 },        /* R */
            { 0, 1, 1, 5, 5 },        /* G */
            { 0, 1, 2, 3, 4 },        /* B */
        },
    },
    /*[PIX_FMT_BGR555BE] = */{
        /* .name = */"bgr555be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_RGB,
 		/* .comp = */{
            { 0, 1, 1, 0, 4 },       /* R */
            { 0, 1, 1, 5, 4 },       /* G */
            { 0, 1, 0, 2, 4 },       /* B */
        },
     },
    /*[PIX_FMT_BGR555LE] = */{
        /* .name = */"bgr555le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB,
  		/* .comp = */{
            { 0, 1, 1, 0, 4 },        /* R */
            { 0, 1, 1, 5, 4 },        /* G */
            { 0, 1, 2, 2, 4 },        /* B */
        },
    },
    /*[PIX_FMT_BGR444BE] = */{
        /* .name = */"bgr444be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_RGB,
 		/* .comp = */{
            { 0, 1, 1, 0, 3 },       /* R */
            { 0, 1, 1, 4, 3 },       /* G */
            { 0, 1, 0, 0, 3 },       /* B */
        },
     },
    /*[PIX_FMT_BGR444LE] = */{
        /* .name = */"bgr444le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_RGB,
 		/* .comp = */{
            { 0, 1, 1, 0, 3 },        /* R */
            { 0, 1, 1, 4, 3 },        /* G */
            { 0, 1, 2, 0, 3 },        /* B */
        },
    },
    /*[PIX_FMT_VAAPI_MOCO] = */{
        /* .name = */"vaapi_moco",
		/* .nb_components = */0,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_HWACCEL,
    },
    /*[PIX_FMT_VAAPI_IDCT] = */{
        /* .name = */"vaapi_idct",
		/* .nb_components = */0,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_HWACCEL,
    },
    /*[PIX_FMT_VAAPI_VLD] = */{
        /* .name = */"vaapi_vld",
		/* .nb_components = */0,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_HWACCEL,
    },
    /*[PIX_FMT_YUV420P9LE] = */{
        /* .name = */"yuv420p9le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
        },
    },
    /*[PIX_FMT_YUV420P9BE] = */{
        /* .name = */"yuv420p9be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
        },
    },
    /*[PIX_FMT_YUV420P10LE] = */{
        /* .name = */"yuv420p10le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
        },
    },
    /*[PIX_FMT_YUV420P10BE] = */{
        /* .name = */"yuv420p10be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
        },
    },
    /*[PIX_FMT_YUV420P12LE] = */{
        /* .name = */"yuv420p12le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 11 },        /* Y */
            { 1, 1, 1, 0, 11 },        /* U */
            { 2, 1, 1, 0, 11 },        /* V */
        },
    },
    /*[PIX_FMT_YUV420P12BE] = */{
        /* .name = */"yuv420p12be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 11 },        /* Y */
            { 1, 1, 1, 0, 11 },        /* U */
            { 2, 1, 1, 0, 11 },        /* V */
        },
    },
    /*[PIX_FMT_YUV420P14LE] = */{
        /* .name = */"yuv420p14le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_PLANAR,
  		/* .comp = */{
            { 0, 1, 1, 0, 13 },        /* Y */
            { 1, 1, 1, 0, 13 },        /* U */
            { 2, 1, 1, 0, 13 },        /* V */
        },
    },
    /*[PIX_FMT_YUV420P14BE] = */{
        /* .name = */"yuv420p14be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
  		/* .comp = */{
            { 0, 1, 1, 0, 13 },        /* Y */
            { 1, 1, 1, 0, 13 },        /* U */
            { 2, 1, 1, 0, 13 },        /* V */
        },
    },
    /*[PIX_FMT_YUV420P16LE] = */{
        /* .name = */"yuv420p16le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_PLANAR,
 		/* .comp = */{
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
        },
    },
    /*[PIX_FMT_YUV420P16BE] = */{
        /* .name = */"yuv420p16be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
  		/* .comp = */{
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
        },
    },
    /*[PIX_FMT_YUV422P9LE] = */{
        /* .name = */"yuv422p9le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
  		/* .comp = */{
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
        },
    },
    /*[PIX_FMT_YUV422P9BE] = */{
        /* .name = */"yuv422p9be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
  		/* .comp = */{
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
        },
    },
    /*[PIX_FMT_YUV422P10LE] = */{
        /* .name = */"yuv422p10le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
 		/* .comp = */{
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
        },
    },
    /*[PIX_FMT_YUV422P10BE] = */{
        /* .name = */"yuv422p10be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
 		/* .comp = */{
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
        },
    },
    /*[PIX_FMT_YUV422P12LE] = */{
        /* .name = */"yuv422p12le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
  		/* .comp = */{
            { 0, 1, 1, 0, 11 },        /* Y */
            { 1, 1, 1, 0, 11 },        /* U */
            { 2, 1, 1, 0, 11 },        /* V */
        },
    },
    /*[PIX_FMT_YUV422P12BE] = */{
        /* .name = */"yuv422p12be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
 		/* .comp = */{
            { 0, 1, 1, 0, 11 },        /* Y */
            { 1, 1, 1, 0, 11 },        /* U */
            { 2, 1, 1, 0, 11 },        /* V */
        },
    },
    /*[PIX_FMT_YUV422P14LE] = */{
        /* .name = */"yuv422p14le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
 		/* .comp = */{
            { 0, 1, 1, 0, 13 },        /* Y */
            { 1, 1, 1, 0, 13 },        /* U */
            { 2, 1, 1, 0, 13 },        /* V */
        },
    },
    /*[PIX_FMT_YUV422P14BE] = */{
        /* .name = */"yuv422p14be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 13 },        /* Y */
            { 1, 1, 1, 0, 13 },        /* U */
            { 2, 1, 1, 0, 13 },        /* V */
        },
    },
    /*[PIX_FMT_YUV422P16LE] = */{
        /* .name = */"yuv422p16le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
        },
    },
    /*[PIX_FMT_YUV422P16BE] = */{
        /* .name = */"yuv422p16be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
        },
    },
    /*[PIX_FMT_YUV444P16LE] = */{
        /* .name = */"yuv444p16le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
        },
    },
    /*[PIX_FMT_YUV444P16BE] = */{
        /* .name = */"yuv444p16be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 15 },        /* Y */
            { 1, 1, 1, 0, 15 },        /* U */
            { 2, 1, 1, 0, 15 },        /* V */
        },
    },
    /*[PIX_FMT_YUV444P10LE] = */{
        /* .name = */"yuv444p10le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
        },
    },
    /*[PIX_FMT_YUV444P10BE] = */{
        /* .name = */"yuv444p10be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 9 },        /* Y */
            { 1, 1, 1, 0, 9 },        /* U */
            { 2, 1, 1, 0, 9 },        /* V */
        },
    },
    /*[PIX_FMT_YUV444P9LE] = */{
        /* .name = */"yuv444p9le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
  		/* .comp = */{
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
        },
    },
    /*[PIX_FMT_YUV444P9BE] = */{
        /* .name = */"yuv444p9be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
 		/* .comp = */{
            { 0, 1, 1, 0, 8 },        /* Y */
            { 1, 1, 1, 0, 8 },        /* U */
            { 2, 1, 1, 0, 8 },        /* V */
        },
    },
    /*[PIX_FMT_YUV444P12LE] = */{
        /* .name = */"yuv444p12le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
    	/* .comp = */{
            { 0, 1, 1, 0, 11 },        /* Y */
            { 1, 1, 1, 0, 11 },        /* U */
            { 2, 1, 1, 0, 11 },        /* V */
        },
    },
    /*[PIX_FMT_YUV444P12BE] = */{
        /* .name = */"yuv444p12be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 11 },        /* Y */
            { 1, 1, 1, 0, 11 },        /* U */
            { 2, 1, 1, 0, 11 },        /* V */
        },
    },
    /*[PIX_FMT_YUV444P14LE] = */{
        /* .name = */"yuv444p14le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 13 },        /* Y */
            { 1, 1, 1, 0, 13 },        /* U */
            { 2, 1, 1, 0, 13 },        /* V */
        },
    },
    /*[PIX_FMT_YUV444P14BE] = */{
        /* .name = */"yuv444p14be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR,
		/* .comp = */{
            { 0, 1, 1, 0, 13 },        /* Y */
            { 1, 1, 1, 0, 13 },        /* U */
            { 2, 1, 1, 0, 13 },        /* V */
        },
    },
    /*[PIX_FMT_DXVA2_VLD] = */{
        /* .name = */"dxva2_vld",
		/* .nb_components = */0,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_HWACCEL,
    },
    /*[PIX_FMT_VDA_VLD] = */{
        /* .name = */"vda_vld",
		/* .nb_components = */0,
        /* .log2_chroma_w = */1,
        /* .log2_chroma_h = */1,
        /* .flags = */PIX_FMT_HWACCEL,
    },
    /*[PIX_FMT_GRAY8A] = */{
        /* .name = */"gray8a",
        /* .nb_components = */2,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */0,
		/* .comp = */{
            { 0, 1, 1, 0, 7 },        /* Y */
            { 0, 1, 2, 0, 7 },        /* A */
        },
    },
    /*[PIX_FMT_GBRP] = */{
        /* .name = */"gbrp",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR | PIX_FMT_RGB,
		/* .comp = */{
            { 2, 0, 1, 0, 7 },        /* R */
            { 0, 0, 1, 0, 7 },        /* G */
            { 1, 0, 1, 0, 7 },        /* B */
        },
    },
    /*[PIX_FMT_GBRP9LE] = */{
        /* .name = */"gbrp9le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR | PIX_FMT_RGB,
		/* .comp = */{
            { 2, 1, 1, 0, 8 },        /* R */
            { 0, 1, 1, 0, 8 },        /* G */
            { 1, 1, 1, 0, 8 },        /* B */
        },
    },
    /*[PIX_FMT_GBRP9BE] = */{
        /* .name = */"gbrp9be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_RGB,
 		/* .comp = */{
            { 2, 1, 1, 0, 8 },        /* R */
            { 0, 1, 1, 0, 8 },        /* G */
            { 1, 1, 1, 0, 8 },        /* B */
        },
    },
    /*[PIX_FMT_GBRP10LE] = */{
        /* .name = */"gbrp10le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR | PIX_FMT_RGB,
		/* .comp = */{
            { 2, 1, 1, 0, 9 },        /* R */
            { 0, 1, 1, 0, 9 },        /* G */
            { 1, 1, 1, 0, 9 },        /* B */
        },
    },
    /*[PIX_FMT_GBRP10BE] = */{
        /* .name = */"gbrp10be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_RGB,
 		/* .comp = */{
            { 2, 1, 1, 0, 9 },        /* R */
            { 0, 1, 1, 0, 9 },        /* G */
            { 1, 1, 1, 0, 9 },        /* B */
        },
    },
    /*[PIX_FMT_GBRP12LE] = */{
        /* .name = */"gbrp12le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR | PIX_FMT_RGB,
 		/* .comp = */{
            { 2, 1, 1, 0, 11 },        /* R */
            { 0, 1, 1, 0, 11 },        /* G */
            { 1, 1, 1, 0, 11 },        /* B */
        },
    },
    /*[PIX_FMT_GBRP12BE] = */{
        /* .name = */"gbrp12be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_RGB,
		/* .comp = */{
            { 2, 1, 1, 0, 11 },        /* R */
            { 0, 1, 1, 0, 11 },        /* G */
            { 1, 1, 1, 0, 11 },        /* B */
        },
    },
    /*[PIX_FMT_GBRP14LE] = */{
        /* .name = */"gbrp14le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR | PIX_FMT_RGB,
		/* .comp = */{
            { 2, 1, 1, 0, 13 },        /* R */
            { 0, 1, 1, 0, 13 },        /* G */
            { 1, 1, 1, 0, 13 },        /* B */
        },
    },
    /*[PIX_FMT_GBRP14BE] = */{
        /* .name = */"gbrp14be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_RGB,
		/* .comp = */{
            { 2, 1, 1, 0, 13 },        /* R */
            { 0, 1, 1, 0, 13 },        /* G */
            { 1, 1, 1, 0, 13 },        /* B */
        },
    },
    /*[PIX_FMT_GBRP16LE] = */{
        /* .name = */"gbrp16le",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_PLANAR | PIX_FMT_RGB,
		/* .comp = */{
            { 2, 1, 1, 0, 15 },       /* R */
            { 0, 1, 1, 0, 15 },       /* G */
            { 1, 1, 1, 0, 15 },       /* B */
        },
    },
    /*[PIX_FMT_GBRP16BE] = */{
        /* .name = */"gbrp16be",
        /* .nb_components = */3,
        /* .log2_chroma_w = */0,
        /* .log2_chroma_h = */0,
        /* .flags = */PIX_FMT_BE | PIX_FMT_PLANAR | PIX_FMT_RGB,
		/* .comp = */{
            { 2, 1, 1, 0, 15 },       /* R */
            { 0, 1, 1, 0, 15 },       /* G */
            { 1, 1, 1, 0, 15 },       /* B */
        },
    },
};

#endif