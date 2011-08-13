#include "spinlexer.h"
#include <Qsci/qsciscintilla.h>
#include <QStringList>
#include <QRegExp>
#include <QString>
#include "pzstpreferences.h"
#include "spincodeparser.h"
#include "spineditor.h"

using namespace PZST;



SpinLexer::SpinLexer()
    : QsciLexerCustom()
{
    for (int i = 1; i < 256; i++) {
        setEolFill(true, i);
    }
}

const char *SpinLexer::language() const
{
    return "SPIN";
}

QString SpinLexer::description(int style) const
{
    if (style > 31 && style < 40) return "Internal";
    if (style > 39) style -= 8;

    switch (style  & 15) {
    case SpinCodeLexer::CG_COMMENT:
        return "Comment";
        break;
    case  SpinCodeLexer::CG_RESERVED:
        return "Reserved";
        break;
    case  SpinCodeLexer::CG_CONDITION:
        return "Condition";
        break;
    case  SpinCodeLexer::CG_IDENTIFIER:
        return "Identifier";
        break;
    case  SpinCodeLexer::CG_NUMBER:
        return "Number";
        break;
    case  SpinCodeLexer::CG_TYPE:
        return "Type";
        break;
    case  SpinCodeLexer::CG_STRING:
        return "String";
        break;
    case  SpinCodeLexer::CG_PREPRO:
        return "Preprocessor";
        break;
    case  SpinCodeLexer::CG_OTHER:
    default:
        break;
    }
    return "Other";
}

void SpinLexer::styleText(int start, int end)
{
    SpinEditor *e = qobject_cast<SpinEditor *>(editor());
    if (!e) return;
    int pos = 0;
    SpinCodeParser *parser = e->getParser();
    const SpinHighlightList &highlights = parser->getHighlighting();
    startStyling(start);
    int n = highlights.size();
    for (int i=0; i < n && pos < end; i++) {
        const SpinHighlightInfo* chunk = highlights[i];
        int len = chunk->len;
        if (pos + len > start) {
            if (start > pos) len -= (start - pos);
            if (pos + len > end) len = end - pos;
            int style = chunk->style;
            if (!zebraOn) {
                if (style > 39) style -= 8;
                style &= ~(7 << 4);
                style |= (6 << 4);
                style &= 127;
                if (style > 31) style += 8;
            }
            setStyling(len, style);
        }
        pos += chunk->len;
    }
}


QFont SpinLexer::defaultFont() const
{
    Preferences pref;
    QFont f(pref.getFontName(), pref.getFontSize());
    return f;
}


QFont SpinLexer::defaultFont(int style) const
{
    Q_UNUSED(style);
    Preferences pref;
    QFont f(pref.getFontName(), pref.getFontSize());
    return f;
}

int SpinLexer::indentationGuideView() const
{
    return QsciScintillaBase::SC_IV_LOOKFORWARD;
}

QStringList  SpinLexer::autoCompletionWordSeparators() const
{
    return QStringList() << "." << "#";
}

int SpinLexer::styleBitsNeeded() const
{
    return 8;
}

