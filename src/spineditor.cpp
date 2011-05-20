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

using namespace PZST;

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
    setLexer(new SpinLexer());
    new SpinCompletionSource(lexer(), this);
    setUtf8(true);
    setAutoIndent(true);
    setBackspaceUnindents(true);
    setAutoCompletionReplaceWord(true);
    setTabIndents(true);
    connect(this, SIGNAL(modificationChanged(bool)), this, SLOT(updateModificationStatus(bool)));
    connect(this, SIGNAL(textChanged()), this, SLOT(documentModified()));
    updateCaption();
    setContextMenuPolicy(Qt::CustomContextMenu);
    markerDefine(Background, 0);
    setMarkerBackgroundColor(QColor(240,240,240), 0);
    readPreferences();
    registerIcons();
    addSearchable(this);
    setAutoCompletionSource(AcsAPIs);
    setAutoCompletionShowSingle(true);
}


SpinEditor *SpinEditor::loadFile(QString fName, QWidget *parent)
{
    fName = QDir::toNativeSeparators(fName);
    QFile file(fName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(parent, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fName)
                             .arg(file.errorString()));
        return 0;
    }

    SpinEditor *e = new SpinEditor(parent);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QTextCodec *utf8 = QTextCodec::codecForName("UTF8");
    QTextStream in(&file);
    in.setCodec(utf8);
    e->setText(in.readAll().replace("\r\n", "\n").replace("\r", "\n"));
    e->setFileName(fName);
    e->setModified(false);
    e->updateModificationStatus(false);
    SpinSourceFactory::instance()->addSource(e->fileName, e->text());
    QApplication::restoreOverrideCursor();
    return e;
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
    QFile file(fName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write %1:\n%2.")
                             .arg(fName)
                             .arg(file.errorString()));
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
    SpinSourceFactory::instance()->removeSource(fileName);
    fileName = fName;
    SpinSourceFactory::instance()->addSource(fileName, text());
    HasFilename = true;
    setModified(false);
    return true;
}

void SpinEditor::closeEvent(QCloseEvent *event)
{
    if (maybeSave(this)) {
        // FIXME SearchEngine::removeScope(this);
        emit closed(this);
        event->accept();
    }
    else event->ignore();
}

int SpinEditor::maybeSave(QWidget *parent, bool forClose)
{
    if (!isModified()) return true;
    QMessageBox msgBox(parent);
    msgBox.setText(tr("The document has been modified."));
    msgBox.setInformativeText(tr("Do you want to save changes to %1?").arg(fileName));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::SaveAll | QMessageBox::Discard | QMessageBox::Cancel);
    if (!forClose) {
        QAbstractButton *b = msgBox.button(QMessageBox::Discard);
        b->setText(tr("Continue without saving"));
    }
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Save:
         return save() ? 1 : 0;
            break;
        case QMessageBox::SaveAll:
            return save() ? 2 : 0;
            break;
       case QMessageBox::Discard:
           return 1;
           break;
       case QMessageBox::Cancel:
       default:
           return 0;
           break;
     }
    return false;
}


void SpinEditor::readPreferences()
{
    Preferences pref;
    setTabWidth(pref.getTabSize());
    setTabIndents(pref.getTabsToSpaces());
    setTabIndents(pref.getTabsToSpaces());
    setIndentationsUseTabs(!pref.getTabsToSpaces());
    setIndentationGuides(pref.getTabsVisible());
    if (pref.getLineNumbers()) {
        setMarginType(1, QsciScintilla::NumberMargin);
        setMarginWidth(1, "999999");
        setMarginLineNumbers(1, true);
    } else {
        setMarginWidth(1, 0);
        setMarginWidth(1, "0");
        setMarginLineNumbers(1, false);
    }
    if (pref.getCurLineMarker()) {
        connect(this, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(cursorPositionChanged(int,int)));
        int l, c;
        getCursorPosition(&l, &c);
        cursorPositionChanged(l, c);
    } else {
        markerDeleteAll(0);
        disconnect(this, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(cursorPositionChanged(int,int)));
    }
}


void SpinEditor::documentModified()
{
    QString code = text();
    SpinSourceFactory::instance()->addSource(fileName, code);
    emit methodsListChanged(SpinSourceFactory::instance()->getParser(fileName)->getMethods());
}

