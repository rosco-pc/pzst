#include "spinlexer.h"
#include "spineditor.h"
#include "spinsourcefactory.h"

#include <QTextStream>
#include <QMessageBox>
#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFileInfo>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMenu>
#include <QAbstractButton>
#include "pzstpreferences.h"
#include "spincodeparser.h"
#include "searchengine.h"
#include "shortcuts.h"

using namespace PZST;

static int convert(int key);

SpinEditor::SpinEditor(QWidget *p)
    :QsciScintilla(p), HasFilename(false)
{
    initialize();
}

SpinEditor::~SpinEditor()
{
    SpinSourceFactory::instance()->removeSource(fileName);
}

void SpinEditor::initialize()
{
    static int c = 1;
    fileName = QString("Untitled%1.spin").arg(c++);
    spinLexer = new SpinLexer();
    setLexer(spinLexer);
    new SpinCompletionSource(lexer(), this);
    setUtf8(true);
    setAutoIndent(true);
    setBackspaceUnindents(true);
    setAutoCompletionReplaceWord(true);
    setTabIndents(true);
    connect(this, SIGNAL(modificationChanged(bool)), this, SLOT(updateModificationStatus(bool)));
    connect(this, SIGNAL(textChanged()), this, SLOT(documentModified()));
    connect(this,SIGNAL(SCN_MODIFIED(int,int,const char*,int,int,int,int,int,int,int)),
            this, SLOT(handlePreModified(int,int,const char*,int,int,int,int,int,int,int)));
    updateCaption();
    setContextMenuPolicy(Qt::CustomContextMenu);
    markerDefine(Background, 0);

    readPreferences();
    registerIcons();
    addSearchable(this);
    setAutoCompletionSource(AcsAPIs);
    setAutoCompletionShowSingle(true);
    setSelectionToEol(true);
}


SpinEditor *SpinEditor::loadFile(QString fName, QWidget *parent)
{
    SpinEditor *e = new SpinEditor(parent);
    if (!e->loadFile(fName)) {
        delete e;
        e = 0;
    }
    return e;
}

bool SpinEditor::loadFile(QString fName)
{
    fName = QDir::toNativeSeparators(fName);
    QFile file(fName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(parentWidget(), tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fName)
                             .arg(file.errorString()));
        return false;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QTextCodec *utf8 = QTextCodec::codecForName("UTF8");
    QTextStream in(&file);
    in.setCodec(utf8);
    setText(in.readAll().replace("\r\n", "\n").replace("\r", "\n"));
    setFileName(fName);
    setModified(false);
    updateModificationStatus(false);
    SpinSourceFactory::instance()->addSource(fileName, text());
    QApplication::restoreOverrideCursor();
    return true;
}

void SpinEditor::setFileName(QString fName)
{
    fileName = fName;
    HasFilename = true;
    updateCaption();
}


void SpinEditor::updateCaption()
{
    QString caption = QFileInfo(fileName).completeBaseName();
    if (isModified()) caption += " *";
    setWindowTitle(caption);
}

void SpinEditor::updateModificationStatus(bool)
{
    updateCaption();
}

bool SpinEditor::save()
{
    return save(QString(""));
}

bool SpinEditor::save(QString fName)
{
    if (!HasFilename || fName.isNull()) {
        QFileDialog dlg(this, tr("Save file"), "", tr("SPIN source (*.spin)"));
        dlg.setAcceptMode(QFileDialog::AcceptSave);
        dlg.setDefaultSuffix("spin");
        if (HasFilename) dlg.selectFile(fileName);
        if (dlg.exec() != QDialog::Accepted) {
            return false;
        }
        fName = dlg.selectedFiles().value(0);
    } else {
        fName = fileName;
    }
    fName = QDir::toNativeSeparators(fName);
    SpinSourceFactory::instance()->removeSource(fileName);
    QFile file(fName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write %1:\n%2.")
                             .arg(fName)
                             .arg(file.errorString()));
        SpinSourceFactory::instance()->addSource(fileName, text());
        return false;
    }
    QTextStream out(&file);
    Preferences pref;
    if (pref.getUTF16()) {
        out.setCodec(QTextCodec::codecForName("UTF16"));
        out.setGenerateByteOrderMark(true);
    } else {
        out.setCodec(QTextCodec::codecForName("UTF8"));
    }
    out << text();
    file.close();
    fileName = fName;
    SpinSourceFactory::instance()->addSource(fileName, text());
    HasFilename = true;
    setModified(false);
    return true;
}


