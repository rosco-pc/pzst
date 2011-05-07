#include "spinlexer.h"
#include "spineditor.h"
#include "spinsourcefactory.h"

#include <QTextStream>
#include <QMessageBox>
#include <QApplication>
#include <QFile>
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
    //setFolding(QsciScintilla::BoxedTreeFoldStyle, 2);
    if (pref.getCurLineMarker()) {
        connect(this, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(cursorPositionChanged(int,int)));
        int l, c;
        getCursorPosition(&l, &c);
        cursorPositionChanged(l, c);
    } else {
        markerDeleteAll(0);
        disconnect(this, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(cursorPositionChanged(int,int)));
    }
    setAutoCompletionSource(AcsAPIs);
    setAutoCompletionShowSingle(true);
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

    enum {
        Either,
        Separator,
        Word
    };

    QStringList words;
    int good_pos = pos, expecting = Either;

    last_word_start = -1;

    while (pos > 0)
    {
        QString sep;
        if (!(sep = getSeparator(pos)).isNull())
        {
            if (expecting != Word)
                words.prepend(sep);
            else break;

            good_pos = pos;
            expecting = Word;
        }
        else
        {
            QString word = getWord(pos);

            if (word.isEmpty() || expecting == Separator)
                break;

            words.prepend(word);

            good_pos = pos;
            expecting = Separator;

            // Return the position of the start of the last word if required.
            if (last_word_start < 0)
                last_word_start = pos;
        }

        // Strip any preceding spaces (mainly around operators).
        char ch;

        while ((ch = getCharacter(pos)) != '\0')
        {
            // This is the same definition of space that Scintilla uses.
            if (ch != ' ' && (ch < 0x09 || ch > 0x0d))
            {
                ++pos;
                break;
            }
        }
    }

    context_start = good_pos;

    if (!words.isEmpty()) {
        if (words.last() == "." || words.last() == "#") words << "";
    }
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

QString SpinEditor::getSeparator(int &pos) const
{
    int opos = pos;

    QStringList wseps = lexer()->autoCompletionWordSeparators();
    // Go through each separator.
    for (int i = 0; i < wseps.count(); ++i)
    {
        const QString &ws = wseps[i];

        // Work backwards.
        uint l;

        for (l = ws.length(); l; --l)
        {
            char ch = getCharacter(pos);

            if (ch == '\0' || ws.at(l - 1) != ch)
                break;
        }

        if (!l)
            return ws;

        // Reset for the next separator.
        pos = opos;
    }

    return QString();
}

QString SpinEditor::getWord(int &pos) const
{
    QString word;
    bool numeric = true;
    char ch;

    while ((ch = getCharacter(pos)) != '\0')
    {
        if (!isWordCharacter(ch))
        {
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
