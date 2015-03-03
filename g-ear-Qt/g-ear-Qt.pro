#-------------------------------------------------
#
# Project created by QtCreator 2013-08-12T14:25:44
#
#-------------------------------------------------

QT += core gui qml quick declarative webkitwidgets network
#otherwise printf goes to /dev/null on windows. oh, wait...
#CONFIG  += console

TARGET = g-ear-Qt
TEMPLATE = app

INCLUDEPATH += .

SOURCES += main.cpp\
    App.cpp \
    QTPreferences.cpp \
    QTKeychain.cpp \
    qtquick2controlsapplicationviewer.cpp \
    qtwebwindow.cpp \
    QmlController/PlaylistController.cpp \
    QmlController/SongListController.cpp \
    QmlController/SettingPanelController.cpp \
    QmlController/QmlController.cpp \
    QmlModel/PlaylistModel.cpp \
    QmlModel/SongListModel.cpp \
    QmlModel/PlaylistCategoryModel.cpp \
    QmlModel/PlaylistModelItem.cpp \
    QmlModel/PlaylistCategoryModelItem.cpp \
    BoostFileManager.cpp \
    QmlController/MainWindowController.cpp \
    WinFeature/SystemTrayIcon.cpp \
    QmlController/PlaybackPanelController.cpp \
    Theme/Painter.cpp \
    QmlController/QmlControllerManager.cpp \
    QmlController/StatusBarController.cpp \
    Theme/IconProvider.cpp

HEADERS  += App.h \
    QTPreferences.h \
    QTKeychain.h \
    qtquick2controlsapplicationviewer.h \
    qtwebwindow.h \
    QmlController/PlaylistController.h \
    QmlController/SongListController.h \
    QmlController/SettingPanelController.h \
    QmlController/QmlController.h \
    QmlModel/ListModel.h \
    QmlModel/PlaylistModel.h \
    QmlModel/PlaylistModelItem.h \
    QmlModel/SongListModel.h \
    QmlModel/ListModelItem.h \
    QmlModel/PlaylistCategoryModel.h \
    QmlModel/PlaylistCategoryModelItem.h \
    BoostFileManager.h \
    QmlController/MainWindowController.h \
    WinFeature/SystemTrayIcon.h \
    QmlController/PlaybackPanelController.h \
    Theme/Painter.h \
    QmlController/QmlControllerManager.h \
    SharedMemoryStatusThreadWorker.h \
    QmlController/StatusBarController.h \
    Theme/IconProvider.h

INCLUDEPATH += ../include
INCLUDEPATH += ../include/core
INCLUDEPATH += ../include/core/Base
DEFINES += NON_APPLE
DEFINES += BOOST_ALL_NO_LIB
qml.files = GEarGui.qml

CONFIG += c++11
RESOURCES += GEarGui.qrc

INSTALL += qml

linux {
    QMAKE_CXXFLAGS = -std=c++11
    QMAKE_LIBDIR += ../lib/linux
    LIBS += -lcore-qt -lssl -lcrypto -lmpg123
    QMAKE_LFLAGS = -std=c++11
}

macx {
    QMAKE_MAC_SDK = macosx10.9
    QMAKE_CXXFLAGS = -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.7
    QMAKE_LFLAGS = -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.7 -L../lib/macosx -lcore-qt -lssl -lcrypto -lmpg123\
                   -lavcodec.56.1.100 -lavutil.54.7.100 -lavresample.2.1.0 -lavformat.56.4.101 -lavfilter.5.1.100 \
                   -framework Cocoa -framework Security -framework SystemConfiguration
}

win32 {
    SOURCES += PlaybackWorkerDX.cpp
    HEADERS += PlaybackWorkerDX.h

    QT += winextras

    mingw {
        LIBS += ..\lib\win\libcore.a
    } else {
        LIBS += ..\lib\win\core.lib
    }

    LIBS += ..\lib\win\Crypt32.Lib
    LIBS += ..\lib\win\avcodec-56.dll ..\lib\win\avfilter-5.dll ..\lib\win\avformat-56.dll ..\lib\win\avresample-2.dll ..\lib\win\avutil-54.dll
    LIBS += ..\lib\win\libmpg123-0.dll
    LIBS += -ldsound -ldxguid -lwinmm

    #LIBS += ..\lib\win\libssl-30.dll ..\lib\win\libcrypto-30.dll
    LIBS += ..\lib\win\ssleay32.dll ..\lib\win\libeay32.dll
    LIBS += -lws2_32 -lwsock32 -lmswsock
    LIBS += -lgdi32

    DESTDIR = ..\out
    QMAKE_POST_LINK = copy ..\lib\win\*.dll $${DESTDIR} && xcopy ..\lib\assets $${DESTDIR}\assets\ /E /y

    debug {
        #QMAKE_CXXFLAGS += /Od /Ob0
        #DEFINES += _DEBUG
    }

    RC_FILE = g-ear-Qt.rc
}
