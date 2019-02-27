#-------------------------------------------------
#
# Project created by QtCreator 2019-02-27T18:03:17
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = flv_player
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

win32 {
    THIRD_PARTY_INSTALL_PREFIX = D:

    INCLUDEPATH += $${THIRD_PARTY_INSTALL_PREFIX}/fdk-aac/include \
        $${THIRD_PARTY_INSTALL_PREFIX}/openh264/include \
        $${THIRD_PARTY_INSTALL_PREFIX}/SDL2-2.0.9/include
    LIBS += -L$${THIRD_PARTY_INSTALL_PREFIX}/fdk-aac/lib -lfdk-aac-2 \
        -L$${THIRD_PARTY_INSTALL_PREFIX}/openh264/lib -lopenh264 \
        -L$${THIRD_PARTY_INSTALL_PREFIX}/SDL2-2.0.9/lib/x86 -lSDL2
}

macx {
    # mac only
}

unix:!macx {
    CONFIG(debug, debug|release) {
        THIRD_PARTY_INSTALL_PREFIX = /opt/third_party/debug
    } else {
        THIRD_PARTY_INSTALL_PREFIX = /opt/third_party/release
    }

    INCLUDEPATH += $${THIRD_PARTY_INSTALL_PREFIX}/fdk_aac/include \
        $${THIRD_PARTY_INSTALL_PREFIX}/openh264/include \
        $${THIRD_PARTY_INSTALL_PREFIX}/sdl/include
    LIBS += -L$${THIRD_PARTY_INSTALL_PREFIX}/fdk_aac/lib -lfdk-aac \
        -L$${THIRD_PARTY_INSTALL_PREFIX}/openh264/lib -lopenh264 \
        -L$${THIRD_PARTY_INSTALL_PREFIX}/sdl/lib -lSDL2
}
SOURCES += \
        main.cpp \
        mainwindow.cpp \
    h264_decoder.cpp \
    aac_decoder.cpp \
    script_tag.cpp \
    audio_tag.cpp \
    video_tag.cpp \
    flv_parser.cpp \
    pcm_player.cpp \
    yuv420p_player.cpp \
    video_widget.cpp

HEADERS += \
        mainwindow.h \
    yuv420p.h \
    h264_decoder.h \
    aac_decoder.h \
    script_tag.h \
    audio_tag.h \
    video_tag.h \
    flv_parser.h \
    pcm_player.h \
    yuv420p_player.h \
    video_widget.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    shaders.qrc
