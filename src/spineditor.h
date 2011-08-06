#ifndef SPINEDITOR_H
#define SPINEDITOR_H

#include <Qsci/qsciscintilla.h>
#include <QVariant>
#include "spincodeparser.h"
#include "spinlexer.h"
#include "spincompletionsource.h"
#include "searchscope.h"
#include "searchable.h"

namespace PZST {
    class SpinEditor : public QsciScintilla, public Searchable, public SearchScope
    {
        Q_OBJECT
    public:
        SpinEditor(QWidget *p = 0);
        ~SpinEditor();
        static SpinEditor *loadFile(QString fName, QWidget *parent);
        bool loadFile(QString fName);
        void setFileName(QString fName);
        QString getFileName() const {return fileName;};
        bool save();
        bool save(QString fName);
        void readPreferences();
        bool hasFilename() {return HasFilename;};
        SpinContextList getMethodDefs();
        SpinCodeParser *getParser();
        QString getWordAtCursor() const;
        virtual QStringList apiContext(int pos, int &context_start, int &last_word_start);
        char getCharacter(int &pos) const;
        virtual QString searchTargetId() const;
        virtual QString searchTargetText() const;
        virtual QString searchScopeName() const;
        virtual bool supportsReplace() const;
        virtual int getStartPosition() const;
        virtual void replaceInTarget(QString);

    private:
        void initialize();
        void updateCaption();
        QString fileName;
        bool HasFilename;
        char getChar(int &pos, int delta) const;
        SpinCompletionSource *completion;
        SpinLexer *spinLexer;
        void registerIcons();
    protected:
        virtual void closeEvent ( QCloseEvent * event );
        virtual void keyPressEvent(QKeyEvent *e);

    private slots:
        void cursorPositionChanged(int,int);
        void handlePreModified(int pos, int mtype, const char *text, int len,
                int added, int line, int foldNow, int foldPrev, int token,
                int annotationLinesAdded);
    public slots:
        void updateModificationStatus(bool m);
        void documentModified();
        void beginUndoActionSlot();
        void endUndoActionSlot();
        void preferencesChanged(QString, QString, QVariant);
        void readKeys();
    signals:
        // !!!!!!!!! e can be already deleted !!!!!!!!
        void closed(SpinEditor*e);
        void methodsListChanged(SpinEditor*, SpinContextList);
    };
}

#endif // SPINEDITOR_H
