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
    setEolFill(true, -1);
    setEolFill(false, 0);
}

const char *SpinLexer::language() const
{
    return "SPIN";
}

QString SpinLexer::description(int style) const
{
    switch ((style - 40) % 10) {
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
            if (style < 32 || style > 39) {
                if (style > 39) style -= 8;
                if (!zebraOn) {
                    style = style % 10 + 128;
                }
                if (style > 31) style += 8;
            }
            setStyling(len, style);
        }
        pos += chunk->len;
    }
}

QColor SpinLexer::defaultPaper(int style) const
{
    if (!style) QsciLexer::defaultPaper();
    if (style == 33) return QColor(240, 240, 240);
    if (style > 31 && style < 40) return QsciLexer::defaultPaper();
    if (style > 39) style -= 8;
    if (style > 127) return QsciLexer::defaultPaper();
    QColor paper;
    switch ((style / 10) % 6) {
    case SpinCodeLexer::CON:
        paper = QColor(0xfd, 0xf3, 0xa8); break;
    case SpinCodeLexer::OBJ:
        paper = QColor(0xff, 0xbf, 0xbf); break;
    case SpinCodeLexer::VAR:
        paper = QColor(0xff, 0xdf, 0xbf); break;
    case SpinCodeLexer::PUB:
        paper = QColor(0xbf, 0xdf, 0xff); break;
    case SpinCodeLexer::PRI:
        paper = QColor(0xbf, 0xf8, 0xff); break;
    case SpinCodeLexer::DAT:
        paper = QColor(0xbf, 0xff, 0xc8); break;
    default:
        return QsciLexer::defaultPaper();
    }

    if (style > 59) return paper.darker(105);
    return paper;

    /*
      CON: fdf3a8, efe9b4
      OBJ: ffbfbf, ffbfbf
      VAR: ffdfbf, efd1b3
      PUB: bfdfff, b3d1ef
      PRI: bff8ff, b3e9ef
      DAT: bfffc8, b3efbb
      */
}

QColor SpinLexer::defaultColor(int style) const
{
    if (style > 31 && style < 40) return QsciLexerCustom::defaultColor();
    if (style > 39) style -= 8;
    if (style > 127) style -= 128;

    switch (style  % 10) {
    case SpinCodeLexer::CG_CONDITION:
        return QColor(255, 0, 0, 0);
        break;
    case SpinCodeLexer::CG_COMMENT:
        return QColor(128, 128, 128, 0);
        break;
    case SpinCodeLexer::CG_RESERVED:
        return QColor(0, 128, 0, 0);
        break;
    case SpinCodeLexer::CG_IDENTIFIER:
        return QColor(0, 0, 255, 0);
        break;
    case SpinCodeLexer::CG_NUMBER:
        return QColor(255, 0, 255);
        break;
    case SpinCodeLexer::CG_TYPE:
        return QColor(128, 128, 0);
        break;
    case SpinCodeLexer::CG_STRING:
        return QColor(128, 64, 0);
        break;
    case SpinCodeLexer::CG_PREPRO:
        return QColor(128, 0, 128);
        break;
    case SpinCodeLexer::CG_OTHER:
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

int SpinLexer::styleBitsNeeded() const
{
    return 8;
}
