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
    signal_center.cpp \
    video_widget.cpp \
    pcm_player.cpp \
    yuv420p_player.cpp \
    file/file_parsers.cpp \
    file/flv/flv_base.cpp \
    file/flv/flv_parser.cpp \
    file/flv/metadata_tag.cpp \
    file/flv/audio_tag.cpp \
    file/flv/video_tag.cpp \
    audio/audio_decoders.cpp \
    audio/aac/aac_decoder.cpp \
    audio/aac/fdkaac_dec.cpp \
    video/video_decoders.cpp \
    video/h264/h264_decoder.cpp \
    pcm_device.cpp

HEADERS += \
    mainwindow.h \
    byte_util.h \
    singleton.h \
    signal_center.h \
    yuv420p.h \
    pcm.h \
    video_widget.h \
    yuv420p_player.h \
    pcm_player.h \
    file/file_parsers.h \
    file/flv/flv_base.h \
    file/flv/flv_parser.h \
    file/flv/metadata_tag.h \
    file/flv/audio_tag.h \
    file/flv/video_tag.h \
    audio/audio_decoders.h \
    audio/aac/aac_decoder.h \
    video/video_decoders.h \
    video/h264/h264_decoder.h \
    pcm_device.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    shaders.qrc
