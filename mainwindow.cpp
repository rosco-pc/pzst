#include <QMdiSubWindow>
#include <QWidget>
#include <QFileDialog>
#include <QApplication>
#include <QCloseEvent>
#include <QMimeData>
#include <QTabBar>
#include <QSettings>
#include <QProgressBar>
#include <QDockWidget>
#include <QMessageBox>
#include <QDir>
#include <QTextCodec>
#include <Qsci/qscilexercustom.h>
#include <QScrollArea>
#include <QPainter>
#include <QLineEdit>
#include <QKeyEvent>
#include <Qsci/qsciprinter.h>
#include <QPrintDialog>
#include <QPrinter>

#include "mainwindow.h"
#include "spineditor.h"
#include "spinlexer.h"
#include "eserialport.h"
#include "propellerloader.h"
#include "spincompiler.h"
#include "erroritem.h"
#include "aboutdialog.h"
#include "preferencesdialog.h"
#include "pzstpreferences.h"

using namespace PZST;

SpinError::SpinError(QString msg, QString f, int l, int c) :message(msg), filename(f), line(l), col(c)
{
}
SpinError::SpinError() :message(""), filename(""), line(-1), col(-1)
{
}
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), findDialog(this)
{
    lastActiveWindow = 0;
    searchEngine = new SearchEngine();

    windowSwitcher = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(windowSwitcher);
    layout->setContentsMargins(5, 5, 5, 5);
    windowSwitcher->setLayout(layout);
    windowsList = new QListWidget(windowSwitcher);
    windowsList->setStyleSheet("border-color:#999;border-width:6px; border-style:ridge;");
    layout->addWidget(windowsList);
    windowSwitcher->setWindowModality(Qt::ApplicationModal);
    windowSwitcher->resize(300, 200);
    windowSwitcher->hide();

    QFont::insertSubstitution("Parallax", "Courier New");
    mapper = new QSignalMapper(this);
    mruMapper = new QSignalMapper(this);
    mdi = new QMdiArea(this);
    setCentralWidget(mdi);
    mdi->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdi->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdi->setViewMode(QMdiArea::TabbedView);
    mdi->setTabShape(QTabWidget::Triangular);
    connect(mdi, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(windowActivated(QMdiSubWindow*)));
    connect(mapper, SIGNAL(mapped(QWidget*)), this, SLOT(windowActivated(QWidget*)));
    connect(mruMapper, SIGNAL(mapped(QString)), this, SLOT(openOrActivate(QString)));

    connect(QApplication::clipboard(), SIGNAL(changed (QClipboard::Mode)), this, SLOT(clipboardChanged(QClipboard::Mode)));
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/Files/appicon.ico"), QSize(), QIcon::Normal, QIcon::Off);
    setWindowIcon(icon);
    setDocumentMode(true);

    terminal = new TermialWindow(this);

    createActions();
    createMenus();
    createStatusBar();
    createToolbars();
    createDocks();

    QSettings *s = new QSettings();
    restoreGeometry(s->value("geometry").toByteArray());
    restoreState(s->value("windowState").toByteArray());

    setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );

    setTabShape(QTabWidget::Triangular);
    windowActivated((QWidget*)0);


    connect(
        &findDialog,
        SIGNAL(searchRequested(QString,SearchEngine::SearchOptions)),
        this,
        SLOT(searchRequested(QString,SearchEngine::SearchOptions))
    );
    qRegisterMetaType<SearchEngine::Result>("SearchEngine::Result");
    connect(searchEngine, SIGNAL(foundInTarget(QString,SearchEngine::Result,bool)), this, SLOT(foundInTarget(QString,SearchEngine::Result,bool)));

    QApplication::instance()->installEventFilter(this);
}
QAction* MainWindow::createAction(QString text, QString seq, QString iconFile, bool inMenu)
{
    QAction *act = new QAction(text, this);
    act->setShortcut(QKeySequence(seq));
    act->setIcon(QIcon(iconFile));
    act->setIconVisibleInMenu(inMenu);
    return act;
}

