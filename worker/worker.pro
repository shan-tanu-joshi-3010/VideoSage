QT += core network

CONFIG += console c++17
CONFIG -= app_bundle

TARGET = worker

SOURCES += \
    worker.cpp \
    ../ffmpeg.cpp \
    ../io.cpp

HEADERS += \
    ../ffmpeg.h \
    ../io.h

INCLUDEPATH += \
    .. \
    ../crow/include \
    ../asio/asio/include \
    /usr/include/librdkafka

LIBS += \
    -lrdkafka++ \
    -lrdkafka \
    -lsqlite3 \
    -lssl \
    -lcrypto