void SpinEditor::readPreferences()
{
    Preferences pref;
    setTabWidth(pref.getTabSize());
    setTabIndents(pref.getTabsToSpaces());
    setTabIndents(pref.getTabsToSpaces());
    setIndentationsUseTabs(!pref.getTabsToSpaces());
    setIndentationGuides(pref.getTabsVisible());
    setZebra(pref.getZebra());
    setLineNumbers(pref.getLineNumbers());
    setCurLineMarker(pref.getCurLineMarker());
    readKeys();

    setHighlightColor(SpinCodeLexer::CG_COMMENT,    pref.getColorComment());
    setHighlightColor(SpinCodeLexer::CG_CONDITION,  pref.getColorCondition());
    setHighlightColor(SpinCodeLexer::CG_IDENTIFIER, pref.getColorIdentifier());
    setHighlightColor(SpinCodeLexer::CG_NUMBER,     pref.getColorNumber());
    setHighlightColor(SpinCodeLexer::CG_OTHER,      pref.getColorOther());
    setHighlightColor(SpinCodeLexer::CG_PREPRO,     pref.getColorPreprocessor());
    setHighlightColor(SpinCodeLexer::CG_RESERVED,   pref.getColorReserved());
    setHighlightColor(SpinCodeLexer::CG_STRING,     pref.getColorString());
    setHighlightColor(SpinCodeLexer::CG_TYPE,       pref.getColorType());

    setHighlightBackground(SpinCodeLexer::PUB, pref.getPaperPub());
    setHighlightBackground(SpinCodeLexer::PRI, pref.getPaperPri());
    setHighlightBackground(SpinCodeLexer::CON, pref.getPaperCon());
    setHighlightBackground(SpinCodeLexer::VAR, pref.getPaperVar());
    setHighlightBackground(SpinCodeLexer::OBJ, pref.getPaperObj());
    setHighlightBackground(SpinCodeLexer::DAT, pref.getPaperDat());

    setCurLineBackground(pref.getPaperCurrentLine());
    setNumbersBackground(pref.getPaperLineNumbers());
    setNumbersForeground(pref.getColorLineNumbers());

    setSelectionBackgroundColor(pref.getPaperSelection());
    setSelectionForegroundColor(pref.getColorSelection());

}


void SpinEditor::readKeys()
{
    Preferences pref;
    SendScintilla(QsciScintillaBase::SCI_CLEARALLCMDKEYS);
    QStringList commands = Shortcuts::allNames();
    foreach (QString name, commands) {
        int msg = Shortcuts::qsciCommand(name);
        if (msg) {
            QList<QKeySequence> list = pref.getShortcuts(name);
            int k1 = 0, k2 = 0, k3 = 0;
            if (list.size() > 0) k1 = list[0][0];
            if (list.size() > 1) k2 = list[1][0];
            if (list.size() > 2) k3 = list[2][0];
            k1 = convert(k1);
            if (k1) {
                SendScintilla(QsciScintillaBase::SCI_CLEARCMDKEY, k1);
                SendScintilla(QsciScintillaBase::SCI_ASSIGNCMDKEY, k1, msg);
            }
            k2 = convert(k2);
            if (k2) {
                SendScintilla(QsciScintillaBase::SCI_CLEARCMDKEY, k2);
                SendScintilla(QsciScintillaBase::SCI_ASSIGNCMDKEY, k2, msg);
            }
            k3 = convert(k3);
            if (k3) {
                SendScintilla(QsciScintillaBase::SCI_CLEARCMDKEY, k3);
                SendScintilla(QsciScintillaBase::SCI_ASSIGNCMDKEY, k3, msg);
            }
        }
        SendScintilla(QsciScintillaBase::SCI_ASSIGNCMDKEY, convert(Qt::Key_Return), QsciScintillaBase::SCI_NEWLINE);
        SendScintilla(QsciScintillaBase::SCI_ASSIGNCMDKEY, convert(Qt::Key_Escape), QsciScintillaBase::SCI_CANCEL);
    }
}

