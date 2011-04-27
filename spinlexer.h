#ifndef SPINLEXER_H
#define SPINLEXER_H

#include <Qsci/qscilexercustom.h>
#include <QVector>
#include <QStringList>
#include <QTextCodec>

namespace PZST {
    class SpinLexer : public QsciLexerCustom
    {
    public:
        SpinLexer();
        virtual const char *language() const;
        virtual QString description(int style) const;
        virtual void styleText(int start, int end);
        virtual QColor defaultColor(int style) const;
        virtual QFont defaultFont() const;
        virtual QFont defaultFont(int style) const;
        virtual int indentationGuideView() const;
        virtual QStringList autoCompletionWordSeparators() const;
        virtual bool caseSensitive() const {return false;};
    };
}

#endif // SPINLEXER_H
