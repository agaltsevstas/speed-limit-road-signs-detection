TEMPLATE = app
TARGET = speed-limit-road-signs-detection
INCLUDEPATH += include
CONFIG += debug
CONFIG += c++11
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11

DESTDIR = $$PWD/bin/

#Отключить "теневую сборку" в криейторе!
CONFIG(release, debug|release) {

message(Project $$TARGET (Release))

OBJECTS_DIR = build/
MOC_DIR = build/
RCC_DIR = build/
UI_DIR = build/
}
CONFIG(debug, debug|release) {

message(Project $$TARGET (Debug))

OBJECTS_DIR = build/
MOC_DIR = build/
RCC_DIR = build/
UI_DIR = build/
DEFINES += DEBUG_BUILD
}

HEADERS += include/hough_circles.h
SOURCES += src/hough_circles.cpp \
           src/main.cpp

INCLUDEPATH += /usr/local/include
LIBS += -L/usr/local/lib \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_video \
        -lopencv_videoio \
        -lopencv_imgcodecs \
        -lopencv_tracking \
        -lopencv_text

LIBS += -L/usr/lib/x86_64-linux-gnu \
        -lboost_filesystem \
        -lboost_system \
        -lboost_log \
        -lboost_log_setup \
        -lboost_thread \
        -lpthread \
        -ldl -fPIC

LIBS += -L/usr/lib \
        -llept \
        -ltesseract
