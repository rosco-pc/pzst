#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMdiArea>
#include <QVBoxLayout>
#include <QAction>
#include <QToolBar>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QLabel>
#include <QSignalMapper>
#include <QClipboard>
#include <QProgressBar>
#include <QDockWidget>
#include <QListWidget>
#include <QTextBrowser>
#include <QToolBar>
#include <QComboBox>
#include <QTreeWidget>

#include "spineditor.h"
#include "chartable.h"
#include "finddialog.h"

namespace PZST {

    typedef struct {
        QString text;
        QString replacement;
        bool re;
        bool cs;
        bool wo;
        bool wrap;
        bool forward;
    } SearchSettings ;

    class MainWindow : public QMainWindow
    {
    Q_OBJECT
    public:
        explicit MainWindow(QWidget *parent = 0);

    signals:

    public slots:
        void newDocument();
        void openDocument();
        void saveDocument();
        void saveDocumentAs();
        void windowActivated(QWidget* w);
        void windowActivated(QMdiSubWindow* w);
        void updateCursorPosition(int, int);
        void rebuildWindowMenu();
        void rebuildMRUMenu();
        void documentModified(bool);
        void cutSelection();
        void copySelection();
        void pasteClipboard();
        void selectionChanged();
        void undo();
        void redo();
        void clipboardChanged(QClipboard::Mode m);
        void closeTab(int);
        bool openOrActivate(QString fName, int l = -1, int c = -1, bool focus = true);
        void detectProp();
        void compile();
        void compileToRAM();
        void compileToEEPROM();
        void compileSaveBINARY();
        void compileSaveEEPROM();
        void showError(QListWidgetItem *item);
        void messageChanged(const QString &message);
        void about();
        void preferences();
        void charSelected(QChar c);
        void charHighlighted(QChar c);
        void closeWindow();
        void closeWindowAll();
        void contextMenuRequested(const QPoint &position);
        void find();
        void autoComplete();
        void callTip();
        void fold();
        void unfold();
        void printDocument();
        void increaseFontSize();
        void decreaseFontSize();

    private slots:
        void methodsListChanged(SpinContextList);
        void methodChosen(int);
        void jumpToMethod();
        void editorClosed(SpinEditor *e);
        void searchStarted(bool allTargets);
        void searchFinished(bool allTargets);
        void searchTreeClicked(QModelIndex);

    private:
        void showStatusMessage(const QString& msg, int type = 0);
        bool askForSave(bool forClose = true);
        void connectEditor(SpinEditor *e);
        SpinEditor *activeEditor();
        void checkClipboard();
        void addToMRU(QString fileName);
        void updateCaption();
        void enableUI(bool v);
        void doProgramming(int command, QByteArray code = QByteArray());
        void readPreferences();
        void createActions();
        QAction* createAction(QString text, QString seq = QString(), QString iconFile = QString(), bool inMenu = false);
        QAction* createAction(QString text, QKeySequence seq, QString iconFile = QString(), bool inMenu = false);
        QAction* createAction(QString text, QKeySequence::StandardKey seq, QString iconFile = QString(), bool inMenu = false);

        void createMenus();
        void createStatusBar();
        void createToolbars();
        void createDocks();
        void rebuildWindowSwitcher();

        // -1 - compile
        // 0 - to RAM
        // 1 - to EEPROM
        // 2 - save BIN
        // 3 - save EEPROM
        void doCompilation(int command);

        int cursorPositionToMethodIndex(int pos);
        int wordToMethodIndex(QString w);

        QDockWidget *errorsDock;
        QDockWidget *infoDock;
        QListWidget *errorsList;
        QTextBrowser *codeInfoBrowser;
        CharTable *charTable;
        QDockWidget *charTableDock;
        QDockWidget *searchResultsDock;

        QMdiArea *mdi;
        QVBoxLayout *layout;
        QToolBar *toolbar;

        QMenu *menuFile;
        QMenu *menuEdit;
        QMenu *menuView;
        QMenu *menuWindow;
        QMenu *menuCompile;
        QMenu *menuMRU;
        QMenu *menuHelp;

        QAction *actClose;
        QAction *actCloseAll;
        QAction *actOpen;
        QAction *actNew;
        QAction *actSave;
        QAction *actSaveAs;
        QAction *actQuit;
        QAction *actCut;
        QAction *actCopy;
        QAction *actPaste;
        QAction *actUndo;
        QAction *actRedo;
        QAction *actDetectProp;
        QAction *actCompile;
        QAction *actLoadRAM;
        QAction *actLoadEEPROM;
        QAction *actSaveBIN;
        QAction *actSaveEEPROM;
        QAction *actAbout;
        QAction *actPreferences;
        QAction *actFind;
        QAction *actFindNext;
        QAction *actReplace;
        QAction *actComplete;
        QAction *actCallTip;
        QAction *actFold;
        QAction *actUnfold;
        QAction *actPrint;
        QAction *actFontLarger;
        QAction *actFontSmaller;

        QLabel *statusFileName;
        QLabel *statusPosition;
        QLabel *statusMessage;
        QLabel *uploadMessage;
        QProgressBar *uploadProgress;

        QSignalMapper *mapper;
        QSignalMapper *mruMapper;

        QToolBar *editToolBar;
        QComboBox *methodsListCombo;

        QString wordUnderCursor;

        SearchSettings searchSettings;
        FindDialog findDialog;
        QWidget *lastActiveWindow;

        QTreeWidget *searchTree;

        QWidget *windowSwitcher;
        QListWidget *windowsList;

    protected:
        void closeEvent(QCloseEvent *event);
        void keyPressEvent(QKeyEvent *e);
        bool eventFilter(QObject *obj, QEvent *ev);
    };
}
#endif // MAINWINDOW_H
