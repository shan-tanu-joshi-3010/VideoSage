QT += core network
CONFIG += c++17 console
CONFIG -= app_bundle

SOURCES += \
    ffmpeg.cpp \
    io.cpp \
    main.cpp \
    kafka/KafkaProducer.cpp

HEADERS += \
    ffmpeg.h \
    io.h \ 
    kafka/KafkaProducer.h


unix {
    # Crow
    INCLUDEPATH += $$PWD/crow/include
    INCLUDEPATH += $$PWD/asio/asio/include
    INCLUDEPATH += /usr/include/librdkafka

    # SQLite
    LIBS += -lsqlite3

    # OpenSSL
    LIBS += -lssl
    LIBS += -lcrypto

    # Kafka
    LIBS += -lrdkafka++
    LIBS += -lrdkafka

    # Threads
    LIBS += -lpthread
}

win32 {
    # Crow
    INCLUDEPATH += C:/Users/nikku/vcpkg/packages/crow_x64-windows/include

    # Asio
    INCLUDEPATH += C:/Users/nikku/vcpkg/packages/asio_x64-windows/include

    # SQLite
    INCLUDEPATH += C:/sqllite
    LIBS += -L C:/sqllite -lsqlite3

    # Windows Networking
    LIBS += -lws2_32 -lmswsock

    # OpenSSL (adjust path if using vcpkg)
    LIBS += -lssl -lcrypto
}