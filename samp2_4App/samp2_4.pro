#-------------------------------------------------
#
# Project created by QtCreator 2016-11-14T00:17:13
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = samp2_4
TEMPLATE = app


SOURCES += main.cpp\
        qwmainwind.cpp

HEADERS  += qwmainwind.h

FORMS    += qwmainwind.ui

RESOURCES += \
    res.qrc

# 说明：项目路径含中文（实验1），MinGW 的 windres 无法处理中文路径下的图标，
# 会导致 debug/samp2_4_resource_res.o Error 1，故注释掉 RC_ICONS，
# 窗口图标改为在 qwmainwind.cpp 构造函数中用 setWindowIcon() 从资源加载。
# RC_ICONS = AppIcon.ico
