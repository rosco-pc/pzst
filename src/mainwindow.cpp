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
#include "eserialportmanager.h"
#include "propellerloader.h"
#include "spincompiler.h"
#include "erroritem.h"
#include "aboutdialog.h"
#include "preferencesdialog.h"
#include "pzstpreferences.h"
#include "searchengine.h"
#include "shortcuts.h"
#include "spinsourcefactory.h"
#include "groupactiondialog.h"

using namespace PZST;

SpinError::SpinError(QString msg, QString f, int l, int c) :message(msg), filename(f), line(l), col(c)
{
}
SpinError::SpinError() :message(""), filename(""), line(-1), col(-1)
{
}

MainWindow::~MainWindow()
{
    SpinSourceFactory::shutdown();
    delete modifiedFiles;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), findDialog(this), modifiedFiles(0)
{
    lastActiveWindow = 0;
    modifiedFiles = new QStringList();
    setFocusPolicy(Qt::StrongFocus);
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
    windowMapper = new QSignalMapper(this);
    mruMapper = new QSignalMapper(this);
    mdi = new QMdiArea(this);
    setCentralWidget(mdi);
    mdi->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdi->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdi->setViewMode(QMdiArea::TabbedView);
    mdi->setTabShape(QTabWidget::Triangular);
    connect(mdi, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(windowActivated(QMdiSubWindow*)));
    connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(windowActivated(QWidget*)));
    connect(mruMapper, SIGNAL(mapped(QString)), this, SLOT(openOrActivate(QString)));

    connect(QApplication::clipboard(), SIGNAL(changed (QClipboard::Mode)), this, SLOT(clipboardChanged(QClipboard::Mode)));
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/Files/appicon.ico"), QSize(), QIcon::Normal, QIcon::Off);
    setWindowIcon(icon);
    setDocumentMode(true);

    qsText = new QLineEdit(this);

    createDocks();
    createActions();
    createToolbars();
    createMenus();
    createStatusBar();

    QSettings *s = new QSettings();
    restoreGeometry(s->value("geometry").toByteArray());
    restoreState(s->value("windowState").toByteArray());

    setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );


    setTabShape(QTabWidget::Triangular);
    windowActivated((QWidget*)0);

    QApplication::instance()->installEventFilter(this);
    SearchEngine::connectInstance(SIGNAL(found(Searchable*,int,int,const SearchRequest*)), this, SLOT(searchFound(Searchable*,int,int,const SearchRequest*)));
    SearchEngine::connectInstance(SIGNAL(searchStarted(const SearchRequest*)), this, SLOT(searchStarted(const SearchRequest*)));
    SearchEngine::connectInstance(SIGNAL(searchFinished(const SearchRequest*)), this, SLOT(searchFinished(const SearchRequest*)));
    SearchEngine::connectInstance(SIGNAL(noResults()), this, SLOT(noResults()));
    findDialog.setOpenFiles(this);

    Preferences::connectInstance(SIGNAL(valueChanged(QString,QString,QVariant)), charTable, SLOT(preferencesChanged(QString,QString,QVariant)));
    Preferences::connectInstance(SIGNAL(shortcutChanged(QString,QString)), this, SLOT(shortcutChanged(QString,QString)));

    connect(SpinSourceFactory::instance(), SIGNAL(extrnallyModified(QString)), this, SLOT(externallyModified(QString)));
}

QAction* MainWindow::createAction(QString name, QString iconFile, bool inMenu)
{
    Preferences p;
    QAction *act = new QAction(Shortcuts::title(name), this);
    act->setShortcuts(p.getShortcuts(name));
    act->setIcon(QIcon(iconFile));
    act->setIconVisibleInMenu(inMenu);
    actions[name] = act;
    Preferences::connectInstance(SIGNAL(shortcutChanged(QString,QString)), this, SLOT(shortcutChanged(QString,QString)));
    return act;
}


