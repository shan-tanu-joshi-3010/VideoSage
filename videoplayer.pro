QT = core

CONFIG += c++17 cmdline

SOURCES += \
    ffmpeg.cpp \
    io.cpp \
    main.cpp

HEADERS += \
    ffmpeg.h \
    io.h

# Include paths (use += so they stack!)
INCLUDEPATH += C:/Users/nikku/vcpkg/packages/crow_x64-windows/include
INCLUDEPATH += C:/Users/nikku/vcpkg/packages/asio_x64-windows/include
INCLUDEPATH += C:/sqllite
LIBS += -L C:/sqllite -lsqlite3

# Libraries
LIBS += -lws2_32 -lmswsock