void SpinEditor::documentModified()
{
    QString code = text();
    SpinSourceFactory::instance()->addSource(fileName, code);
}

SpinContextList SpinEditor::getMethodDefs()
{
    return getParser()->getMethods();
}

QString SpinEditor::getWordAtCursor() const
{
    int line, index;
    getCursorPosition(&line, &index);
    int pos = positionFromLineIndex(line, index);

    QString word;
    bool numeric = true;
    char ch;
    pos --;
    do {
        ch = getChar(pos, 1);
    } while (isWordCharacter(ch) && ch);
    if (!ch) pos++;
    while ((ch = getChar(pos, -1)) != '\0')
    {
        if (!isWordCharacter(ch)) {
            ++pos;
            break;
        }
        if (ch < '0' || ch > '9')
            numeric = false;
        word.prepend(ch);
    }

    // We don't auto-complete numbers.
    if (numeric)
        word.truncate(0);
    return word;
}

char SpinEditor::getChar(int &pos, int delta) const
{
    if (pos <= 0)
        return '\0';
    pos += delta;
    char ch = SendScintilla(SCI_GETCHARAT, pos);
    // Don't go past the end of the previous line.
    if (ch == '\n' || ch == '\r') {
        pos -= delta;
        return '\0';
    }
    return ch;
}


void SpinEditor::cursorPositionChanged(int l, int)
{
    markerDeleteAll(0);
    markerAdd(l, 0);
}

void SpinEditor::keyPressEvent(QKeyEvent *e)
{
    if (e->modifiers() == Qt::ShiftModifier && e->key() == Qt::Key_Backtab) {
        QKeyEvent *e1 = new QKeyEvent(e->type(), Qt::Key_Tab, Qt::ShiftModifier, e->text(), e->isAutoRepeat(), e->count());
        QsciScintilla::keyPressEvent(e1);
        if (e1->isAccepted()) {
            delete e1;
            e->accept();
            return;
        }
        delete e1;
    }
    QKeyEvent *e2 = 0;
    if (e->modifiers() & ~(Qt::ShiftModifier | Qt::KeypadModifier)) {
        if ((e->key() > 0x1F && e->key() < 0x80)) {
            e2 = new QKeyEvent(
                e->type(),
                e->key(),
                e->modifiers(),
                "",
                e->isAutoRepeat(),
                e->count()
            );
        }
    }
    QsciScintilla::keyPressEvent(e2 ? e2 : e);
    if (e2) {
        if (e2->isAccepted()) e->accept();
        delete e2;
    }
}

void SpinEditor::registerIcons()
{
    registerImage(1, QPixmap(":/Icons/pub.png"));
    registerImage(2, QPixmap(":/Icons/pri.png"));
    registerImage(3, QPixmap(":/Icons/constant.png"));
    registerImage(4, QPixmap(":/Icons/long.png"));
    registerImage(5, QPixmap(":/Icons/local.png"));
    registerImage(6, QPixmap(":/Icons/label.png"));
    registerImage(7, QPixmap(":/Icons/word.png"));
    registerImage(8, QPixmap(":/Icons/byte.png"));
    registerImage(9, QPixmap(":/Icons/obj.png"));
}

