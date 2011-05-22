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
}

const char *SpinLexer::language() const
{
    return "SPIN";
}

QString SpinLexer::description(int style) const
{
    switch ((SpinCodeLexer::Retval)style) {
    case  SpinCodeLexer::COMMENT:
        return "Comment";
        break;
    case  SpinCodeLexer::PUB:
    case  SpinCodeLexer::PRI:
    case  SpinCodeLexer::DAT:
    case  SpinCodeLexer::OBJ:
    case  SpinCodeLexer::CON:
    case  SpinCodeLexer::VAR:
    case  SpinCodeLexer::RESERVED:
    case  SpinCodeLexer::FILE:
        return "Reserved";
        break;
    case  SpinCodeLexer::CONDITION:
        return "Condition";
        break;
    case  SpinCodeLexer::IDENTIFIER:
        return "Identifier";
        break;
    case  SpinCodeLexer::NUMBER:
        return "Number";
        break;
    case  SpinCodeLexer::TYPE:
        return "Type";
        break;
    case  SpinCodeLexer::STRING:
        return "String";
        break;
    case  SpinCodeLexer::PREPRO:
        return "Preprocessor";
        break;
    case  SpinCodeLexer::EOI:
    case  SpinCodeLexer::WHITESPACE:
    case  SpinCodeLexer::NL:
    case  SpinCodeLexer::CHAR:
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
    if (!parser->isValid()) parser->parseCode(e->text());
    const SpinHighlightList &highlights = parser->getHighlighting();
    startStyling(start);
    int n = highlights.size();
    for (int i=0; i < n && pos < end; i++) {
        const SpinHighlightInfo* chunk = highlights[i];
        int len = chunk->len;
        if (pos + len > start) {
            if (start > pos) len -= (start - pos);
            if (pos + len > end) len = end - pos;
            setStyling(len, (int)chunk->style);
        }
        pos += chunk->len;
    }
}

QColor SpinLexer::defaultColor(int style) const
{
    switch ((SpinCodeLexer::Retval)style) {
    case SpinCodeLexer::CONDITION:
        return QColor(255, 0, 0, 0);
        break;
    case SpinCodeLexer::COMMENT:
        return QColor(128, 128, 128, 0);
        break;
    case SpinCodeLexer::RESERVED:
    case SpinCodeLexer::PUB:
    case SpinCodeLexer::PRI:
    case SpinCodeLexer::DAT:
    case SpinCodeLexer::CON:
    case SpinCodeLexer::VAR:
    case SpinCodeLexer::OBJ:
    case SpinCodeLexer::FILE:
        return QColor(0, 128, 0, 0);
        break;
    case SpinCodeLexer::IDENTIFIER:
        return QColor(0, 0, 255, 0);
        break;
    case SpinCodeLexer::NUMBER:
        return QColor(255, 0, 255);
        break;
    case SpinCodeLexer::TYPE:
        return QColor(128, 128, 0);
        break;
    case SpinCodeLexer::STRING:
        return QColor(128, 64, 0);
        break;
    case SpinCodeLexer::PREPRO:
        return QColor(128, 0, 128);
        break;
    case SpinCodeLexer::CHAR:
    case SpinCodeLexer::EOI:
    case SpinCodeLexer::WHITESPACE:
    case SpinCodeLexer::NL:
        return QColor(0, 0, 0);
    }
    return QColor(0, 0, 0, 0);
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
