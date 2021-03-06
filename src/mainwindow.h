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
#include <QLineEdit>
#include <QMap>

#include "spineditor.h"
#include "chartable.h"
#include "finddialog.h"
#include "searchscope.h"
#include "searchengine.h"

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

    class MainWindow : public QMainWindow, public SearchScope
    {
    Q_OBJECT
    public:
        explicit MainWindow(QWidget *parent = 0);
        virtual QString searchScopeName() const;
        ~MainWindow();

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
        void replace();
        void autoComplete();
        void callTip();
        void fold();
        void unfold();
        void printDocument();
        void increaseFontSize();
        void decreaseFontSize();

    private slots:
        void searchFound(Searchable*, int pos, int len, const SearchRequest*);
        void methodsListChanged(SpinEditor *, SpinContextList);
        void methodChosen(int);
        void jumpToMethod();
        void editorClosed(SpinEditor *e);
        void searchStarted(const SearchRequest*);
        void searchFinished(const SearchRequest*);
        void searchTreeClicked(QModelIndex);
        void quickSearch();
        void quickSearch(QString);
        void noResults();
        void qsEnter();
        void shortcutChanged(QString, QString);
        void readKeys();
        void externallyModified(QString);

    private:
        void showStatusMessage(const QString& msg, int type = 0);
        bool askForSave(SpinEditor *which, bool forClose);
        void connectEditor(SpinEditor *e);
        SpinEditor *activeEditor();
        void checkClipboard();
        void addToMRU(QString fileName);
        void updateCaption();
        void enableUI(bool v);
        void doProgramming(int command, QByteArray code = QByteArray());
        void createActions();
        QAction* createAction(QString name, QString iconFile = QString(), bool inMenu = false);

        void createMenus();
        void createStatusBar();
        void createToolbars();
        void createDocks();
        void rebuildWindowSwitcher();

        void reloadExternallyModified();

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
        QAction *actQuickSearch;
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

        QSignalMapper *windowMapper;
        QSignalMapper *mruMapper;
        QMap<QString, QAction*> actions;

        QToolBar *editToolBar;
        QComboBox *methodsListCombo;

        QString wordUnderCursor;

        FindDialog findDialog;
        QWidget *lastActiveWindow;

        QTreeWidget *searchTree;

        QWidget *windowSwitcher;
        QListWidget *windowsList;

        QLineEdit *qsText;
        QStringList *modifiedFiles;

        int qsLine, qsCol;

    protected:
        void closeEvent(QCloseEvent *event);
        void keyPressEvent(QKeyEvent *e);
        bool eventFilter(QObject *obj, QEvent *ev);
        bool event(QEvent *event);
    };
}
#endif // MAINWINDOW_H