QStringList SpinEditor::apiContext(int pos, int &context_start,
        int &last_word_start)
{
    last_word_start = pos;
    enum {
        Unknown = 0,
        Identifier,
        Dot,
        Hash,
        Bracket,
        ObjIdentifier,
        ObjMaybeHash,
    };
    QStringList words;
    SpinCodeParser* parser = getParser();
    const SpinHighlightList &hl = parser->getHighlighting();
    int n = 0;
    bool found = false;
    int curBlock;
    for (curBlock = 0; curBlock < hl.size(); curBlock++) {
        const SpinHighlightInfo* info = hl[curBlock];
        if (pos <= n + info->len) {
            found = true;
            break;
        }
        n += info->len;
    }
    if (!found) {
        return words;
    }
    n += hl[curBlock]->len;
    int state = Unknown;
    int level;
    QByteArray bytes = text().toUtf8();
    const SpinHighlightInfo* info;
    for (;curBlock >= 0; curBlock--, n-= info->len) {
        info = hl[curBlock];
        if (info->token == SpinCodeLexer::NL) {
            break;
        }
        if (info->token == SpinCodeLexer::COMMENT || info->token == SpinCodeLexer::WHITESPACE) {
            if (state == Unknown) break;
            continue;
        }
        switch (state) {
        case Unknown:
        case Identifier:
            if (state == Unknown && info->token == SpinCodeLexer::IDENTIFIER) {
                int wordStart = n-info->len;
                last_word_start = wordStart;
                int wordLen = info->len;
                if (wordStart + wordLen > pos) wordLen = pos - wordStart;
                words << QString::fromUtf8(bytes.mid(wordStart, wordLen));
                state = Identifier;
            } else if (info->token == SpinCodeLexer::CHAR) {
                QString chr = QString::fromUtf8(bytes.mid(n-info->len, info->len));
                if (chr == ".")  {
                    if (words.isEmpty()) words << "";
                    words.prepend(chr);
                    state = Dot;
                } else if (chr == "#")  {
                    if (words.isEmpty()) words << "";
                    words.prepend(chr);
                    state = Hash;
                } else {
                    curBlock = -1;
                }
            } else {
                curBlock = -1;
            }
            break;
        case Dot:
        case Hash:
            if (info->token == SpinCodeLexer::CHAR) {
                QString chr = QString::fromUtf8(bytes.mid(n-info->len, info->len));
                if (chr == "]") {
                    state = Bracket;
                    level = 1;
                } else {
                    curBlock = -1;
                }
            } else if (info->token == SpinCodeLexer::IDENTIFIER) {
                int wordStart = n-info->len;
                int wordLen = info->len;
                if (wordStart + wordLen > pos) wordLen = pos - wordStart;
                words.prepend(QString::fromUtf8(bytes.mid(wordStart, wordLen)));
                state = ObjMaybeHash;
            }
            break;
        case Bracket:
            if (info->token == SpinCodeLexer::CHAR) {
                QString chr = QString::fromUtf8(bytes.mid(n-info->len, info->len));
                if (chr == "]") {
                    level++;
                }
                if (chr == "[") {
                    level--;
                    if (!level) {
                        state = ObjIdentifier;
                    }
                }
            }
            break;
        case ObjIdentifier:
            if (info->token == SpinCodeLexer::IDENTIFIER) {
                int wordStart = n-info->len;
                int wordLen = info->len;
                if (wordStart + wordLen > pos) wordLen = pos - wordStart;
                words.prepend(QString::fromUtf8(bytes.mid(wordStart, wordLen)));
                state = ObjMaybeHash;
            }
            break;
        case ObjMaybeHash:
            if (info->token == SpinCodeLexer::CHAR) {
                QString chr = QString::fromUtf8(bytes.mid(n-info->len, info->len));
                if (chr == "#") words.prepend(chr);
            }
            curBlock = -1;
            break;
        }
    }
    context_start = n - info->len;
    return words;
}


char SpinEditor::getCharacter(int &pos) const
{
    if (pos <= 0) return '\0';
    char ch = SendScintilla(SCI_GETCHARAT, --pos);
    // Don't go past the end of the previous line.
    if (ch == '\n' || ch == '\r') {
        ++pos;
        return '\0';
    }
    return ch;
}



SpinCodeParser * SpinEditor::getParser()
{
    SpinCodeParser* parser = SpinSourceFactory::instance()->getParser(fileName);
    if (!parser->isValid()) {
        parser->parseCode(text());
        emit methodsListChanged(this, parser->getMethods());
    }
    return parser;
}

QString SpinEditor::searchTargetId() const
{
    return getFileName();
}

QString SpinEditor::searchTargetText() const
{
    return text();
}

QString SpinEditor::searchScopeName() const
{
    return tr("Current file");
}

bool SpinEditor::supportsReplace() const
{
    return true;
}

int SpinEditor::getStartPosition() const
{
    int line, col, pos;
    getCursorPosition(&line, &col);
    pos = positionFromLineIndex(line, col);
    QByteArray bytes = text().toUtf8().mid(0, pos);
    return QString::fromUtf8(bytes.data(), bytes.size()).size();
}

void SpinEditor::replaceInTarget(QString str)
{
    if (hasSelectedText()) {
        replaceSelectedText(str);
    }
}

void SpinEditor::beginUndoActionSlot()
{
    beginUndoAction();
}

void SpinEditor::endUndoActionSlot()
{
    endUndoAction();
}