void MainWindow::createActions()
{
    actNew = createAction(tr("New"), "Ctrl+N", ":/Icons/new.png", true);
    connect(actNew, SIGNAL(triggered()), this, SLOT(newDocument()));

    actOpen = createAction(tr("Open ..."), "Ctrl+O", ":/Icons/open.png", true);
    connect(actOpen, SIGNAL(triggered()), this, SLOT(openDocument()));

    actClose = createAction(tr("Close"), "Ctrl+W", ":/Icons/close.png", true);
    connect(actClose, SIGNAL(triggered()), this, SLOT(closeWindow()));

    actCloseAll = createAction(tr("Close all"), "Ctrl+Shift+W");
    connect(actCloseAll, SIGNAL(triggered()), this, SLOT(closeWindowAll()));

    actSave = createAction(tr("Save"), "Ctrl+S", ":/Icons/save.png", true);
    connect(actSave, SIGNAL(triggered()), this, SLOT(saveDocument()));

    actSaveAs = createAction(tr("Save as ..."), "Shift+Ctrl+S", ":/Icons/saveas.png", true);
    connect(actSaveAs, SIGNAL(triggered()), this, SLOT(saveDocumentAs()));

    actPrint = createAction(tr("Print ..."), "Ctrl+P", ":/Icons/print.png", true);
    connect(actPrint, SIGNAL(triggered()), this, SLOT(printDocument()));

    actQuit = createAction(tr("Quit"), "Ctrl+Q", ":/Icons/exit.png", true);
    connect(actQuit, SIGNAL(triggered()), this, SLOT(close()));

    actCut = createAction(tr("Cut"), "Ctrl+X", ":/Icons/cut.png", true);
    connect(actCut, SIGNAL(triggered()), this, SLOT(cutSelection()));

    actCopy = createAction(tr("Copy"), "Ctrl+C", ":/Icons/copy.png", true);
    connect(actCopy, SIGNAL(triggered()), this, SLOT(copySelection()));

    actPaste = createAction(tr("Paste"), "Ctrl+V", ":/Icons/paste.png", true);
    connect(actPaste, SIGNAL(triggered()), this, SLOT(pasteClipboard()));

    actUndo = createAction(tr("Undo"), "Ctrl+Z", ":/Icons/undo.png", true);
    connect(actUndo, SIGNAL(triggered()), this, SLOT(undo()));

    actRedo = createAction(tr("Redo"), "Ctrl+Y", ":/Icons/redo.png", true);
    connect(actRedo, SIGNAL(triggered()), this, SLOT(redo()));

    actDetectProp = new QAction(tr("Detect Propeller"), this);
    actDetectProp->setShortcut(QKeySequence("F7"));
    connect(actDetectProp, SIGNAL(triggered()), this, SLOT(detectProp()));

    actCompile = new QAction(tr("Compile"), this);
    actCompile->setShortcut(QKeySequence("F9"));
    connect(actCompile, SIGNAL(triggered()), this, SLOT(compile()));

    actLoadRAM = new QAction(tr("Compile and load RAM"), this);
    actLoadRAM->setShortcut(QKeySequence("F10"));
    connect(actLoadRAM, SIGNAL(triggered()), this, SLOT(compileToRAM()));

    actLoadEEPROM = new QAction(tr("Compile and load EEPROM"), this);
    actLoadEEPROM->setShortcut(QKeySequence("F11"));
    connect(actLoadEEPROM, SIGNAL(triggered()), this, SLOT(compileToEEPROM()));

    actSaveBIN = new QAction(tr("Compile and save BINARY file"), this);
    connect(actSaveBIN, SIGNAL(triggered()), this, SLOT(compileSaveBINARY()));

    actSaveEEPROM = new QAction(tr("Compile and save EEPROM file"), this);
    connect(actSaveEEPROM, SIGNAL(triggered()), this, SLOT(compileSaveEEPROM()));

    actAbout = new QAction(tr("About PZST ..."), this);
    connect(actAbout, SIGNAL(triggered()), this, SLOT(about()));

    actPreferences = createAction(tr("Preferences ..."), "", ":/Icons/configure.png", true);
    connect(actPreferences, SIGNAL(triggered()), this, SLOT(preferences()));

    actTerminal = new QAction(tr("Terminal"), this);
    actTerminal->setEnabled(false);
    actTerminal->setCheckable(true);
    connect(actTerminal, SIGNAL(triggered(bool)), terminal, SLOT(setVisible(bool)));
    connect(terminal, SIGNAL(visibilityChanged(bool)), actTerminal, SLOT(setChecked(bool)));

    actFind = createAction(tr("Search/Replace"), "Ctrl+F", ":/Icons/find.png", true);
    connect(actFind, SIGNAL(triggered()), this, SLOT(find()));

    actFindNext = createAction(tr("Find next"), "Ctrl+G", "", true);
    connect(actFindNext, SIGNAL(triggered()), this, SLOT(findNext()));

    actReplace = createAction(tr("Replace and find next"), "Ctrl+R", "", true);
    actReplace->setEnabled(false);
    connect(actReplace, SIGNAL(triggered()), this, SLOT(replace()));

    actComplete= createAction(tr("Autocomplete"), "Ctrl+Space", "", true);
    connect(actComplete, SIGNAL(triggered()), this, SLOT(autoComplete()));
    actComplete->setEnabled(false);

    actCallTip = createAction(tr("Call tip"), "Ctrl+Shift+Space", "", true);
    connect(actCallTip, SIGNAL(triggered()), this, SLOT(callTip()));
    actCallTip->setEnabled(false);

    actFold = createAction(tr("Fold"), "Ctrl+<", "", true);
    connect(actFold, SIGNAL(triggered()), this, SLOT(fold()));
    actFold->setEnabled(false);

    actUnfold = createAction(tr("Unfold"), "Ctrl+>", "", true);
    connect(actUnfold, SIGNAL(triggered()), this, SLOT(unfold()));
    actUnfold->setEnabled(false);

}
void MainWindow::createMenus()
{
    menuFile = menuBar()->addMenu(tr("&File"));
    menuEdit = menuBar()->addMenu(tr("&Edit"));
    menuCompile = menuBar()->addMenu(tr("&Compile"));
    menuWindow = menuBar()->addMenu(tr("&Window"));
    menuHelp = menuBar()->addMenu(tr("&Help"));
    connect(menuWindow, SIGNAL(aboutToShow()), this, SLOT(rebuildWindowMenu()));

    menuFile->addAction(actNew);
    menuFile->addAction(actOpen);
    menuMRU = menuFile->addMenu(tr("Recent files"));
    connect(menuMRU, SIGNAL(aboutToShow()), this, SLOT(rebuildMRUMenu()));
    menuFile->addSeparator();
    menuFile->addAction(actSave);
    menuFile->addAction(actSaveAs);
    menuFile->addSeparator();
    menuFile->addAction(actClose);
    menuFile->addAction(actCloseAll);
    menuFile->addSeparator();
    menuFile->addAction(actPrint);
    menuFile->addSeparator();
    menuFile->addAction(actQuit);

    menuEdit->addAction(actUndo);
    menuEdit->addAction(actRedo);
    menuEdit->addSeparator();
    menuEdit->addAction(actCut);
    menuEdit->addAction(actCopy);
    menuEdit->addAction(actPaste);
    menuEdit->addSeparator();
    menuEdit->addAction(actFind);
    menuEdit->addAction(actFindNext);
    menuEdit->addAction(actReplace);
    menuEdit->addSeparator();
    menuEdit->addAction(actComplete);
    menuEdit->addAction(actCallTip);
    menuEdit->addSeparator();
    //menuEdit->addAction(actFold);
    //menuEdit->addAction(actUnfold);
    //menuEdit->addSeparator();
    menuEdit->addAction(actPreferences);

    menuCompile->addAction(actDetectProp);
    menuCompile->addSeparator();
    menuCompile->addAction(actCompile);
    menuCompile->addAction(actLoadRAM);
    menuCompile->addAction(actLoadEEPROM);
    menuCompile->addSeparator();
    menuCompile->addAction(actSaveBIN);
    menuCompile->addAction(actSaveEEPROM);

    menuHelp->addAction(actAbout);
}
void MainWindow::createToolbars()
{
    editToolBar = new QToolBar(tr("Editor toolbar"));
    methodsListCombo = new QComboBox(this);
    methodsListCombo->setFocusPolicy(Qt::NoFocus);
    methodsListCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    editToolBar->setObjectName("editToolBar");

    editToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);

    editToolBar->addAction(actCopy);
    editToolBar->addAction(actCut);
    editToolBar->addAction(actPaste);
    editToolBar->addAction(actUndo);
    editToolBar->addAction(actRedo);

    editToolBar->addSeparator();

    editToolBar->addWidget(new QLabel(tr(" Method:")));
    editToolBar->addWidget(methodsListCombo);
    connect(methodsListCombo, SIGNAL(activated(int)), this, SLOT(methodChosen(int)));

    addToolBar(editToolBar);
}

