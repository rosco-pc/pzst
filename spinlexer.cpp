#include "spinlexer.h"
#include <Qsci/qsciscintilla.h>
#include <QStringList>
#include <QRegExp>
#include <QString>
#include "pzstpreferences.h"

using namespace PZST;

SpinLexer::SpinLexer()
    : QsciLexerCustom(), fnt("Parallax", 13)
{
    NL = "\n";
    reservedWords << "ABS" << "ABSNEG" << "ADD" << "ADDABS" << "ADDS" << "ADDSX"
        << "ADDX" << "AND" << "ANDN" << "BYTEFILL" << "BYTEMOVE"
        << "CALL" << "CHIPVER" << "CLKFREQ" << "_CLKFREQ" << "CLKMODE"
        << "_CLKMODE" << "CLKSET" << "CMP" << "CMPS" << "CMPSUB"
        << "CMPSX" << "CMPX" << "CNT" << "COGID" << "COGINIT"
        << "COGNEW" << "COGSTOP" << "CON" << "CONSTANT" << "CTRA"
        << "CTRB" << "DAT" << "DIRA" << "DIRB" << "DJNZ" << "ENC"
        << "FALSE" << "FILE" << "FIT" << "_FREE" << "FRQA" << "FRQB"
        << "HUBOP" << "INA" << "INB" << "CONST" << "JMP" << "JMPRET"
        << "LOCKCLR" << "LOCKNEW" << "LOCKRET" << "LOCKSET"
        << "LONGFILL" << "LONGMOVE" << "LOOKDOWN" << "LOOKDOWNZ"
        << "LOOKUP" << "LOOKUPZ" << "MAX" << "MAXS" << "MIN" << "MINS"
        << "MOV" << "MOVD" << "MOVI" << "MOVS" << "MUL" << "MULS"
        << "MUXC" << "MUXNC" << "MUXNZ" << "MUXZ" << "NEG" << "NEGC"
        << "NEGNC" << "NEGNZ" << "NEGX" << "NEGZ" << "NOP" << "NOT"
        << "OBJ" << "ONES" << "OR" << "ORG" << "OUTA" << "OUTB"
        << "PAR" << "PHSA" << "PHSB" << "PI" << "PLL16X" << "PLL1X"
        << "PLL2X" << "PLL4X" << "PLL8X" << "POSX" << "PRI" << "PUB"
        << "RCFAST" << "RCL" << "RCR" << "RCSLOW" << "RDBYTE"
        << "RDLONG" << "RDWORD" << "REBOOT" << "RES" << "RESULT"
        << "RET" << "REV" << "ROL" << "ROR" << "SAR" << "SHL" << "SHR"
        << "SPR" << "_STACK" << "STRCOMP" << "STRING" << "STRSIZE"
        << "SUB" << "SUBABS" << "SUBS" << "SUBSX" << "SUBX"
        << "SUMC" << "SUMNC" << "SUMNZ" << "SUMZ" << "TEST" << "TESTN"
        << "TJNZ" << "TJZ" << "TRUE" << "VAR" << "VCFG" << "VSCL"
        << "WAITCNT" << "WAITPEQ" << "WAITPNE" << "WAITVID"
        << "WORDFILL" << "WORDMOVE" << "WRBYTE" << "WRLONG" << "WRWORD"
        << "_XINFREQ" << "XINPUT" << "XOR" << "XTAL1" << "XTAL2"
        << "XTAL3" << "RETURN" << "ABORT"
        ;
    conditionals
        << "IF_ALWAYS" << "IF_NEVER" << "IF_E" << "IF_Z" << "IF_NE"
        << "IF_NZ" << "IF_NC_AND_NZ" << "IF_NZ_AND_NC" << "IF_A"
        << "IF_B" << "IF_C" << "IF_AE" << "IF_NC" << "IF_C_OR_Z"
        << "IF_Z_OR_C" << "IF_BE" << "IF_C_EQ_Z" << "IF_Z_EQ_C"
        << "IF_C_NE_Z" << "IF_Z_NE_C" << "IF_C_AND_Z" << "IF_Z_AND_C"
        << "IF_C_AND_NZ" << "IF_NZ_AND_C" << "IF_NC_AND_Z"
        << "IF_Z_AND_NC" << "" << "IF_C_OR_NZ" << "IF_NZ_OR_C"
        << "IF_NC_OR_Z" << "IF_Z_OR_NC" << "IF_NC_OR_NZ"
        << "IF_NZ_OR_NC" << "" << "WC" << "WZ" << "WR" << "NR" << "IF"
        << "ELSE" << "ELSEIF" << "ELSEIFNOT" << "IFNOT" << "CASE"
        << "OTHER" << "REPEAT" << "FROM" << "TO" << "STEP" << "UNTIL"
        << "WHILE" << "NEXT" << "QUIT"
        ;

    types << "LONG" << "BYTE" << "WORD";

    codec = QTextCodec::codecForName("UTF8");
}