void MainWindow::createActions()
{
    actNew = createAction("File.New", ":/Icons/new.png", true);
    connect(actNew, SIGNAL(triggered()), this, SLOT(newDocument()));

    actOpen = createAction("File.Open", ":/Icons/open.png", true);
    connect(actOpen, SIGNAL(triggered()), this, SLOT(openDocument()));

    actClose = createAction("File.Close", ":/Icons/close.png", true);
    connect(actClose, SIGNAL(triggered()), this, SLOT(closeWindow()));

    actCloseAll = createAction("File.CloseAll");
    connect(actCloseAll, SIGNAL(triggered()), this, SLOT(closeWindowAll()));

    actSave = createAction("File.Save", ":/Icons/save.png", true);
    connect(actSave, SIGNAL(triggered()), this, SLOT(saveDocument()));

    actSaveAs = createAction("File.SaveAs", ":/Icons/saveas.png", true);
    connect(actSaveAs, SIGNAL(triggered()), this, SLOT(saveDocumentAs()));

    actPrint = createAction("File.Print", ":/Icons/print.png", true);
    connect(actPrint, SIGNAL(triggered()), this, SLOT(printDocument()));

    actQuit = createAction("File.Quit", ":/Icons/exit.png", true);
    connect(actQuit, SIGNAL(triggered()), this, SLOT(close()));

    actCut = createAction("Editor.Cut", ":/Icons/cut.png", true);
    connect(actCut, SIGNAL(triggered()), this, SLOT(cutSelection()));

    actCopy = createAction("Editor.Copy", ":/Icons/copy.png", true);
    connect(actCopy, SIGNAL(triggered()), this, SLOT(copySelection()));

    actPaste = createAction("Editor.Paste", ":/Icons/paste.png", true);
    connect(actPaste, SIGNAL(triggered()), this, SLOT(pasteClipboard()));

    actUndo = createAction("Editor.Undo", ":/Icons/undo.png", true);
    connect(actUndo, SIGNAL(triggered()), this, SLOT(undo()));

    actRedo = createAction("Editor.Redo", ":/Icons/redo.png", true);
    connect(actRedo, SIGNAL(triggered()), this, SLOT(redo()));

    actDetectProp = createAction("Compile.Detect");
    connect(actDetectProp, SIGNAL(triggered()), this, SLOT(detectProp()));

    actCompile = createAction("Compile.Compile");
    connect(actCompile, SIGNAL(triggered()), this, SLOT(compile()));

    actLoadRAM = createAction("Compile.LoadRAM");
    connect(actLoadRAM, SIGNAL(triggered()), this, SLOT(compileToRAM()));

    actLoadEEPROM = createAction("Compile.LoadROM");
    connect(actLoadEEPROM, SIGNAL(triggered()), this, SLOT(compileToEEPROM()));

    actSaveBIN = createAction("Compile.WriteBINARY");
    connect(actSaveBIN, SIGNAL(triggered()), this, SLOT(compileSaveBINARY()));

    actSaveEEPROM = createAction("Compile.WriteEEPROM");
    connect(actSaveEEPROM, SIGNAL(triggered()), this, SLOT(compileSaveEEPROM()));

    actAbout = createAction("Help.About");
    connect(actAbout, SIGNAL(triggered()), this, SLOT(about()));

    actPreferences = createAction("Environment.Preferences", ":/Icons/configure.png", true);
    connect(actPreferences, SIGNAL(triggered()), this, SLOT(preferences()));

    actFind = createAction("Editor.Find", ":/Icons/find.png", true);
    connect(actFind, SIGNAL(triggered()), this, SLOT(find()));

    actFindNext = createAction("Editor.FindNext", "", true);
    connect(actFindNext, SIGNAL(triggered()), &findDialog, SLOT(findNext()));

    actReplace = createAction("Editor.Replace", "", true);
    connect(actReplace, SIGNAL(triggered()), this, SLOT(replace()));

    actQuickSearch = createAction("Editor.QuickSearch", ":/Icons/find.png", true);
    connect(actQuickSearch, SIGNAL(triggered()), qsText, SLOT(setFocus()));
    connect(actQuickSearch, SIGNAL(triggered()), qsText, SLOT(selectAll()));
    connect(actQuickSearch, SIGNAL(triggered()), this, SLOT(qsEnter()));

    actComplete= createAction("Editor.Autocomplete", "", true);
    connect(actComplete, SIGNAL(triggered()), this, SLOT(autoComplete()));
    actComplete->setEnabled(false);

    actCallTip = createAction("Editor.CallTip", "", true);
    connect(actCallTip, SIGNAL(triggered()), this, SLOT(callTip()));
    actCallTip->setEnabled(false);

    actFold = createAction("Editor.Fold", "", true);
    connect(actFold, SIGNAL(triggered()), this, SLOT(fold()));
    actFold->setEnabled(false);

    actUnfold = createAction("Editor.Unfold", "", true);
    connect(actUnfold, SIGNAL(triggered()), this, SLOT(unfold()));
    actUnfold->setEnabled(false);

    actFontLarger = createAction("Editor.ZoomIn", ":/Icons/fontsizeup.png", true);
    connect(actFontLarger, SIGNAL(triggered()), this, SLOT(increaseFontSize()));
    actFontLarger->setEnabled(false);


    actFontSmaller = createAction("Editor.ZoomOut", ":/Icons/fontsizedown.png", true);
    connect(actFontSmaller, SIGNAL(triggered()), this, SLOT(decreaseFontSize()));
    actFontSmaller->setEnabled(false);
}
void MainWindow::createMenus()
{
    menuFile = menuBar()->addMenu(tr("&File"));
    menuEdit = menuBar()->addMenu(tr("&Edit"));
    menuView = menuBar()->addMenu(tr("&View"));
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
    menuEdit->addAction(actQuickSearch);
    menuEdit->addSeparator();
    menuEdit->addAction(actComplete);
    menuEdit->addAction(actCallTip);
    menuEdit->addSeparator();
    //menuEdit->addAction(actFold);
    //menuEdit->addAction(actUnfold);
    //menuEdit->addSeparator();
    menuEdit->addAction(actPreferences);

    menuView->addAction(actFontLarger);
    menuView->addAction(actFontSmaller);
    menuView->addSeparator();
    menuView->addAction(errorsDock->toggleViewAction());
    menuView->addAction(infoDock->toggleViewAction());
    menuView->addAction(charTableDock->toggleViewAction());
    menuView->addAction(searchResultsDock->toggleViewAction());
    menuView->addAction(editToolBar->toggleViewAction());

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

    editToolBar->addSeparator();
    editToolBar->addWidget(new QLabel(tr(" Quick search:")));
    editToolBar->addWidget(qsText);
    connect(qsText, SIGNAL(returnPressed()), this, SLOT(quickSearch()));
    connect(qsText, SIGNAL(textChanged(QString)), this, SLOT(quickSearch(QString)));

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
    searchTree->setFocusPolicy(Qt::ClickFocus);
    searchTree->setHeaderHidden(true);
    searchResultsDock->setWidget(searchTree);
    addDockWidget(Qt::BottomDockWidgetArea, searchResultsDock);
    connect(searchTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(searchTreeClicked(QModelIndex)));
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

    findDialog.setCurrentFile(0);

    if (!w) {
        w = mdi->currentSubWindow();
    }
    if (w) {
        lastActiveWindow = w;
        QMdiSubWindow *sw = qobject_cast<QMdiSubWindow *>(w);
        QWidget *widget = sw->widget();
        mdi->setActiveSubWindow(sw);
        SpinEditor *e = qobject_cast<SpinEditor *>(widget);
        if (e) {
            findDialog.setCurrentFile(e);
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
            actReplace->setEnabled(e->hasSelectedText());
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
            actQuickSearch->setEnabled(true);
            actFontLarger->setEnabled(true);
            actFontSmaller->setEnabled(true);
            actFindNext->setEnabled(true);
            methodsListChanged(e, e->getMethodDefs());
            methodsListCombo->setEnabled(true);
            updateCursorPosition(line, col);
            qsText->setEnabled(true);
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
        actQuickSearch->setEnabled(false);
        actFindNext->setEnabled(false);
        actReplace->setEnabled(false);
        actFontLarger->setEnabled(false);
        actFontSmaller->setEnabled(false);
        methodsListCombo->clear();
        methodsListCombo->setEnabled(false);
        qsText->setEnabled(false);
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
    connect(e, SIGNAL(methodsListChanged(SpinEditor*,SpinContextList)), this, SLOT(methodsListChanged(SpinEditor*,SpinContextList)));
    connect(e, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequested(QPoint)));
    SearchEngine::connectInstance(SIGNAL(searchStarted()), e, SLOT(beginUndoActionSlot()));
    SearchEngine::connectInstance(SIGNAL(searchFinished(const SearchRequest*)), e, SLOT(endUndoActionSlot()));
    Preferences::connectInstance(SIGNAL(valueChanged(QString,QString,QVariant)), e, SLOT(preferencesChanged(QString,QString,QVariant)));
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
            connect(a, SIGNAL(triggered()), windowMapper, SLOT(map()));
            windowMapper->setMapping(a, (QWidget*)windows.at(i));
            menuWindow->addAction(a);
        }
        menuWindow->addSeparator();
    }
}