void MainWindow::createStatusBar()
{
    QStatusBar *sb = statusBar();
    statusFileName = new QLabel(sb);
    statusPosition = new QLabel(sb);
    statusMessage = new QLabel(sb);
    statusMessage->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    uploadProgress = new QProgressBar(sb);
    uploadProgress->hide();
    uploadMessage = new QLabel(sb);
    uploadMessage->setStyleSheet("color:blue; font-weight:bold;");
    uploadMessage->hide();
    sb->addWidget(statusFileName);
    sb->addWidget(statusPosition);
    sb->addWidget(uploadProgress, 2);
    sb->addWidget(uploadMessage);
    sb->addWidget(statusMessage, 2);
    connect(sb, SIGNAL(messageChanged(QString)), this, SLOT(messageChanged(QString)));
}
void MainWindow::createDocks()
{
    setDockOptions(AnimatedDocks | ForceTabbedDocks | VerticalTabs);

    errorsDock = new QDockWidget(tr("Errors"), this);
    errorsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, errorsDock);
    errorsList = new QListWidget(errorsDock);
    errorsDock->setWidget(errorsList);
    errorsDock->setObjectName("errorsDock");
    connect(errorsList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(showError(QListWidgetItem*)));

    infoDock = new QDockWidget(tr("Compiler information"), this);
    infoDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    infoDock->setObjectName("infoDock");
    codeInfoBrowser = new QTextBrowser(this);
    infoDock->setWidget(codeInfoBrowser);
    addDockWidget(Qt::BottomDockWidgetArea, infoDock, Qt::Horizontal);

    charTable = new CharTable(this);
    charTable->resize(300, 400);
    charTableDock = new QDockWidget(tr("Character table"), this);
    charTableDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    charTableDock->setObjectName("charTableDock");
    QScrollArea *sa = new QScrollArea(this);
    sa->setWidget(charTable);
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sa->setWidgetResizable(true);
    sa->setFocusPolicy(Qt::NoFocus);
    charTableDock->setWidget(sa);
    addDockWidget(Qt::BottomDockWidgetArea, charTableDock, Qt::Horizontal);
    connect(charTable, SIGNAL(charSelected(QChar)), this, SLOT(charSelected(QChar)));
    connect(charTable, SIGNAL(charHighlighted(QChar)), this, SLOT(charHighlighted(QChar)));

    searchResultsDock = new QDockWidget(tr("Search results"), this);
    searchResultsDock->setObjectName("searchResultsDock");
    searchTree = new QTreeWidget(this);
    searchTree->setHeaderHidden(true);
    searchResultsDock->setWidget(searchTree);
    addDockWidget(Qt::BottomDockWidgetArea, searchResultsDock);
    connect(searchEngine, SIGNAL(searchStarted(bool)), this, SLOT(searchStarted(bool)));
    connect(searchEngine, SIGNAL(searchFinished(bool)), this, SLOT(searchFinished(bool)));
    connect(searchTree, SIGNAL(activated(QModelIndex)), this, SLOT(searchTreeClicked(QModelIndex)));
}
void MainWindow::newDocument()
{
    SpinEditor *e = new SpinEditor(this);
    QMdiSubWindow *w = mdi->addSubWindow(e);
    w->setSystemMenu(0);
    w->show();
    w->setAttribute(Qt::WA_DeleteOnClose);
    connectEditor(e);
    checkClipboard();
    searchEngine->registerSearchable(e);
    connect(e, SIGNAL(closed(SpinEditor*)), this, SLOT(editorClosed(SpinEditor*)));
}
void MainWindow::openDocument()
{
    QFileDialog dlg(this, tr("Open file"), "", tr("SPIN source (*.spin)"));
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setDefaultSuffix("spin");
    if (dlg.exec() != QDialog::Accepted) return;
    QString fName = dlg.selectedFiles().at(0);
    if (!fName.isEmpty()) {
        openOrActivate(QDir::toNativeSeparators(fName));
    }

}
void MainWindow::windowActivated(QMdiSubWindow *w)
{
    windowActivated((QWidget*)w);
}
void MainWindow::windowActivated(QWidget *w)
{
    windowSwitcher->hide();
    bool hasEditor = false;
    actSave->setEnabled(false);
    actSaveAs->setEnabled(false);
    if (!w) {
        w = mdi->currentSubWindow();
    }
    if (w) {
        if (w != lastActiveWindow) actReplace->setEnabled(false);
        lastActiveWindow = w;
        QMdiSubWindow *sw = qobject_cast<QMdiSubWindow *>(w);
        QWidget *widget = sw->widget();
        mdi->setActiveSubWindow(sw);
        SpinEditor *e = qobject_cast<SpinEditor *>(widget);
        if (e) {
            hasEditor = true;
            statusFileName->setText(e->getFileName());
            statusFileName->show();
            int newPos = e->SendScintilla(QsciScintillaBase::SCI_GETCURRENTPOS);
            int line = e->SendScintilla(QsciScintillaBase::SCI_LINEFROMPOSITION, newPos);
            int col = e->SendScintilla(QsciScintillaBase::SCI_GETCOLUMN, newPos);
            actSave->setEnabled(e->isModified() || !e->hasFilename());
            actSaveAs->setEnabled(e->hasFilename());
            actCut->setEnabled(e->hasSelectedText());
            actCopy->setEnabled(e->hasSelectedText());
            actUndo->setEnabled(e->isUndoAvailable());
            actRedo->setEnabled(e->isRedoAvailable());
            actCompile->setEnabled(true);
            actLoadRAM->setEnabled(true);
            actLoadEEPROM->setEnabled(true);
            actSaveBIN->setEnabled(true);
            actSaveEEPROM->setEnabled(true);
            actComplete->setEnabled(true);
            actFold->setEnabled(true);
            actUnfold->setEnabled(true);
            actCallTip->setEnabled(true);
            actPrint->setEnabled(true);
            actClose->setEnabled(true);
            actCloseAll->setEnabled(true);
            actFind->setEnabled(true);
            actFindNext->setEnabled(!searchSettings.text.isEmpty());
            methodsListChanged(e->getMethodDefs());
            methodsListCombo->setEnabled(true);
            updateCursorPosition(line, col);
        }
    }
    if (!hasEditor) {
        statusFileName->hide();
        statusPosition->hide();
        actSave->setEnabled(false);
        actCut->setEnabled(false);
        actCopy->setEnabled(false);
        actPaste->setEnabled(false);
        actUndo->setEnabled(false);
        actRedo->setEnabled(false);
        actCompile->setEnabled(false);
        actLoadRAM->setEnabled(false);
        actLoadEEPROM->setEnabled(false);
        actSaveBIN->setEnabled(false);
        actSaveEEPROM->setEnabled(false);
        actPrint->setEnabled(false);
        actClose->setEnabled(false);
        actCloseAll->setEnabled(false);
        actComplete->setEnabled(false);
        actCallTip->setEnabled(false);
        actFold->setEnabled(false);
        actUnfold->setEnabled(false);
        actFind->setEnabled(false);
        actFindNext->setEnabled(false);
        methodsListCombo->clear();
        methodsListCombo->setEnabled(false);
        searchEngine->setCurrentSearchTarget(NULL);
    }
    Q_FOREACH (QTabBar* tabBar, mdi->findChildren<QTabBar*>())
    {
        if (!tabBar->tabsClosable()) {
            tabBar->setTabsClosable(true);
            connect(tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
        }
    }
    updateCaption();
}
void MainWindow::connectEditor(SpinEditor *e)
{
    connect(e, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(updateCursorPosition(int,int)));
    connect(e, SIGNAL(modificationChanged(bool)), this, SLOT(documentModified(bool)));
    connect(e, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(e, SIGNAL(methodsListChanged(SpinContextList)), this, SLOT(methodsListChanged(SpinContextList)));
    connect(e, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequested(QPoint)));
    searchEngine->setCurrentSearchTarget(e);
}
void MainWindow::updateCursorPosition(int r, int c)
{
    statusPosition->setText(QString("%1:%2").arg(r+1).arg(c+1));
    statusPosition->show();
    SpinEditor *e = activeEditor();
    if (e) {
        int pos = e->positionFromLineIndex(r, 0);
        methodsListCombo->setCurrentIndex(cursorPositionToMethodIndex(pos));
        wordUnderCursor =  e->getWordAtCursor();
    } else {
        wordUnderCursor = QString();
    }
}
void MainWindow::rebuildWindowMenu()
{
    QList<QMdiSubWindow*> windows = mdi->subWindowList();
    menuWindow->clear();
    if (windows.size()) {
        QActionGroup *g = new QActionGroup(menuWindow);
        for (int i = 0; i < windows.size(); i++) {
            QAction *a = g->addAction(windows[i]->windowTitle());
            a->setCheckable(true);
            a->setChecked(mdi->currentSubWindow() == windows.at(i));
            connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
            mapper->setMapping(a, (QWidget*)windows.at(i));
            menuWindow->addAction(a);
        }
        menuWindow->addSeparator();
    }
    menuWindow->addAction(actTerminal);
    menuWindow->addAction(errorsDock->toggleViewAction());
    menuWindow->addAction(infoDock->toggleViewAction());
    menuWindow->addAction(charTableDock->toggleViewAction());
    menuWindow->addAction(searchResultsDock->toggleViewAction());
}
void MainWindow::rebuildMRUMenu()
{
    QList<QMdiSubWindow*> windows = mdi->subWindowList();
    menuMRU->clear();
    QSettings settings;
    int size = settings.beginReadArray("MRU");
    for (int i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        QAction *a = menuMRU->addAction(settings.value("filename").toString());
        connect(a, SIGNAL(triggered()), mruMapper, SLOT(map()));
        mruMapper->setMapping(a, settings.value("filename").toString());
    }
    settings.endArray();
    if (!size) {
        QAction *a = menuMRU->addAction(tr("No recent files"));
        a->setEnabled(false);
    }
}
void MainWindow::saveDocument()
{
    SpinEditor *e = activeEditor();
    if (!e) return;
    if (e->save()) {
        addToMRU(e->getFileName());
        showStatusMessage(tr("File %1 saved").arg(e->getFileName()));
    }
    actSaveAs->setEnabled(e->hasFilename());
}
void MainWindow::saveDocumentAs()
{
    SpinEditor *e = activeEditor();
    if (!e) return;
    if (e->save(QString())) {
        addToMRU(e->getFileName());
        showStatusMessage(tr("File %1 saved").arg(e->getFileName()));
    }
    actSaveAs->setEnabled(e->hasFilename());
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (askForSave()) {
        event->accept();
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
    }
    else event->ignore();
}
void MainWindow::documentModified(bool m)
{
    SpinEditor *e = activeEditor();
    if (e) {
        actSave->setEnabled(m || !e->hasFilename());
        actUndo->setEnabled(e->isUndoAvailable());
        actRedo->setEnabled(e->isRedoAvailable());
    }
    updateCaption();
}
void MainWindow::cutSelection()
{
    SpinEditor *e = activeEditor();
    if (e) {
        e->cut();
    }
}
void MainWindow::copySelection()
{
    SpinEditor *e = activeEditor();
    if (e) {
        e->copy();
    }
}
void MainWindow::pasteClipboard()
{
    SpinEditor *e = activeEditor();
    if (e) {
        e->paste();
    }
}
void MainWindow::selectionChanged()
{
    SpinEditor *e = activeEditor();
    if (e) {
        actCut->setEnabled(e->hasSelectedText());
        actCopy->setEnabled(e->hasSelectedText());
        actReplace->setEnabled(false);
    }
}
SpinEditor* MainWindow::activeEditor()
{
    if (!mdi->currentSubWindow()) return 0;
    QWidget *w = mdi->currentSubWindow()->widget();
    SpinEditor *e = qobject_cast<SpinEditor *>(w);
    return e;
}
void MainWindow::clipboardChanged(QClipboard::Mode m)
{
    if (m == QClipboard::Clipboard) {
        checkClipboard();
    }
}
void MainWindow::checkClipboard()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    actPaste->setEnabled(mimeData->hasText() && (activeEditor() != 0));
}
void MainWindow::undo()
{
    SpinEditor *e = activeEditor();
    if (e) {
        e->undo();
    }
}
void MainWindow::redo()
{
    SpinEditor *e = activeEditor();
    if (e) {
        e->redo();
    }
}
void MainWindow::closeTab(int n)
{
    QList<QMdiSubWindow*> windows = mdi->subWindowList();
    if (n < 0 || n >= windows.length()) return;
    windows[n]->close();
}
void MainWindow::addToMRU(QString fileName)
{
    QStringList mru;
    QSettings settings;
    int size = settings.beginReadArray("MRU");
    for (int i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        if (settings.value("filename").toString() != fileName)
            mru.append(settings.value("filename").toString());
    }
    settings.endArray();
    mru.insert(0, fileName);
    mru = mru.mid(0, 10);
    settings.beginWriteArray("MRU");
    for (int i = 0; i < mru.size(); i++) {
        settings.setArrayIndex(i);
        settings.setValue("filename", mru[i]);
    }
    settings.endArray();
}
bool MainWindow::openOrActivate(QString fName, int l, int c, bool focus)
{
    Q_UNUSED(c);
    QList<QMdiSubWindow*> windows = mdi->subWindowList();
    for (int i = 0; i < windows.size(); i++) {
        SpinEditor *e = qobject_cast<SpinEditor *>(windows[i]->widget());
        if (e) {
            if(e->getFileName() == fName) {
                mdi->setActiveSubWindow(windows[i]);
                if (l > 0) {
                    e->setCursorPosition(l-1, 0);
                }
                if (focus) {
                    setFocus();
                    activateWindow();
                    e->setFocus();
                }
                return true;
            }
        }
    }
    SpinEditor *e = SpinEditor::loadFile(fName, this);
    if (e) {
        e->setWindowTitle(QFileInfo(fName).completeBaseName());
        QMdiSubWindow *w = mdi->addSubWindow(e);
        w->setSystemMenu(0);
        w->show();
        w->setAttribute(Qt::WA_DeleteOnClose);
        connectEditor(e);
        if (l > 0) {
            e->setCursorPosition(l-1, 0);
        }
        checkClipboard();
        addToMRU(e->getFileName());
        connect(e, SIGNAL(closed(SpinEditor*)), this, SLOT(editorClosed(SpinEditor*)));
        searchEngine->registerSearchable(e);
        return true;
    }
    return false;
}
void MainWindow::updateCaption()
{
    QString caption = "PZST";
    SpinEditor *e = activeEditor();
    if (e) {
        caption = e->windowTitle() + " - " + caption;
    }
    setWindowTitle(caption);
}
void MainWindow::detectProp()
{
    enableUI(false);
    showStatusMessage("");
    QCoreApplication::processEvents();
    doProgramming(0);
    enableUI(true);
}
void MainWindow::enableUI(bool v)
{
    menuBar()->setEnabled(v);
    mdi->setEnabled(v);
    if (v) {
        SpinEditor *e = activeEditor();
        if (e) e->setFocus(Qt::MouseFocusReason);
        QApplication::restoreOverrideCursor();
    } else {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    }
    errorsDock->setEnabled(v);
    infoDock->setEnabled(v);
    charTableDock->setEnabled(v);
    searchResultsDock->setEnabled(v);
}
void MainWindow::doProgramming(int command, QByteArray code)
{
    if (!command)  showStatusMessage("");
    ESerialPort p;
    Preferences pref;
    p.setDeviceName(pref.getPortName());
    if (p.open(QIODevice::ReadWrite | QIODevice::Unbuffered)) {
        p.setBaudRate(ESerialPort::B_115200 );
        if (command) {
            uploadProgress->setValue(0);
            uploadProgress->show();
            uploadMessage->show();
        }
        PropellerLoader l(&p, uploadProgress, uploadMessage);
        l.updateFirmware(command, code);
        while (l.isRunning()) {
            QApplication::processEvents();
        }
        if (command) {
            uploadProgress->hide();
            uploadMessage->hide();
            QApplication::processEvents();
        }
        PropellerLoader::LoaderResult result = l.getResult();
        if (!command) {
            if (result == PropellerLoader::LoaderOK) {
                showStatusMessage(tr("Propeller detected on port %1").arg(p.getDeviceName()));
            } else {
                showStatusMessage(tr("Propeller not found on port %1").arg(p.getDeviceName()), 2);
            }
        } else {
            if (result == PropellerLoader::LoaderOK) {
                showStatusMessage(tr("Propeller programmed!"));
            } else {
                showStatusMessage(tr("Propeller programming failed"), 2);
            }
        }
    } else {
        showStatusMessage(tr("Unable to open port %1").arg(p.getDeviceName()), 2);
    }
    p.close();
}
void MainWindow::compile()
{
    doCompilation(-1);
}
void MainWindow::compileToEEPROM()
{
    doCompilation(1);
}
void MainWindow::compileToRAM()
{
    doCompilation(0);
}
void MainWindow::compileSaveBINARY()
{
    doCompilation(2);
}
void MainWindow::compileSaveEEPROM()
{
    doCompilation(3);
}
void MainWindow::showError(QListWidgetItem *item)
{
    ErrorItem *errItem = (ErrorItem*)(item);
    if (errItem) {
        if (!errItem->err.filename.isEmpty()) {
            openOrActivate(errItem->err.filename, errItem->err.line, errItem->err.col);
        }
    }
}
void MainWindow::messageChanged(const QString &message)
{
    if (message.isNull()) {
        //statusBar()->setStyleSheet("");
    }
}
void MainWindow::showStatusMessage(const QString &msg, int type)
{
    statusMessage->setText(msg + " ");
    switch (type) {
    case 1:
        statusMessage->setStyleSheet("color: #660; font-weight:bold");
        break;
    case 2:
        statusMessage->setStyleSheet("color: red; font-weight:bold");
        break;
    default:
        statusMessage->setStyleSheet("color: green; font-weight:bold");
        break;
    }
}
bool MainWindow::askForSave(bool forClose)
{
    QList<QMdiSubWindow*> windows = mdi->subWindowList();
    bool saveAll = false;
    for (int i = 0; i < windows.size(); i++) {
        SpinEditor *e = qobject_cast<SpinEditor*>(windows.at(i)->widget());
        if (e) {
            if (e->isModified()) {
                int result;
                if (saveAll) result = e->save();
                else result = e->maybeSave(this, forClose);
                if (!result) {
                    return false;
                }
                if (result == 2) saveAll = true;
            }
        }
    }
    return true;
}
void MainWindow::doCompilation(int command)
{
    if (!askForSave(false)) return;
    SpinEditor *e = activeEditor();
    if (!e) return;
    enableUI(false);
    showStatusMessage("");
    statusMessage->repaint();
    QCoreApplication::processEvents();
    SpinCompiler c;
    SpinCompiler::CompileDestination dest;
    switch (command) {
    case 0:
        dest = (SpinCompiler::CompileDestination)(SpinCompiler::TEMP | SpinCompiler::BINARY);
        break;
    case 1:
        dest = (SpinCompiler::CompileDestination)(SpinCompiler::TEMP | SpinCompiler::BINARY);
        break;
    case 2:
        dest = (SpinCompiler::CompileDestination)(SpinCompiler::TEMP | SpinCompiler::BINARY);
        break;
    case 3:
        dest = (SpinCompiler::CompileDestination)(SpinCompiler::TEMP | SpinCompiler::EEPROM);
        break;
    default:
        dest = SpinCompiler::TEMP;
    }

    c.compile(e->getFileName(), dest);
    while (c.isRunning()) {}
    SpinErrors errors = c.getErrors();
    errorsList->clear();
    for (int i = 0; i < errors.size(); i++) {
        new ErrorItem(errors.at(i), errorsList);
    }
    codeInfoBrowser->clear();
    SpinCompiler::Status status = c.getStatus();
    if (status == SpinCompiler::CompileError || status == SpinCompiler::StartError) {
        errorsDock->show();
        errorsDock->raise();
    } else {
        infoDock->raise();
    }
    if (status == SpinCompiler::StartError) {
        showStatusMessage(tr("Failed to start compiler"), 2);
        enableUI(true);
        return;
    } else if (status != SpinCompiler::OK) {
        QString message = tr("Compile error");
        int level = 0;
        switch (status) {
        case SpinCompiler::CompileError : level = 2; break;
        case SpinCompiler::CompileWarning : level = 1; message = tr("Compile warning"); break;
        case SpinCompiler::CompileInfo : level = 1; message = tr("Compile note"); break;
        default:break;
        }

        showStatusMessage(message, level);
        if (status == SpinCompiler::CompileError) {
            enableUI(true);
            return;
        }
    } else {
        showStatusMessage(tr("Compiled successfully"));
    }
    SpinCodeInfo codeInfo = c.parseListFile();
    codeInfoBrowser->append(tr("Code size: %1 bytes, %2 longs").arg(codeInfo.codeSize).arg(codeInfo.codeSize >> 2));
    codeInfoBrowser->append(tr("Variables size: %1 bytes, %2 longs").arg(codeInfo.varSize).arg(codeInfo.varSize >> 2));
    codeInfoBrowser->append(tr("Stack/free space: %1 bytes, %2 longs").arg(codeInfo.stackSize).arg(codeInfo.stackSize >> 2));
    codeInfoBrowser->append(tr("Clock frequency: %1 Hz").arg(codeInfo.clkFreq));
    codeInfoBrowser->append(tr("Clock mode: %1").arg(codeInfo.clkMode));

    if (command < 0) {
        enableUI(true);
        return;
    }
    statusBar()->clearMessage();
    if (command == 2 || command == 3) {
        QString filter;
        if (command == 2) filter = tr("BINARY files (*.binary)");
        else filter = tr("EEPROM files (*.eeprom)");
        QFileDialog dlg(this, tr("Save file"), "", filter);
        dlg.setAcceptMode(QFileDialog::AcceptSave);
        dlg.setDefaultSuffix(command == 2 ? "binary" : "eeprom");
        QApplication::restoreOverrideCursor();
        if (dlg.exec() != QDialog::Accepted) {
            enableUI(true);
            return;
        }
        QString fName = QDir::toNativeSeparators(dlg.selectedFiles().value(0));
        QFile f(fName);
        if (!f.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, tr("Application"),
                                 tr("Cannot write %1:\n%2.")
                                 .arg(fName)
                                 .arg(f.errorString()));
            enableUI(true);
            return;
        }
        if (command == 2) f.write(c.binary());
        else f.write(c.eeprom());
    } else {
        int cmd = command ? 3 : 1;
        doProgramming(cmd, c.binary());
    }
    enableUI(true);
}
void MainWindow::about()
{
    AboutDialog dlg(this);
    dlg.setModal(true);
    dlg.exec();
}
void MainWindow::preferences()
{
    PreferencesDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        readPreferences();
    }
}
void MainWindow::readPreferences()
{
    QList<QMdiSubWindow*> windows = mdi->subWindowList();
    for (int i = 0; i < windows.size(); i++) {
        SpinEditor *e = qobject_cast<SpinEditor *>(windows.at(i)->widget());
        if (!e) continue;
        QsciLexer *lex = e->lexer();
        delete lex;
        e->setLexer(0);
        lex = new SpinLexer();
        e->setLexer(lex);
        e->readPreferences();
    }
    Preferences pref;
    QFont f(pref.getFontName());
    charTable->changeFont(f);
}
void MainWindow::charSelected(QChar c)
{
    SpinEditor *e = activeEditor();
    if (e) {
        if (e->hasSelectedText()) e->removeSelectedText();
        int line, pos;
        e->getCursorPosition(&line, &pos);
        e->insertAt(c, line, pos);
        e->setCursorPosition(line, pos + 1);
        setFocus();
        activateWindow();
        e->setFocus();
    }
}
void MainWindow::charHighlighted(QChar c)
{
    Q_UNUSED(c);
}
void MainWindow::closeWindow()
{
    mdi->closeActiveSubWindow();
}
void MainWindow::closeWindowAll()
{
    mdi->closeAllSubWindows();
}