const char *SpinLexer::language() const
{
    return "SPIN";
}

QString SpinLexer::description(int style) const
{
    switch ((SpinLexerStyle)style) {
    case  Comment:
        return "Comment";
        break;
    case  MLComment1:
        return "Comment";
        break;
    case  MLComment2:
        return "Comment";
        break;
    case  Reserved:
        return "Reserved";
        break;
    case  Condition:
        return "Condition";
        break;
    case  Identifier:
        return "Identifier";
        break;
    case  Number:
        return "Number";
        break;
    case  Type:
        return "Type";
        break;
    case  String:
        return "String";
        break;
    }
    return "";
}

void SpinLexer::styleText(int start, int end)
{
    QRegExp idRE ("\\b[A-Z_][A-Z_0-9]*\\b");
    idRE.setCaseSensitivity(Qt::CaseInsensitive);
    idRE.setMinimal(false);

    QRegExp numRE ("#?\\-?([0-9][0-9_]*|%[01][01_]*|%%[0-3][0-3_]*|\\$[0-9A-F][0-9A-F_]*)\\b");
    numRE.setCaseSensitivity(Qt::CaseInsensitive);
    numRE.setMinimal(false);

    QVector<SpinStyleRef> styles;
    QByteArray ba = codec->fromUnicode(editor()->text()).mid(start, end - start);
    QString text = codec->toUnicode(ba);
    int sIdx = 0, eIdx = text.length();
    invalidateComments(start);
    int inComment = isComment(start);
    bool dblComment = false;
    if (inComment >= 0) {
        dblComment = comments[inComment].dbl;
    }
    while (sIdx < eIdx) {
        int commentStart = eIdx;
        int mlCommentStart2 = eIdx;
        int mlCommentStart1 = eIdx;
        int resStart = eIdx;
        int numStart = eIdx;
        int strStart = eIdx;
        int pos;
        if (inComment < 0) {
            commentStart = text.indexOf("'", sIdx);
            if (commentStart < 0) commentStart = eIdx;
            mlCommentStart2 = text.indexOf("{{", sIdx);
            if (mlCommentStart2 < 0) mlCommentStart2 = eIdx;
            mlCommentStart1 = text.indexOf("{", sIdx);
            if (mlCommentStart1 < 0) mlCommentStart1 = eIdx;
            resStart = idRE.indexIn(text, sIdx);
            if (resStart < 0) resStart = eIdx;
            numStart = numRE.indexIn(text, sIdx);
            if (numStart < 0) numStart = eIdx;
            strStart = text.indexOf("\"", sIdx);
            if (strStart < 0) strStart = eIdx;
        }
        pos = qMin(commentStart,qMin(mlCommentStart2,qMin(mlCommentStart1, qMin(resStart, qMin(numStart, strStart)))));
        if (pos == eIdx && inComment < 0) break;
        int chunkLen;
        if (inComment >= 0) {
            pos = -1;
        }
        if (pos == commentStart && inComment < 0) {
            chunkLen = processComment(text, NL, pos, styles, Comment);
        } else if (pos == strStart && inComment < 0) {
            chunkLen = processString(text, pos, styles);
        } else if (pos == mlCommentStart2 || (inComment >= 0 && dblComment)) {
            if (inComment >= 0 ) pos = sIdx;
            if (inComment >= 0) {
                chunkLen = processComment(text, "}}", pos, styles, Comment);
                comments[inComment].len = start + codec->fromUnicode(text.mid(pos, chunkLen)).size() - comments[inComment].start ;
            } else {
                chunkLen = processComment(text, "}}", pos, styles, MLComment2);
            }
        } else if (pos == mlCommentStart1 || (inComment >= 0 && !dblComment)) {
            if (inComment >= 0 ) pos = sIdx;
            if (inComment >= 0) {
                chunkLen = processComment(text, "}", pos, styles, Comment);
                comments[inComment].len = start + codec->fromUnicode(text.mid(pos, chunkLen)).size() - comments[inComment].start;
            } else {
                chunkLen = processComment(text, "}", pos, styles, MLComment1);
            }
        } else if (pos == resStart) {
            QString match = idRE.capturedTexts().at(0).toUpper();
            int style = Identifier;
            if (reservedWords.contains(match)) {
                style = Reserved;
            } else if (conditionals.contains(match)) {
                style = Condition;
            } else if (types.contains(match)) {
                style = Type;
            }
            chunkLen = processKeyword(match, pos, styles, style);
        } else if (pos == numStart) {
            QString match = numRE.capturedTexts().at(0).toUpper();
            chunkLen = processKeyword(match, pos, styles, Number);
        } else {
            break;
        }
        sIdx = pos + chunkLen;
        inComment = -1;
    }
    startStyling(start);
    setStyling(end - start, 0);
    startStyling(start);
    int charIdx = 0, byteIdx = 0;
    for (int i = 0; i < styles.count(); i++) {
        SpinStyleRef s = styles[i];
        int byteStart = codec->fromUnicode(text.mid(charIdx, s.start - charIdx)).length() + byteIdx;
        setStyling(byteStart - byteIdx, 0);
        int nBytes = codec->fromUnicode(text.mid(s.start, s.len)).length();
        setStyling(nBytes, s.style);
        if (s.style == MLComment1 || s.style == MLComment2) {
            comments.append(SpinCommentBlock(byteStart + start, nBytes, s.style == MLComment2));
        }
        byteIdx = byteStart + nBytes;
        charIdx = s.start + s.len;
    }
}

