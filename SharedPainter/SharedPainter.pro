#-------------------------------------------------
#
# Project created by QtCreator 2012-09-08T14:31:07
#
#-------------------------------------------------

QT       += core gui network

TARGET = SharedPainter
TEMPLATE = app

CONFIG   += precompile_header

DEFINES += _CRT_SECURE_NO_WARNINGS

SOURCES +=  \
    stdafx.cpp \
    SharedPaintManager.cpp \
    SharedPaintCommandManager.cpp \
    SharedPainterScene.cpp \
    SharedPainter.cpp \
    SharedPaintCommand.cpp \
    SharedPaintTask.cpp \
    SettingManager.cpp \
    PacketBuffer.cpp \
    DefferedCaller.cpp \
    Util.cpp \
    TextItemDialog.cpp \
    FindingServerDialog.cpp \
    SyncDataProgressDialog.cpp \
    PreferencesDialog.cpp \
    UpgradeWindow.cpp \
    UpgradeManager.cpp \
    AboutWindow.cpp \
    main.cpp \
    JoinerListWindow.cpp \
    PainterListWindow.cpp \
    ScreenRecoder.cpp \
    ffmpegwrapper.cpp

HEADERS  += \
    stdafx.h \
    PreferencesDialog.h \
    UpgradeWindow.h \
    FindingServerDialog.h \
    WindowPacketBuilder.h \
    TextItemDialog.h \
    SyncDataProgressDialog.h \
    AboutWindow.h \
    SystemPacketBuilder.h \
    UdpPacketBuilder.h \
    UpgradeManager.h \
    Singleton.h \
    SharedPaintPolicy.h \
    SharedPaintManager.h \
    SharedPaintManagementData.h \
    SharedPainterScene.h \
    SharedPainter.h \
    SharedPaintCommandManager.h \
    SharedPaintCommand.h \
    SharedPaintTask.h \
    SettingManager.h \
    resource.h \
    PaintUser.h \
    PaintSession.h \
    PaintPacketBuilder.h \
    PaintItemFactory.h \
    PaintItem.h \
    PacketSlicer.h \
    PacketCodeDefine.h \
    PacketBuffer.h \
    NetServiceRunner.h \
    NetPeerSession.h \
    NetUdpSession.h \
    NetPeerServer.h \
    NetPacketData.h \
    NetBroadCastSession.h \
    INetPeerEvent.h \
    DefferedCaller.h \
    CommonPacketBuilder.h \
    BroadCastPacketBuilder.h \
    TaskPacketBuilder.h \
    UIStyleSheet.h \
    JoinerListWindow.h \
    PainterListWindow.h \
    ScreenRecoder.h \
    ffmpegwrapper.h

FORMS    += \
    FindingServerDialog.ui \
    SyncDataProgressDialog.ui \
    AboutWindow.ui \
    textitemdialog.ui \
    PreferencesDialog.ui \
    UpgradeWindow.ui \
    sharedpainter.ui \
    JoinerListWindow.ui \
    PainterListWindow.ui

RESOURCES += \
    sharedpainter.qrc


win32{
RC_FILE = SharedPainter.rc
QMAKE_CXXFLAGS_DEBUG += -wd4100 -wd4101
QMAKE_CXXFLAGS_RELEASE += -wd4100 -wd4101
}

PRECOMPILED_HEADER = stdafx.h

macx{
QMAKE_CXXFLAGS_WARN_ON = ""
QMAKE_CXXFLAGS += -Wno-unused-variable -Wno-unused-parameter -Wno-reorder
}

INCLUDEPATH += $(BOOST_DIR) ../Compatibility
LIBS += -L$(BOOST_DIR)/lib

win32{
LIBS += -luser32 -lshell32 -lgdi32
}

macx{
LIBS += -lboost_thread -lboost_system
}



# ##############################################################################
# ##############################################################################
# FFMPEG: START OF CONFIGURATION BELOW ->
# Copy these lines into your own project
# Make sure to set the path variables for:
# 1) QTFFmpegWrapper sources (i.e. where the QVideoEncoder.cpp and QVideoDecoder.cpp lie),
# 2) FFMPEG include path (i.e. where the directories libavcodec, libavutil, etc. lie),
# 3) the binary FFMPEG libraries (that must be compiled separately).
# Under Linux path 2 and 3 may not need to be set as these are usually in the standard include and lib path.
# Under Windows, path 2 and 3 must be set to the location where you placed the FFMPEG includes and compiled binaries
# Note that the FFMPEG dynamic librairies (i.e. the .dll files) must be in the PATH
# ##############################################################################
# ##############################################################################

# ##############################################################################
# Modify here: set FFMPEG_LIBRARY_PATH and FFMPEG_INCLUDE_PATH
# ##############################################################################

# Set QTFFMPEGWRAPPER_SOURCE_PATH to point to the directory containing the QTFFmpegWrapper sources
#QTFFMPEGWRAPPER_SOURCE_PATH = ../ffmpeg/QTFFmpegWrapper

# Set FFMPEG_LIBRARY_PATH to point to the directory containing the FFmpeg import libraries (if needed - typically for Windows), i.e. the dll.a files
FFMPEG_LIBRARY_PATH = ../ffmpeg/lib

# Set FFMPEG_INCLUDE_PATH to point to the directory containing the FFMPEG includes (if needed - typically for Windows)
#FFMPEG_INCLUDE_PATH = ../ffmpeg/QTFFmpegWrapper
FFMPEG_INCLUDE_PATH = ../ffmpeg/include

# ##############################################################################
# Do not modify: FFMPEG default settings
# ##############################################################################
# Sources for QT wrapper
#SOURCES += $$QTFFMPEGWRAPPER_SOURCE_PATH/QVideoEncoder.cpp \
#   $$QTFFMPEGWRAPPER_SOURCE_PATH/QVideoDecoder.cpp
#HEADERS += $$QTFFMPEGWRAPPER_SOURCE_PATH/QVideoEncoder.h \
#    $$QTFFMPEGWRAPPER_SOURCE_PATH/QVideoDecoder.h

# Set list of required FFmpeg libraries
LIBS += -lavutil \
    -lavcodec \
    -lavformat \
    -lswscale

# Add the path
LIBS += -L$$FFMPEG_LIBRARY_PATH
INCLUDEPATH += QVideoEncoder
INCLUDEPATH += $$FFMPEG_INCLUDE_PATH

# Requied for some C99 defines
DEFINES += __STDC_CONSTANT_MACROS

# ##############################################################################
# FFMPEG: END OF CONFIGURATION
# ##############################################################################
