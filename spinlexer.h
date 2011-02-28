#ifndef SPINLEXER_H
#define SPINLEXER_H

#include <Qsci/qscilexercustom.h>
#include <QVector>
#include <QStringList>
#include <QTextCodec>

namespace PZST {
    struct SpinStyleRef
    {
        SpinStyleRef(int start, int len, int style);
        SpinStyleRef() {};
        int start, len, style;
    };

    struct SpinCommentBlock
    {
        SpinCommentBlock(int start, int len, bool dbl);
        SpinCommentBlock() {};
        int start, len;
        bool dbl;
    };

    class SpinLexer : public QsciLexerCustom
    {
        enum SpinLexerStyle {
            Comment = 1,
            MLComment1,
            MLComment2,
            Reserved,
            Condition ,
            Identifier,
            Number,
            Type,
            String,
        };
    public:
        SpinLexer();
        virtual const char *language() const;
        virtual QString description(int style) const;
        virtual void styleText(int start, int end);
        virtual QColor defaultColor(int style) const;
        virtual QFont defaultFont() const;
        virtual QFont defaultFont(int style) const;
        virtual int indentationGuideView() const;
    private:
        QFont fnt;
        QString NL;
        int processComment(QString const text, QString const end, int pos, QVector<SpinStyleRef> &styles, int style);
        int processKeyword(QString kw, int pos, QVector<SpinStyleRef> &styles, int style);
        int processString(QString text, int pos, QVector<SpinStyleRef> &styles);
        QStringList reservedWords;
        QStringList conditionals;
        QStringList types;
        QVector<SpinCommentBlock> comments;
        void invalidateComments(int pos);
        int isComment(int pos);
        QTextCodec *codec;
    };
}

#endif // SPINLEXER_H