void MainWindow::rebuildMRUMenu()
{
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
    if (askForSave(0, true)) {
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
        actReplace->setEnabled(e->hasSelectedText());
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
    QMdiSubWindow* wnd = windows[n];
    QWidget *w = wnd->widget();
    SpinEditor *e = qobject_cast<SpinEditor *>(w);
    if (askForSave(e, true)) windows[n]->close();
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
        addSearchable(e);
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
    Preferences pref;
    ESerialPortProxy* p = ESerialPortManager::obtain(pref.getPortName());
    bool close = !p->isOpen();
    p->grab();
    if (p->open(QIODevice::ReadWrite | QIODevice::Unbuffered)) {
        p->setBaudRate(ESerialPort::B_115200 );
        if (command) {
            uploadProgress->setValue(0);
            uploadProgress->show();
            uploadMessage->show();
        }
        PropellerLoader l(p, uploadProgress, uploadMessage);
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
                showStatusMessage(tr("Propeller detected on port %1").arg(p->getDeviceName()));
            } else {
                showStatusMessage(tr("Propeller not found on port %1").arg(p->getDeviceName()), 2);
            }
        } else {
            if (result == PropellerLoader::LoaderOK) {
                showStatusMessage(tr("Propeller programmed!"));
            } else {
                showStatusMessage(tr("Propeller programming failed"), 2);
            }
        }
    } else {
        showStatusMessage(tr("Unable to open port %1").arg(p->getDeviceName()), 2);
    }
    if (close)
        p->close();
    delete p;
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
bool MainWindow::askForSave(SpinEditor *which, bool forClose)
{

    QList<QMdiSubWindow*> windows = mdi->subWindowList();
    GroupActionDialog dlg(forClose ? GroupActionDialog::Close : GroupActionDialog::Other,  this);
    QStringList files;
    bool haveUnsaved = false;
    for (int i = 0; i < windows.size(); i++) {
        SpinEditor *e = qobject_cast<SpinEditor*>(windows.at(i)->widget());
        if (e) {
            if (e->isModified() && (which == 0 || which == e)) {
                dlg.addFile(e->getFileName());
                haveUnsaved = true;
            }
        }
    }
    if (!haveUnsaved) return true;
    if (dlg.run(files)) {
        for (int i = 0; i < windows.size(); i++) {
            SpinEditor *e = qobject_cast<SpinEditor*>(windows.at(i)->widget());
            if (e) {
                if (e->isModified()) {
                    if (files.contains(e->getFileName())) {
                        if (!e->save()) return false;
                    }
                }
            }
        }
        return true;
    }
    return false;
}
void MainWindow::doCompilation(int command)
{
    if (!askForSave(0, false)) return;
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
    connect(&dlg, SIGNAL(shortcutsChanged()), this, SLOT(readKeys()));
    dlg.exec();
}

