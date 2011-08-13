#ifndef SPINLEXER_H
#define SPINLEXER_H

#include <Qsci/qscilexercustom.h>
#include <QVector>
#include <QStringList>
#include <QTextCodec>
#include "spincodeparser.h"

namespace PZST {
    class SpinLexer : public QsciLexerCustom
    {
        Q_OBJECT
    public:
        SpinLexer();
        virtual const char *language() const;
        virtual QString description(int style) const;
        virtual int styleBitsNeeded() const;
        virtual void styleText(int start, int end);
        virtual QFont defaultFont() const;
        virtual QFont defaultFont(int style) const;
        virtual int indentationGuideView() const;
        virtual QStringList autoCompletionWordSeparators() const;
        virtual bool caseSensitive() const {return false;};
        void setZebra(bool value) {zebraOn = value;};
    private:
        bool zebraOn;
    };
}

#endif // SPINLEXER_H
