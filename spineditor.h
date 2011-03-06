#ifndef SPINEDITOR_H
#define SPINEDITOR_H

#include <Qsci/qsciscintilla.h>
#include "spinparser.h"
#include "searchable.h"

namespace PZST {
    class SpinEditor : public QsciScintilla, public Searchable
    {
        Q_OBJECT
    public:
        SpinEditor(QWidget *p = 0);
        static SpinEditor *loadFile(QString fName, QWidget *parent = 0);
        void setFileName(QString fName);
        QString getFileName() {return fileName;};
        bool save();
        bool save(QString fName);
        int maybeSave(QWidget *parent = 0, bool forClose = true);
        void readPreferences();
        bool hasFilename() {return HasFilename;};
        SpinMethodInfoList getMethodDefs();
        QString getWordAtCursor() const;
        virtual QStringList getSearchTargets();
        virtual QString getSearchTargetText(QString &target);
        virtual int getSearchTargetStartPosition(QString &target, bool backward);
        virtual void replaceInTarget(QString &target, int start, int len, QString &text);
        virtual bool supportsReplace(QString &target);

    private:
        void initialize();
        void updateCaption();
        QString fileName;
        bool HasFilename;
        SpinParser parser;
        void parseMethods();
        char getChar(int &pos, int delta) const;
    protected:
        virtual void closeEvent ( QCloseEvent * event );
        virtual void keyPressEvent(QKeyEvent *e);
    private slots:
        void cursorPositionChanged(int,int);
    public slots:
        void updateModificationStatus(bool m);
        void documentModified();
    signals:
        void methodsListChanged(SpinMethodInfoList);
        // !!!!!!!!! e can be already deleted !!!!!!!!
        void closed(SpinEditor*e);
    };
}

#endif // SPINEDITOR_H