void MainWindow::readKeys()
{
    QList<QMdiSubWindow*> windows = mdi->subWindowList();
    for (int i = 0; i < windows.size(); i++) {
        SpinEditor *e = qobject_cast<SpinEditor*>(windows.at(i)->widget());
        if (e) {
            e->readKeys();
        }
    }
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
    if (askForSave(activeEditor(), true)) mdi->closeActiveSubWindow();
}
void MainWindow::closeWindowAll()
{
    if (askForSave(0, true)) mdi->closeAllSubWindows();
}

void SpinEditor::closeEvent(QCloseEvent *event)
{
    emit closed(this);
    QsciScintilla::closeEvent(event);
}

void MainWindow::methodsListChanged(SpinEditor *source, SpinContextList l)
{
    SpinEditor *e = activeEditor();
    if (e != source) return;
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

void MainWindow::replace()
{
    SpinEditor *e = activeEditor();
    if (e) {
        int sl, sc, el, ec;
        e->getSelection(&sl, &sc, &el, &ec);
        e->setCursorPosition(sl, sc);
        findDialog.on_replaceButton_clicked();
        findDialog.on_findButton_clicked();
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


void MainWindow::editorClosed(SpinEditor *e)
{
    removeSearchable(e);
}

void MainWindow::searchStarted(const SearchRequest *)
{
    searchTree->clear();
}

void MainWindow::searchFinished(const SearchRequest *req)
{
    if (req->getOptions() & SearchRequest::All) {
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
            int charPos = idx.data(Qt::UserRole).toInt();
            int pos = e->text().left(charPos).toUtf8().size();
            e->lineIndexFromPosition(pos, &sl, &si);
            e->setCursorPosition(sl, si);
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

void MainWindow::increaseFontSize()
{
    Preferences pref;
    int fontSize = pref.getFontSize();
    fontSize = (fontSize >= 10) ? fontSize + 2 : fontSize + 1;
    if (fontSize > 48) fontSize = 48;
    pref.setFontSize(fontSize);
}

void MainWindow::decreaseFontSize()
{
    Preferences pref;
    int fontSize = pref.getFontSize();
    fontSize = (fontSize <= 10) ? fontSize - 1 : fontSize - 2;
    if (fontSize < 6) fontSize = 6;
    pref.setFontSize(fontSize);
}

QString MainWindow::searchScopeName() const
{
    return tr("All open documents");
}

void MainWindow::searchFound(Searchable *target, int pos, int len, const SearchRequest* req)
{
    SpinEditor *e = activeEditor();
    if (!e) return;
    if (target == dynamic_cast<Searchable*>(e) && (!(req->getOptions() & SearchRequest::All) || req->getOptions() & SearchRequest::Replace)) {
        QString txt = e->text();
        int startPos = txt.left(pos).toUtf8().size();
        int endPos = txt.left(pos + len).toUtf8().size();
        int l1, c1, l2, c2;
        e->lineIndexFromPosition(startPos, &l1, &c1);
        e->lineIndexFromPosition(endPos, &l2, &c2);
        if (req->getOptions() & SearchRequest::Backwards) e->setSelection(l2, c2, l1, c1);
        else e->setSelection(l1, c1, l2, c2);
        actReplace->setEnabled(true);
    } else {
        QStringList data;
        data << target->searchTargetId();
        QTreeWidgetItem *parentItem = NULL;
        for (int i = 0; i < searchTree->topLevelItemCount(); i++) {
            QTreeWidgetItem *item = searchTree->topLevelItem(i);
            if (item->text(0) == target->searchTargetId()) {
                parentItem = item;
                break;
            }
        }
        if (!parentItem) {
            parentItem = new QTreeWidgetItem(data);
            searchTree->addTopLevelItem(parentItem);
        }
        QTreeWidgetItem *childItem = new QTreeWidgetItem();
        childItem->setText(0, target->searchTargetText().mid(pos, len).simplified());
        childItem->setData(0, Qt::UserRole, pos);
        childItem->setData(0, Qt::UserRole + 1, len);
        parentItem->addChild(childItem);
    }
}


void MainWindow::quickSearch()
{
    SpinEditor *e = activeEditor();
    if (!e) return;
    SearchRequest req;
    if (!SearchEngine::parseQuickSearch(qsText->text(), req)) {
        qsText->setStyleSheet("background-color: #FDD");
        return;
    }
    qsText->setStyleSheet("");
    findDialog.search(req);
    if ((req.getOptions() & SearchRequest::All)) {
        e->setFocus();
    } else {
        e->getCursorPosition(&qsLine, &qsCol);
    }
}

void MainWindow::quickSearch(QString str)
{
    SpinEditor *e = activeEditor();
    if (!e) return;
    e->setCursorPosition(qsLine, qsCol);
    SearchRequest req;
    if (!SearchEngine::parseQuickSearch(str, req)) {
        qsText->setStyleSheet("background-color: #FDD");
        return;
    }
    qsText->setStyleSheet("");
    req.removeOptions(SearchRequest::Replace);
    req.removeOptions(SearchRequest::All);
    findDialog.search(req);
}

void MainWindow::noResults()
{
    statusBar()->clearMessage();
    statusBar()->showMessage(tr("No results"), 2000);
    qsText->setStyleSheet("background-color: #FFC");
}

void MainWindow::qsEnter()
{
    SpinEditor *e = activeEditor();
    if (!e) return;
    e->getCursorPosition(&qsLine, &qsCol);
    quickSearch(qsText->text());
}

void MainWindow::shortcutChanged(QString name, QString value)
{
    if (actions.contains(name)) {
        QStringList shortcuts = value.split("\n");
        QList<QKeySequence> seq;
        foreach (QString shortcut, shortcuts) {
            seq << QKeySequence::fromString(shortcut, QKeySequence::PortableText);
        }
        actions[name]->setShortcuts(seq);
    }
}

void MainWindow::externallyModified(QString fileName)
{
    if (!modifiedFiles->contains(fileName)) {
        modifiedFiles->append(fileName);
    }
    if (QApplication::activePopupWidget() || QApplication::activeModalWidget() || QApplication::activeWindow() != this)
        return;
    reloadExternallyModified();
}

bool MainWindow::event(QEvent *event)
{
    bool result = QMainWindow::event(event);
    if (!modifiedFiles) return result;
    if (event->type() == QEvent::WindowActivate) {
        if (QApplication::activePopupWidget() || QApplication::activeModalWidget() || QApplication::activeWindow() != this)
            return result;
        if (modifiedFiles->empty()) return result;
        reloadExternallyModified();
    }
    return result;
}

void MainWindow::reloadExternallyModified()
{
    GroupActionDialog dlg(GroupActionDialog::Reload,  this);
    QStringList modFiles(*modifiedFiles);
    modifiedFiles->clear();
    foreach (QString fileName, modFiles) {
        dlg.addFile(fileName);
    }
    QStringList reloadFiles;
    bool result = dlg.run(reloadFiles);
    if (!result) reloadFiles.clear();
    QList<QMdiSubWindow*> windows = mdi->subWindowList();
    for (int i = 0; i < windows.size(); i++) {
        SpinEditor *e = qobject_cast<SpinEditor*>(windows.at(i)->widget());
        if (e) {
            QString fName = e->getFileName();
            if (reloadFiles.contains(fName)) {
                e->loadFile(fName);
            } else if (modFiles.contains(fName)) {
                SpinSourceFactory::instance()->removeSource(fName);
                SpinSourceFactory::instance()->addSource(fName, e->text());
            }
        }
    }
}
