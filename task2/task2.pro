#-------------------------------------------------
#
# Project created by QtCreator 2013-10-29T14:40:57
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = task2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    objmodel.cpp \
    modelviewer.cpp \
    colorpicker.cpp

HEADERS  += mainwindow.h \
    objmodel.h \
    modelviewer.h \
    colorpicker.h

win32 {
    LIBS += -L"D:/libs/glew-1.10.0/lib/"
    INCLUDEPATH += D:/libs/glew-1.10.0/include
}

LIBS += -lGLEW

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    vertexShader.vsh \
    fragmentShader.fsh