void MainWindow::methodsListChanged(SpinContextList l)
{
    methodsListCombo->clear();
    qSort(l);
    for (int i = 0; i < l.size(); i++) {
        SpinCodeContext info = l.at(i);
        QIcon icon(info.ctx == SpinCodeContext::Pri ? ":/Icons/pri.png" : ":/Icons/pub.png");
        methodsListCombo->addItem(icon, info.name, info.start);
        methodsListCombo->setItemData(methodsListCombo->count()-1, info.end, Qt::UserRole + 1);
    }
}

void MainWindow::methodChosen(int n)
{
    int pos = methodsListCombo->itemData(n).toInt();
    SpinEditor *e = activeEditor();
    if (e) {
        int line, index;
        e->lineIndexFromPosition(pos, &line, &index);
        e->setCursorPosition(line, 0);
    }
}

int MainWindow::cursorPositionToMethodIndex(int pos)
{
    for (int i = 0; i < methodsListCombo->count(); i ++) {
        int methodStart = methodsListCombo->itemData(i).toInt();
        int methodEnd = methodsListCombo->itemData(i, Qt::UserRole + 1).toInt();
        if (pos >= methodStart && pos < methodEnd) {
            return i;
        }
    }
    return -1;
}

int MainWindow::wordToMethodIndex(QString w)
{
    for (int i = 0; i < methodsListCombo->count(); i ++) {
        if (methodsListCombo->itemText(i).toLower().trimmed() == w.toLower().trimmed()) {
            return i;
        }
    }
    return -1;
}