void SpinEditor::preferencesChanged(QString section, QString name, QVariant value)
{
    if (section == "Editor") {
        Preferences p;
        if (name == "FontName" || name == "FontSize") {
            QFont f(p.getFontName(), p.getFontSize());
            lexer()->setFont(f, -1);
            if (p.getLineNumbers()) setMarginWidth(1, "999999");
        }
        if (name == "LineNumbers") {
            setLineNumbers(value.toBool());
        }
        if (name == "TabSize") {
            setTabWidth(value.toInt());
        }
        if (name == "TabsToSpaces") {
            setTabIndents(value.toBool());
            setIndentationsUseTabs(!value.toBool());
        }
        if (name == "TabsVisible") {
            setIndentationGuides(value.toBool());
        }
        if (name == "CurLineMarker") {
            setCurLineMarker(value.toBool());
        }
        if (name.startsWith("Color")) {
            applyColorChange(name.mid(5, 100), value.value<QColor>());
        }
        if (name.startsWith("Paper")) {
            applyPaperChange(name.mid(5, 100), value.value<QColor>());
        }
        if (name == "Zebra") {
            setZebra(value.toBool());
        }
    }
}

static int convert(int key)
{
    // Convert the modifiers.
    int sci_mod = 0;

    if (key & Qt::SHIFT)
        sci_mod |= QsciScintillaBase::SCMOD_SHIFT;

    if (key & Qt::CTRL)
        sci_mod |= QsciScintillaBase::SCMOD_CTRL;

    if (key & Qt::ALT)
        sci_mod |= QsciScintillaBase::SCMOD_ALT;

    if (key & Qt::META)
        sci_mod |= QsciScintillaBase::SCMOD_SUPER;

    key &= ~Qt::MODIFIER_MASK;

    // Convert the key.
    int sci_key;

    if (key > 0x7f)
        switch (key)
        {
        case Qt::Key_Down:
            sci_key = QsciScintillaBase::SCK_DOWN;
            break;

        case Qt::Key_Up:
            sci_key = QsciScintillaBase::SCK_UP;
            break;

        case Qt::Key_Left:
            sci_key = QsciScintillaBase::SCK_LEFT;
            break;

        case Qt::Key_Right:
            sci_key = QsciScintillaBase::SCK_RIGHT;
            break;

        case Qt::Key_Home:
            sci_key = QsciScintillaBase::SCK_HOME;
            break;

        case Qt::Key_End:
            sci_key = QsciScintillaBase::SCK_END;
            break;

        case Qt::Key_PageUp:
            sci_key = QsciScintillaBase::SCK_PRIOR;
            break;

        case Qt::Key_PageDown:
            sci_key = QsciScintillaBase::SCK_NEXT;
            break;

        case Qt::Key_Delete:
            sci_key = QsciScintillaBase::SCK_DELETE;
            break;

        case Qt::Key_Insert:
            sci_key = QsciScintillaBase::SCK_INSERT;
            break;

        case Qt::Key_Escape:
            sci_key = QsciScintillaBase::SCK_ESCAPE;
            break;

        case Qt::Key_Backspace:
            sci_key = QsciScintillaBase::SCK_BACK;
            break;

        case Qt::Key_Tab:
            sci_key = QsciScintillaBase::SCK_TAB;
            break;

        case Qt::Key_Return:
            sci_key = QsciScintillaBase::SCK_RETURN;
            break;

        default:
            sci_key = 0;
        }
    else
        sci_key = key;

    if (sci_key)
        sci_key |= (sci_mod << 16);

    return sci_key;
}

void SpinEditor::handlePreModified(int pos, int mtype, const char *text, int len, int added, int line, int foldNow, int foldPrev, int token, int annotationLinesAdded)
{
    Q_UNUSED(pos);
    Q_UNUSED(text);
    Q_UNUSED(len);
    Q_UNUSED(added);
    Q_UNUSED(line);
    Q_UNUSED(foldNow);
    Q_UNUSED(foldPrev);
    Q_UNUSED(token);
    Q_UNUSED(annotationLinesAdded);
    if (mtype & (SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE)) {
        SpinSourceFactory::instance()->getParser(fileName)->invalidate();
    }
}

