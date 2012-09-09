#-------------------------------------------------
#
# Project created by QtCreator 2012-09-08T14:31:07
#
#-------------------------------------------------

QT       += core gui network

TARGET = SharedPainter
TEMPLATE = app

DEFINES += _CRT_SECURE_NO_WARNINGS

win32{
QMAKE_CXXFLAGS_DEBUG += -wd4100 -wd4101
QMAKE_CXXFLAGS_RELEASE += -wd4100 -wd4101
}

PRECOMPILED_HEADER = SharedPainter/stdafx.h

SOURCES +=  \
    SharedPainter/TextItemDialog.cpp \
    SharedPainter/stdafx.cpp \
    SharedPainter/SharedPaintManager.cpp \
    SharedPainter/SharedPainterScene.cpp \
    SharedPainter/SharedPainter.cpp \
    SharedPainter/SharedPaintCommand.cpp \
    SharedPainter/SettingManager.cpp \
    SharedPainter/PaintSession.cpp \
    SharedPainter/PaintItem.cpp \
    SharedPainter/PacketSlicer.cpp \
    SharedPainter/PacketBuffer.cpp \
    SharedPainter/main.cpp \
    SharedPainter/DefferedCaller.cpp \

HEADERS  += \
    SharedPainter/WindowPacketBuilder.h \
    SharedPainter/TextItemDialog.h \
    SharedPainter/SystemPacketBuilder.h \
    SharedPainter/stdafx.h \
    SharedPainter/Singleton.h \
    SharedPainter/SharedPaintPolicy.h \
    SharedPainter/SharedPaintManager.h \
    SharedPainter/SharedPaintManagementData.h \
    SharedPainter/SharedPainterScene.h \
    SharedPainter/SharedPainter.h \
    SharedPainter/SharedPaintCommandManager.h \
    SharedPainter/SharedPaintCommand.h \
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
    SharedPainter/NetPeerServer.h \
    SharedPainter/NetPacketData.h \
    SharedPainter/NetBroadCastSession.h \
    SharedPainter/INetPeerEvent.h \
    SharedPainter/DefferedCaller.h \
    SharedPainter/CommonPacketBuilder.h \
    SharedPainter/BroadCastPacketBuilder.h \

INCLUDEPATH += $(BOOST_DIR)

CONFIG(debug, debug|release) {
LIBS += -L$(BOOST_DIR)/lib
} else {
LIBS += -L$(BOOST_DIR)/lib
}

win32{
LIBS += -luser32 -lshell32 -lgdi32
}

FORMS    += \
    SharedPainter/textitemdialog.ui \
    SharedPainter/sharedpainter.ui

RESOURCES += \
    SharedPainter/sharedpainter.qrc