void MainWindow::contextMenuRequested(const QPoint &position)
{
    Q_UNUSED(position);
    QMenu menu;
    menu.addAction(actCut);
    menu.addAction(actCopy);
    menu.addAction(actPaste);
    menu.addAction(actUndo);
    menu.addAction(actRedo);
    menu.addSeparator();
    menu.addAction(actComplete);
    menu.addAction(actCallTip);
    //menu.addSeparator();
    //menu.addAction(actFold);
    //menu.addAction(actUnfold);
    menu.exec(QCursor::pos());
}

void MainWindow::jumpToMethod()
{
    int idx = wordToMethodIndex(wordUnderCursor);
    if (idx >= 0) {
        methodChosen(idx);
    }
}


void MainWindow::find()
{
    SpinEditor *e = activeEditor();
    if (e) {
        findDialog.exec();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        if (SpinEditor *e = activeEditor()) {
            e->setFocus();
        }
    }
}


void MainWindow::searchRequested(QString search, SearchEngine::SearchOptions options)
{
    searchEngine->find(search, options);
}

void MainWindow::foundInTarget(QString target, SearchEngine::Result res, bool allTargets)
{
    if (!allTargets) {
        openOrActivate(target, -1, -1, false);
        SpinEditor *e = activeEditor();
        if (e) {
            int startIdx = e->text().left(res.pos).toUtf8().size();
            int endIdx = startIdx + res.match.toUtf8().size();
            int sL, sI, eL, eI;
            e->lineIndexFromPosition(startIdx, &sL, &sI);
            e->lineIndexFromPosition(endIdx, &eL, &eI);
            e->setSelection(sL, sI, eL, eI);
        }
    } else {
        QStringList data;
        data << target;
        QTreeWidgetItem *parentItem = NULL;
        for (int i = 0; i < searchTree->topLevelItemCount(); i++) {
            QTreeWidgetItem *item = searchTree->topLevelItem(i);
            if (item->text(0) == target) {
                parentItem = item;
                break;
            }
        }
        if (!parentItem) {
            parentItem = new QTreeWidgetItem(data);
            searchTree->addTopLevelItem(parentItem);
        }
        QTreeWidgetItem *childItem = new QTreeWidgetItem();
        childItem->setText(0, res.context);
        childItem->setData(0, Qt::UserRole, res.pos);
        childItem->setData(0, Qt::UserRole + 1, res.match.size());
        parentItem->addChild(childItem);
    }
}

