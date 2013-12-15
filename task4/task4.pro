#-------------------------------------------------
#
# Project created by QtCreator 2013-12-11T17:00:16
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = task4
TEMPLATE = app

win32 {
    LIBS += -L"D:/libs/glew-1.10.0/lib/"
    INCLUDEPATH += D:/libs/glew-1.10.0/include
}

LIBS += -lGLEW

SOURCES += \
    modelviewer.cpp \
    mainwindow.cpp \
    main.cpp \
    colorpicker.cpp \
    terrain.cpp

HEADERS  += \
    modelviewer.h \
    mainwindow.h \
    colorpicker.h \
    terrain.h

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    vertexShader.vsh \
    fragmentShader.fsh \
    boxVS.vsh \
    boxFS.fsh \
    particleFS.fsh \
    particleVS.vsh \
    particleGS.gsh \
    terrainVS.vsh \
    terrainFS.fsh

