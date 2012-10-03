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
    PainterListWindow.cpp

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
    PainterListWindow.h

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

INCLUDEPATH += $(BOOST_DIR)
LIBS += -L$(BOOST_DIR)/lib

win32{
LIBS += -luser32 -lshell32 -lgdi32
}

macx{
LIBS += -lboost_thread -lboost_system
}