void MainWindow::editorClosed(SpinEditor *e)
{
    searchEngine->unregisterSearchable(e);
}

void MainWindow::searchStarted(bool allTargets)
{
    if (allTargets) searchTree->clear();
}

void MainWindow::searchFinished(bool allTargets)
{
    if (allTargets) {
        searchResultsDock->show();
        searchResultsDock->raise();
        searchTree->expandAll();
    }
}

void MainWindow::searchTreeClicked(QModelIndex idx)
{
    QModelIndex parent = idx.parent();
    QModelIndex child = idx.child(0, 0);
    if (child.isValid()) return;
    if (!parent.isValid()) return;
    QString fname = parent.data(Qt::DisplayRole).toString();
    if (openOrActivate(fname)) {
        SpinEditor *e = activeEditor();
        if (e) {
            int sl, si;
            int el, ei;
            int charPos = idx.data(Qt::UserRole).toInt();
            int charLen = idx.data(Qt::UserRole+1).toInt();
            int pos = e->text().left(charPos).toUtf8().size();
            int len = e->text().mid(charPos, charLen).toUtf8().size();
            e->lineIndexFromPosition(pos, &sl, &si);
            e->lineIndexFromPosition(pos+len, &el, &ei);
            e->setSelection(sl, si, el, ei);
        }
    }
}


bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    if (!obj)
        return false;

    if (QApplication::activePopupWidget() || QApplication::activeModalWidget() || QApplication::activeWindow() != this)
        return false;

    if (!mdi->isEnabled()) return false;

    if (ev->type() == QEvent::KeyPress || ev->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(ev);
#ifdef Q_WS_MAC
        if (!(keyEvent->modifiers() & Qt::MetaModifier) && keyEvent->key() != Qt::Key_Meta)
#else
        if (!(keyEvent->modifiers() & Qt::ControlModifier) && keyEvent->key() != Qt::Key_Control)
#endif
            return false;

        const bool keyPress = (ev->type() == QEvent::KeyPress) ? true : false;
        switch (keyEvent->key()) {
#ifdef Q_WS_MAC
        case Qt::Key_Meta:
#else
        case Qt::Key_Control:
#endif
            if (!keyPress) {
                if (windowSwitcher->isVisible()) {
                    QList<QMdiSubWindow *> list = mdi->subWindowList(QMdiArea::StackingOrder);
                    mdi->setActiveSubWindow(list.at(windowsList->count() - 1 - windowsList->currentRow()));
                    windowSwitcher->hide();
                    windowSwitcher->releaseMouse();
                    QApplication::setActiveWindow(this);
                }
            }
            return true;
            break;
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            if (keyPress)  {
                if (mdi->subWindowList().size() > 1) {
                    if (!windowSwitcher->isVisible()) {
                        rebuildWindowSwitcher();
                        windowSwitcher->move(rect().x()+rect().width()/2 - 150, rect().y()+rect().height()/2 - 100);
                        windowSwitcher->show();
                        QApplication::setActiveWindow(windowSwitcher);
                        windowSwitcher->raise();
                        windowsList->setFocus();
                        windowSwitcher->grabMouse();
                    }
                    int row = windowsList->currentRow();
                    if (keyEvent->key() == Qt::Key_Tab) row++;
                    else row--;
                    if (row >= windowsList->count()) row = 0;
                    if (row < 0) row =  windowsList->count() - 1;
                    windowsList->setCurrentRow(row);
                }
            }
            return true;
        }
    }
    return false;
}

