#-------------------------------------------------
#
# Project created by QtCreator 2013-10-09T01:15:52
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cg_task_1
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    modelviewer.cpp \
    objmodel.cpp

HEADERS  += \
    mainwindow.h \
    modelviewer.h \
    objmodel.h

win32 {
    LIBS += -L"D:/libs/glew-1.10.0/lib/"
    INCLUDEPATH += D:/libs/glew-1.10.0/include
}

LIBS += -lGLEW

OTHER_FILES += \
    vertexShader.vsh \
    fragmentShader.fsh

RESOURCES += \
    shaders.qrc
