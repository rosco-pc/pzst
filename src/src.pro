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
    searchengine.cpp \
    searchable.cpp \
    filenameresolver.cpp \
    spinpreprocessor.cpp \
    spinsourcefactory.cpp \
    spincodeparser.cpp \
    spincompletionsource.cpp
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
    searchengine.h \
    searchable.h \
    filenameresolver.h \
    spinpreprocessor.h \
    spinsourcefactory.h \
    spincodelexer.h \
    spincodeparser.h \
    spincompletionsource.h
FORMS += aboutdialog.ui \
    preferencesdialog.ui \
    finddialog.ui
LIBS += -lqscintilla2
RESOURCES += PZST.qrc
unix:SOURCES += eserialport_posix.cpp
unix:!macx:SOURCES += eserialport_nonosx.cpp
macx:SOURCES += eserialport_osx.cpp
win32:SOURCES += eserialport_win.cpp
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
OTHER_FILES += spincodelexer.re
