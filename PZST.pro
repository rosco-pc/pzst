# -------------------------------------------------
# Project created by QtCreator 2010-08-10T11:31:14
# -------------------------------------------------
TARGET = pzst
TEMPLATE = app
SOURCES += mainwindow.cpp \
    main.cpp \
    spineditor.cpp \
    spinlexer.cpp \
    eserialport.cpp \
    propellerloader.cpp \
    spincompiler.cpp \
    erroritem.cpp \
    aboutdialog.cpp \
    preferencesdialog.cpp \
    pzstpreferences.cpp \
    chartable.cpp \
    termialwindow.cpp \
    terminalwidget.cpp \
    terminaldisplay.cpp \
    spinobjectlocator.cpp \
    spinparser.cpp \
    finddialog.cpp \
    searchengine.cpp \
    searchable.cpp
HEADERS += mainwindow.h \
    spineditor.h \
    spinlexer.h \
    eserialport.h \
    propellerloader.h \
    spincompiler.h \
    erroritem.h \
    aboutdialog.h \
    preferencesdialog.h \
    pzstpreferences.h \
    chartable.h \
    termialwindow.h \
    terminalwidget.h \
    terminaldisplay.h \
    spinobjectlocator.h \
    spinparser.h \
    finddialog.h \
    searchengine.h \
    searchable.h
FORMS += aboutdialog.ui \
    preferencesdialog.ui \
    terminalwidget.ui \
    finddialog.ui
LIBS += -lqscintilla2
RESOURCES += PZST.qrc
unix:SOURCES += eserialport_posix.cpp
win32:SOURCES += eserialport_win.cpp
TRANSLATIONS = pzst_ru_RU.ts
QMAKE_CXXFLAGS_DEBUG += -pg
QMAKE_LFLAGS_DEBUG += -pg