void MainWindow::rebuildWindowSwitcher()
{
    windowsList->clear();
    QList<QMdiSubWindow *> list = mdi->subWindowList(QMdiArea::StackingOrder);
    for (int i = list.size()-1; i >= 0; i--) {
        windowsList->addItem(list.at(i)->windowTitle());
        if (list.at(i) == mdi->activeSubWindow()) {
            windowsList->setCurrentRow(i);
        }
    }
}

void MainWindow::autoComplete()
{
    SpinEditor *e = activeEditor();
    if (e) {
        e->autoCompleteFromAPIs();
    }
}

void MainWindow::callTip()
{
    SpinEditor *e = activeEditor();
    if (e) {
        e->callTip();
    }
}

void MainWindow::fold()
{
    SpinEditor *e = activeEditor();
    if (e) {
        int line, col, parent;
        e->getCursorPosition(&line, &col);
            parent = e->SendScintilla(QsciScintillaBase::SCI_GETFOLDPARENT, (unsigned long)line, (long)0);
        if (parent >= 0) line = parent;
        if (e->SendScintilla(QsciScintilla::SCI_GETFOLDEXPANDED, line)) {
            e->SendScintilla(QsciScintilla::SCI_TOGGLEFOLD, line);
            e->setCursorPosition(line, 0);
        }
    }
}

void MainWindow::unfold()
{
    SpinEditor *e = activeEditor();
    if (e) {
        int line, col, parent;
        e->getCursorPosition(&line, &col);
        parent = e->SendScintilla(QsciScintillaBase::SCI_GETFOLDPARENT, (unsigned long)line, (long)0);
        if (parent >= 0) line = parent;
        if (!e->SendScintilla(QsciScintilla::SCI_GETFOLDEXPANDED, line))
            e->SendScintilla(QsciScintilla::SCI_TOGGLEFOLD, line);
    }
}

void MainWindow::printDocument()
{
    SpinEditor *e = activeEditor();
    if (e) {
        QsciPrinter p(QPrinter::HighResolution);
        QPrintDialog pd(&p, this);
        if (pd.exec() ==  QDialog::Accepted) {
            p.setMagnification(7 - e->font().pointSize() );
            p.printRange(e);
        }
    }
}
