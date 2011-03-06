#include "spinlexer.h"
#include "spineditor.h"

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

using namespace PZST;

SpinEditor::SpinEditor(QWidget *p)
    :QsciScintilla(p), Searchable(), HasFilename(false)
{
    initialize();
}


void SpinEditor::initialize()
{
    static int c = 1;
    fileName = QString("Untitled%1.spin").arg(c++);
    setLexer(new SpinLexer());
    setUtf8(true);
    setAutoIndent(true);
    setBackspaceUnindents(true);
    setTabIndents(true);
    readPreferences();
    connect(this, SIGNAL(modificationChanged(bool)), this, SLOT(updateModificationStatus(bool)));
    connect(this, SIGNAL(textChanged()), this, SLOT(documentModified()));
    updateCaption();
    setContextMenuPolicy(Qt::CustomContextMenu);
}


SpinEditor *SpinEditor::loadFile(QString fName, QWidget *parent)
{
    fName = QDir::toNativeSeparators(fName);
    QFile file(fName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
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
    e->setText(in.readAll());
    e->parseMethods();
    e->setFileName(fName);
    e->setModified(false);
    e->updateModificationStatus(false);
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

void SpinEditor::updateModificationStatus(bool m)
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
    fileName = fName;
    HasFilename = true;
    setModified(false);
    return true;
}

void SpinEditor::closeEvent(QCloseEvent *event)
{
    if (maybeSave(this)) {
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
}


void SpinEditor::documentModified()
{
    parser.parse(text());
    emit methodsListChanged(parser.getMethods());
}

void SpinEditor::parseMethods()
{
    parser.parse(text());
}

SpinMethodInfoList SpinEditor::getMethodDefs()
{
    return parser.getMethods();
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

QStringList SpinEditor::getSearchTargets()
{
    QStringList targets;
    targets << fileName;
    return targets;
}

QString SpinEditor::getSearchTargetText(QString &target)
{
    Q_UNUSED(target);
    return text();
}

int SpinEditor::getSearchTargetStartPosition(QString &target, bool backward)
{
    Q_UNUSED(target);
    int l, i;
    getCursorPosition(&l, &i);
    QByteArray bytes = text().toUtf8();
    int pos;
    if (hasSelectedText()) {
        int sL, sI, eL, eI;
        getSelection(&sL, &sI, &eL, &eI);
        if (backward) {
            pos = QString::fromUtf8(bytes.data(), positionFromLineIndex(sL, sI)).size();
        } else {
            pos = QString::fromUtf8(bytes.data(), positionFromLineIndex(eL, eI)).size();
        }
    } else {
        pos = QString::fromUtf8(bytes.data(), positionFromLineIndex(l, i)).size();
    }
    return pos;
}

void SpinEditor::replaceInTarget(QString &target, int start, int len, QString &text)
{
    Q_UNUSED(target);
}

bool SpinEditor::supportsReplace(QString &target)
{
    Q_UNUSED(target);
    return true;
}