SpinContextList SpinEditor::getMethodDefs()
{
    return SpinSourceFactory::instance()->getParser(fileName)->getMethods();
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
            e->accept();
            return;
        }
    }
    QsciScintilla::keyPressEvent(e);
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
    Q_UNUSED(last_word_start);
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
    SpinCodeParser* parser = SpinSourceFactory::instance()->getParser(fileName);
    SpinHighlightList hl = parser->getHighlighting();
    int n = 0;
    bool found = false;
    int curBlock;
    for (curBlock = 0; curBlock < hl.size(); curBlock++) {
        SpinHighlightInfo info = hl[curBlock];
        if (pos <= n + info.len) {
            found = true;
            break;
        }
        n += info.len;
    }
    if (!found) {
        return words;
    }
    n += hl[curBlock].len;
    int state = Unknown;
    int level;
    QByteArray bytes = text().toUtf8();
    SpinHighlightInfo info;
    for (;curBlock >= 0; curBlock--, n-= info.len) {
        info = hl[curBlock];
        if (info.style == SpinCodeLexer::NL) {
            break;
        }
        if (info.style == SpinCodeLexer::COMMENT || info.style == SpinCodeLexer::WHITESPACE) {
            if (state == Unknown) break;
            continue;
        }
        switch (state) {
        case Unknown:
        case Identifier:
            if (state == Unknown && info.style == SpinCodeLexer::IDENTIFIER) {
                int wordStart = n-info.len;
                int wordLen = info.len;
                if (wordStart + wordLen > pos) wordLen = pos - wordStart;
                words << QString::fromUtf8(bytes.mid(wordStart, wordLen));
                state = Identifier;
            } else if (info.style == SpinCodeLexer::CHAR) {
                QString chr = QString::fromUtf8(bytes.mid(n-info.len, info.len));
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
                    words.clear();
                }
            } else {
                curBlock = -1;
            }
            break;
        case Dot:
        case Hash:
            if (info.style == SpinCodeLexer::CHAR) {
                QString chr = QString::fromUtf8(bytes.mid(n-info.len, info.len));
                if (chr == "]") {
                    state = Bracket;
                    level = 1;
                } else {
                    curBlock = -1;
                }
            } else if (info.style == SpinCodeLexer::IDENTIFIER) {
                int wordStart = n-info.len;
                int wordLen = info.len;
                if (wordStart + wordLen > pos) wordLen = pos - wordStart;
                words.prepend(QString::fromUtf8(bytes.mid(wordStart, wordLen)));
                state = ObjMaybeHash;
            }
            break;
        case Bracket:
            if (info.style == SpinCodeLexer::CHAR) {
                QString chr = QString::fromUtf8(bytes.mid(n-info.len, info.len));
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
            if (info.style == SpinCodeLexer::IDENTIFIER) {
                int wordStart = n-info.len;
                int wordLen = info.len;
                if (wordStart + wordLen > pos) wordLen = pos - wordStart;
                words.prepend(QString::fromUtf8(bytes.mid(wordStart, wordLen)));
                state = ObjMaybeHash;
            }
            break;
        case ObjMaybeHash:
            if (info.style == SpinCodeLexer::CHAR) {
                QString chr = QString::fromUtf8(bytes.mid(n-info.len, info.len));
                if (chr == "#") words.prepend(chr);
            }
            curBlock = -1;
            break;
        }
    }
    context_start = n - info.len;
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
    return SpinSourceFactory::instance()->getParser(fileName);
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
            qDebug("%s %d", f.family().toUtf8().data(), f.pointSize());
            lexer()->setFont(f, -1);
            if (p.getLineNumbers()) setMarginWidth(1, "999999");
        }
        if (name == "LineNumbers") {
            if (value.toBool()) {
                setMarginType(1, QsciScintilla::NumberMargin);
                setMarginLineNumbers(1, true);
                setMarginWidth(1, "999999");
            } else {
                setMarginWidth(1, 0);
                setMarginWidth(1, "0");
                setMarginLineNumbers(1, false);
            }
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
            if (value.toBool()) {
                connect(this, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(cursorPositionChanged(int,int)));
                int l, c;
                getCursorPosition(&l, &c);
                cursorPositionChanged(l, c);
            } else {
                markerDeleteAll(0);
                disconnect(this, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(cursorPositionChanged(int,int)));
            }
        }
    }
}