QColor SpinLexer::defaultColor(int style) const
{
    switch ((SpinLexerStyle)style) {
    case Condition:
        return QColor(255, 0, 0, 0);
        break;
    case Comment:
    case MLComment1:
    case MLComment2:
        return QColor(128, 128, 128, 0);
        break;
    case Reserved:
        return QColor(0, 128, 0, 0);
        break;
    case Identifier:
        return QColor(0, 0, 255, 0);
        break;
    case Number:
        return QColor(255, 0, 255);
        break;
    case Type:
        return QColor(128, 128, 0);
        break;
    case String:
        return QColor(128, 64, 0);
        break;
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
    Preferences pref;
    QFont f(pref.getFontName(), pref.getFontSize());
    return f;
}

SpinStyleRef::SpinStyleRef(int start, int len, int style)
{
    this->start = start;
    this->len = len;
    this->style = style;
}

SpinCommentBlock::SpinCommentBlock(int start, int len, bool dbl)
{
    this->start = start;
    this->len = len;
    this->dbl = dbl;
}

int SpinLexer::processComment(QString text, QString end, int pos, QVector<SpinStyleRef> &styles, int style)
{
    int commentEnd = text.indexOf(end, pos);
    if (commentEnd < 0) commentEnd = text.length();
    else commentEnd += end.length();
    styles.append(SpinStyleRef(pos, commentEnd - pos, style));
    return commentEnd - pos;
}

int SpinLexer::processString(QString text, int pos, QVector<SpinStyleRef> &styles)
{
    int end = text.indexOf("\"", pos+1);
    if (end < 0) end = text.length();
    else end++;
    styles.append(SpinStyleRef(pos, end - pos, String));
    return end - pos;
}

int SpinLexer::processKeyword(QString kw, int pos, QVector<SpinStyleRef> &styles, int style)
{
    styles.append(SpinStyleRef(pos, kw.length(), style));
    return kw.length();
}

void SpinLexer::invalidateComments(int pos)
{
    for (int i = 0; i < comments.size(); i++) {
        if (comments[i].start >= pos) {
            comments.resize(qMax(i-1, 0));
            return;
        }
    }
}

int SpinLexer::isComment(int pos)
{
    for (int i = 0; i < comments.size(); i++) {
        if (comments[i].start < pos && comments[i].start + comments[i].len >= pos) {
            return i;
        }
        if (comments[i].start > pos) {
            break;
        }
    }
    return -1;
}

int SpinLexer::indentationGuideView() const
{
    return QsciScintillaBase::SC_IV_LOOKFORWARD;
}
