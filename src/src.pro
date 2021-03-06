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
    finddialog.cpp \
    filenameresolver.cpp \
    spinpreprocessor.cpp \
    spinsourcefactory.cpp \
    spincodeparser.cpp \
    spincompletionsource.cpp \
    searchengine.cpp \
    searchscope.cpp \
    searchable.cpp \
    shortcutedit.cpp \
    shortcuts.cpp \
    groupactiondialog.cpp \
    eserialportmanager.cpp \
    eserialportproxy.cpp
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
    finddialog.h \
    filenameresolver.h \
    spinpreprocessor.h \
    spinsourcefactory.h \
    spincodelexer.h \
    spincodeparser.h \
    spincompletionsource.h \
    searchengine.h \
    searchscope.h \
    searchable.h \
    shortcutedit.h \
    shortcuts.h \
    groupactiondialog.h \
    eserialportmanager.h \
    eserialportproxy.h
FORMS += aboutdialog.ui \
    preferencesdialog.ui \
    finddialog.ui \
    groupactiondialog.ui
LIBS += -lqscintilla2
RESOURCES += PZST.qrc
unix:SOURCES += eserialport_posix.cpp
unix:!macx:SOURCES += eserialport_nonosx.cpp
unix:!macx:INCLUDEPATH += /usr/lib/dbus-1.0/include
unix:!macx:INCLUDEPATH += /usr/include/dbus-1.0
macx:SOURCES += eserialport_osx.cpp
macx:LIBS += -framework \
    IOKit
macx:LIBS += -framework \
    CoreFoundation
win32:SOURCES += eserialport_win.cpp
win32:LIBS += -lsetupapi
TRANSLATIONS = pzst_ru_RU.ts
QMAKE_CXXFLAGS_DEBUG += -pg
QMAKE_LFLAGS_DEBUG += -pg
MAKE_SPIN_LEXER_INPUT = spincodelexer.re
make_spin_lexer.commands = re2c \
    -s \
    -o \
    spincodelexer.cpp \
    --case-insensitive \
    ${QMAKE_FILE_IN}
make_spin_lexer.input = MAKE_SPIN_LEXER_INPUT
make_spin_lexer.output = spincodelexer.cpp
make_spin_lexer.variable_out = SOURCES
make_spin_lexer.name = make_spin_lexer
QMAKE_EXTRA_COMPILERS += make_spin_lexer
OTHER_FILES += spincodelexer.re \
    sample.spin
target.path = /usr/bin
INSTALLS += target
translation.files = pzst_ru_RU.qm
translation.path = /usr/share/pzst/lang
INSTALLS += translation
bstc.files = bstc.pzst
bstc.path = /usr/bin
INSTALLS += bstc
menu.files = pzst.desktop
menu.path = /usr/share/applications
INSTALLS += menu
pixmap.files = icons/pzst.png
pixmap.path = /usr/share/pixmaps
INSTALLS += pixmap
manru.files = ../manual/ru/build/man/pzst.1
manru.path = /usr/share/man/ru/man1
INSTALLS += manru
manen.files = ../manual/en/build/man/pzst.1
manen.path = /usr/share/man/man1
INSTALLS += manen
RC_FILE = pzst.rc
