#-------------------------------------------------
#
# Project created by QtCreator 2012-09-08T14:31:07
#
#-------------------------------------------------

QT       += core gui network

TARGET = SharedPainter
TEMPLATE = app

CONFIG   += console precompile_header

DEFINES += _CRT_SECURE_NO_WARNINGS

SOURCES +=  \
    SharedPainter/stdafx.cpp \
    SharedPainter/SharedPaintManager.cpp \
    SharedPainter/SharedPaintCommandManager.cpp \
    SharedPainter/SharedPainterScene.cpp \
    SharedPainter/SharedPainter.cpp \
    SharedPainter/SharedPaintCommand.cpp \
    SharedPainter/SharedPaintTask.cpp \
    SharedPainter/SettingManager.cpp \
    SharedPainter/PacketBuffer.cpp \
    SharedPainter/DefferedCaller.cpp \
    SharedPainter/Util.cpp \
    SharedPainter/TextItemDialog.cpp \
    SharedPainter/FindingServerDialog.cpp \
    SharedPainter/SyncDataProgressDialog.cpp \
    SharedPainter/PreferencesDialog.cpp \
    SharedPainter/UpgradeWindow.cpp \
    SharedPainter/UpgradeManager.cpp \
    SharedPainter/AboutWindow.cpp \
    SharedPainter/main.cpp \
    SharedPainter/JoinerListWindow.cpp \
    SharedPainter/PainterListWindow.cpp

HEADERS  += \
    SharedPainter/stdafx.h \
    SharedPainter/PreferencesDialog.h \
    SharedPainter/UpgradeWindow.h \
    SharedPainter/FindingServerDialog.h \
    SharedPainter/WindowPacketBuilder.h \
    SharedPainter/TextItemDialog.h \
    SharedPainter/SyncDataProgressDialog.h \
    SharedPainter/AboutWindow.h \
    SharedPainter/SystemPacketBuilder.h \
    SharedPainter/UdpPacketBuilder.h \
    SharedPainter/UpgradeManager.h \
    SharedPainter/Singleton.h \
    SharedPainter/SharedPaintPolicy.h \
    SharedPainter/SharedPaintManager.h \
    SharedPainter/SharedPaintManagementData.h \
    SharedPainter/SharedPainterScene.h \
    SharedPainter/SharedPainter.h \
    SharedPainter/SharedPaintCommandManager.h \
    SharedPainter/SharedPaintCommand.h \
    SharedPainter/SharedPaintTask.h \
    SharedPainter/SettingManager.h \
    SharedPainter/resource.h \
    SharedPainter/PaintUser.h \
    SharedPainter/PaintSession.h \
    SharedPainter/PaintPacketBuilder.h \
    SharedPainter/PaintItemFactory.h \
    SharedPainter/PaintItem.h \
    SharedPainter/PacketSlicer.h \
    SharedPainter/PacketCodeDefine.h \
    SharedPainter/PacketBuffer.h \
    SharedPainter/NetServiceRunner.h \
    SharedPainter/NetPeerSession.h \
    SharedPainter/NetUdpSession.h \
    SharedPainter/NetPeerServer.h \
    SharedPainter/NetPacketData.h \
    SharedPainter/NetBroadCastSession.h \
    SharedPainter/INetPeerEvent.h \
    SharedPainter/DefferedCaller.h \
    SharedPainter/CommonPacketBuilder.h \
    SharedPainter/BroadCastPacketBuilder.h \
    SharedPainter/TaskPacketBuilder.h \
    SharedPainter/UIStyleSheet.h \
    SharedPainter/JoinerListWindow.h \
    SharedPainter/PainterListWindow.h

FORMS    += \
    SharedPainter/FindingServerDialog.ui \
    SharedPainter/SyncDataProgressDialog.ui \
    SharedPainter/AboutWindow.ui \
    SharedPainter/textitemdialog.ui \
    SharedPainter/PreferencesDialog.ui \
    SharedPainter/UpgradeWindow.ui \
    SharedPainter/sharedpainter.ui \
    SharedPainter/JoinerListWindow.ui \
    SharedPainter/PainterListWindow.ui

RESOURCES += \
    SharedPainter/sharedpainter.qrc


win32{
RC_FILE = SharedPainter/SharedPainter.rc
QMAKE_CXXFLAGS_DEBUG += -wd4100 -wd4101
QMAKE_CXXFLAGS_RELEASE += -wd4100 -wd4101
}

PRECOMPILED_HEADER = SharedPainter/stdafx.h

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