void SpinEditor::setHighlightBackground(int token, const QColor &c)
{
    if (token >= 0 && token <= 5) {
        SpinLexer *lex = qobject_cast<SpinLexer*>(lexer());
        for (int zebra = 0; zebra < 2; zebra++) {
            for (int i = 1; i <= SpinCodeLexer::CG_OTHER; i++) {
                int style = i;
                style += token << 4;
                style += zebra << 7;
                if (style > 31) style += 8;
                lex->setPaper(zebra ? c.darker(105) : c, style);
            }
        }
    }
}

void SpinEditor::setHighlightColor(int colorGroup, const QColor &c)
{
    if (colorGroup >= 1 && colorGroup <= 9) {
        SpinLexer *lex = qobject_cast<SpinLexer*>(lexer());
        for (int zebra = 0; zebra < 2; zebra++) {
            for (int i = 0; i <= 7; i++) {
                int style = colorGroup;
                style += i << 4;
                style += zebra << 7;
                if (style > 31) style += 8;
                lex->setColor(c, style);
            }
        }
    }
}

void SpinEditor::setNumbersBackground(const QColor &c)
{
    SpinLexer *lex = qobject_cast<SpinLexer*>(lexer());
    lex->setPaper(c, 33);
    lex->styleText(0, text().toUtf8().size()-1);
}

void SpinEditor::setNumbersForeground(const QColor &c)
{
    SpinLexer *lex = qobject_cast<SpinLexer*>(lexer());
    lex->setColor(c, 33);
    lex->styleText(0, text().toUtf8().size()-1);
}

void SpinEditor::setZebra(bool value)
{
    SpinLexer *lex = qobject_cast<SpinLexer*>(lexer());
    lex->setZebra(value);
    lex->styleText(0, text().toUtf8().size()-1);
}

#define APPLY_COLOR(c, cg) if(id == #c) setHighlightColor(SpinCodeLexer::cg, color)

void SpinEditor::applyColorChange(QString id, QColor color)
{
    Preferences pref;
    APPLY_COLOR(Condition, CG_CONDITION);
    APPLY_COLOR(Comment, CG_COMMENT);
    APPLY_COLOR(Reserved, CG_RESERVED);
    APPLY_COLOR(Identifier, CG_IDENTIFIER);
    APPLY_COLOR(Number, CG_NUMBER);
    APPLY_COLOR(Type, CG_TYPE);
    APPLY_COLOR(String, CG_STRING);
    APPLY_COLOR(Preprocessor, CG_PREPRO);
    APPLY_COLOR(Other, CG_OTHER);
    if (id == "LineNumbers") setNumbersForeground(color);
    if (id == "Selection") setSelectionForegroundColor(color);
}

void SpinEditor::applyPaperChange(QString id, QColor color)
{
    if (id == "Con") setHighlightBackground(SpinCodeLexer::CON, color);
    else if (id == "Pub") setHighlightBackground(SpinCodeLexer::PUB, color);
    else if (id == "Pri") setHighlightBackground(SpinCodeLexer::PRI, color);
    else if (id == "Var") setHighlightBackground(SpinCodeLexer::VAR, color);
    else if (id == "Dat") setHighlightBackground(SpinCodeLexer::DAT, color);
    else if (id == "Obj") setHighlightBackground(SpinCodeLexer::OBJ, color);
    else if (id == "LineNumbers") setNumbersBackground(color);
    else if (id == "CurrentLine") setCurLineBackground(color);
    else if (id == "Selection") setSelectionBackgroundColor(color);
}

void SpinEditor::setLineNumbers(bool on)
{
    if (on) {
        setMarginType(1, QsciScintilla::NumberMargin);
        setMarginLineNumbers(1, true);
        setMarginWidth(1, "999999");
    } else {
        setMarginWidth(1, 0);
        setMarginWidth(1, "0");
        setMarginLineNumbers(1, false);
    }

}

void SpinEditor::setCurLineMarker(bool on)
{
    if (on) {
        connect(this, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(cursorPositionChanged(int,int)));
        int l, c;
        getCursorPosition(&l, &c);
        cursorPositionChanged(l, c);
    } else {
        markerDeleteAll(0);
        disconnect(this, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(cursorPositionChanged(int,int)));
    }
}

void SpinEditor::setCurLineBackground(const QColor &c)
{
    setMarkerBackgroundColor(c, 0);
}
