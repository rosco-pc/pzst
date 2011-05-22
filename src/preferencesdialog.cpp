#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include <QSettings>
#include <QFileDialog>
#include <QDir>
#include <QList>
#include <QTreeWidgetItem>
#include <QTextCodec>
#include  "pzstpreferences.h"
#include  "eserialport.h"
#include  "shortcuts.h"

using namespace PZST;

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    ui->pages->setCurrentIndex(0);
    ui->pageSelector->setCurrentItem(ui->pageSelector->itemAt(0, 0));
    ui->pageSelector->expandAll();

    Preferences pref;
    ui->fontName->setCurrentFont(QFont(pref.getFontName()));
    ui->fontSize->setValue(pref.getFontSize());
    ui->tabSize->setValue(pref.getTabSize());
    ui->tabsToSpaces->setChecked(pref.getTabsToSpaces());
    ui->tabsVisible->setChecked(pref.getTabsVisible());
    ui->language->setCurrentIndex(pref.getLanguage());
    ui->UTF16->setChecked(pref.getUTF16());
    ui->lineNumbers->setChecked(pref.getLineNumbers());
    ui->curLine->setChecked(pref.getCurLineMarker());

    QStringList paths = pref.getSearchPath();
    for (int i = 0; i < paths.size(); i++) {
        ui->searchPaths->addItem(new QListWidgetItem(paths[i]));
    }

    ui->foldConstants->setChecked(pref.getFoldConstants());
    ui->eliminateUnused->setChecked(pref.getEliminateUnused());
    ui->foldUnary->setChecked(pref.getFoldUnary());
    ui->genericSafe->setChecked(pref.getGenericSafe());
    ui->fastConstants->setChecked(pref.getFastConstants());
    ui->nonParallaxExt->setChecked(pref.getNonParallaxExt());
    ui->genWarnings->setChecked(pref.getWarnings());
    ui->genInfo->setChecked(pref.getInfo());

    QStringList commands = Shortcuts::allNames();
    ui->shortcuts->clear();
    foreach (QString command, commands) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->shortcuts);
        item->setText(0, command);
        item->setText(1, Shortcuts::title(command));
        item->setTextAlignment(0, Qt::AlignTop | Qt::AlignLeft);
        item->setTextAlignment(1, Qt::AlignTop | Qt::AlignLeft);
        item->setTextAlignment(2, Qt::AlignTop | Qt::AlignLeft);
        item->setData(1, Qt::ToolTipRole, Shortcuts::title(command));
        QString shortcuts;
        foreach (QKeySequence seq, pref.getShortcuts(command)) {
            QString shortcut = seq.toString(QKeySequence::NativeText);
            if (!shortcut.isEmpty()) {
                if (!shortcuts.isEmpty()) shortcuts += "\n";
                shortcuts += shortcut;
            }
        }
        item->setText(2, shortcuts);
        item->setText(3, shortcuts);
    }
    ui->shortcuts->setColumnWidth(1, 240);

    QFont f;
    f = ui->fontName->currentFont();
    f.setPointSize(ui->fontSize->value());
    ui->fontSample->setFont(f);

    ui->portName->addItems(ESerialPort::enumeratePorts());
    ui->portName->setEditText(pref.getPortName());
    QList<QByteArray> available = QTextCodec::availableCodecs();
    QStringList codecs;
    QStringList seen;
    for (int i = 0; i < available.size(); i++) {
        QString codecName = QString(available.at(i));
        if (seen.contains(codecName)) continue;

        QTextCodec *codec = QTextCodec::codecForName(available.at(i));
        codecs << codecName;
        for (int j = 0; j < codec->aliases().size(); j++) {
            seen << codec->aliases().at(j);
        }
    }
    codecs.sort();
    ui->encoding->addItems(codecs);
    ui->encoding->setCurrentIndex(codecs.indexOf(pref.getEncoding()));
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void PreferencesDialog::on_fontSize_valueChanged(int s)
{
    QFont f;
    f = ui->fontSample->font();
    f.setPointSize(s);
    ui->fontSample->setFont(f);
}


void PreferencesDialog::on_fontName_currentIndexChanged(QString s)
{
    QFont f;
    f = ui->fontSample->font();
    f.setFamily(s);
    ui->fontSample->setFont(f);
}

void PreferencesDialog::on_buttonBox_accepted()
{
    Preferences pref;
    pref.setFontName(ui->fontName->currentFont().family());
    pref.setFontSize(ui->fontSize->value());
    pref.setTabSize(ui->tabSize->value());
    pref.setTabsToSpaces(ui->tabsToSpaces->isChecked());
    pref.setTabsVisible(ui->tabsVisible->isChecked());
    pref.setLanguage(ui->language->currentIndex());
    pref.setUTF16(ui->UTF16->isChecked());
    pref.setLineNumbers(ui->lineNumbers->isChecked());
    pref.setCurLineMarker(ui->curLine->isChecked());

    QStringList paths;
    for (int i = 0; i < ui->searchPaths->count(); i++) {
        paths << ui->searchPaths->item(i)->text();
    }
    pref.setSearchPath(paths);
    pref.setFoldConstants(ui->foldConstants->isChecked());
    pref.setEliminateUnused(ui->eliminateUnused->isChecked());
    pref.setFoldUnary(ui->foldUnary->isChecked());
    pref.setGenericSafe(ui->genericSafe->isChecked());
    pref.setFastConstants(ui->fastConstants->isChecked());
    pref.setNonParallaxExt(ui->nonParallaxExt->isChecked());
    pref.setWarnings(ui->genWarnings->isChecked());
    pref.setInfo(ui->genInfo->isChecked());

    pref.setPortName(ui->portName->currentText());
    pref.setEncoding(ui->encoding->currentText());


    for (int i =0; i < ui->shortcuts->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = ui->shortcuts->topLevelItem(i);
        QString name = item->text(0);
        QString value = item->text(2);
        pref.setShortcut(name, value);
    }

}

void PreferencesDialog::on_pathAdd_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Add directory"));
    if (!dir.isEmpty()) {
        ui->searchPaths->addItem(new QListWidgetItem(QDir::toNativeSeparators(dir)));
    }
}

void PreferencesDialog::on_pathRemove_clicked()
{
    if (ui->searchPaths->currentRow() >= 0) {
        QListWidgetItem *item= ui->searchPaths->takeItem(ui->searchPaths->currentRow());
        delete item;
    }
}

void PZST::PreferencesDialog::on_searchPaths_currentRowChanged(int currentRow)
{
    if (currentRow < 0) {
        ui->moveUp->setEnabled(false);
        ui->moveUp->setEnabled(false);
    } else {
        ui->moveUp->setEnabled(currentRow > 0);
        ui->moveDown->setEnabled(currentRow < ui->searchPaths->count()-1);
    }
}

void PZST::PreferencesDialog::on_moveUp_clicked()
{
    int idx = ui->searchPaths->currentRow();
    QString text = ui->searchPaths->item(idx)->text();
    QListWidgetItem *item= ui->searchPaths->takeItem(ui->searchPaths->currentRow());
    delete item;
    ui->searchPaths->insertItem(idx - 1, text);
    ui->searchPaths->setCurrentRow(idx - 1);
}

void PZST::PreferencesDialog::on_moveDown_clicked()
{
    int idx = ui->searchPaths->currentRow();
    QString text = ui->searchPaths->item(idx)->text();
    QListWidgetItem *item= ui->searchPaths->takeItem(ui->searchPaths->currentRow());
    delete item;
    ui->searchPaths->insertItem(idx + 1, text);
    ui->searchPaths->setCurrentRow(idx + 1);
}


void PZST::PreferencesDialog::on_pageSelector_itemSelectionChanged()
{
    int n = ui->pageSelector->currentIndex().row();
    if (ui->pageSelector->currentIndex().parent().isValid()) {
        n += ui->pageSelector->currentIndex().parent().row()*100 + 1;
    } else {
        n *= 100;
    }
    int page = 0;
    switch (n) {
    case 0:
    case 1:
        page = 0;
        break;
    case 100:
    case 101:
        page = 1;
        break;
    case 102:
        page = 2;
        break;
    case 200:
    case 201:
        page = 3;
        break;
    case 202:
        page = 4;
        break;
    }
    ui->pages->setCurrentIndex(page);
}



void PZST::PreferencesDialog::on_shortcuts_itemSelectionChanged()
{
    ui->first->setEnabled(true);
    ui->second->setEnabled(true);
    ui->third->setEnabled(true);
    ui->clear->setEnabled(true);
    ui->revert->setEnabled(true);
    QStringList shortcuts = ui->shortcuts->selectedItems()[0]->text(2).split("\n");
    ui->first->setText(shortcuts[0]);
    if (shortcuts.size() > 1) {
        ui->second->setText(shortcuts[1]);
    } else {
        ui->second->setText("");
    }
    if (shortcuts.size() > 2) {
        ui->third->setText(shortcuts[2]);
    } else {
        ui->third->setText("");
    }
}


void PZST::PreferencesDialog::on_clear_clicked()
{
    QStringList shortcuts = ui->shortcuts->selectedItems()[0]->text(3).split("\n");
    if (ui->first->hasFocus()) {
        ui->first->clear();
    } else if (ui->second->hasFocus()) {
        ui->second->clear();
    } else if (ui->third->hasFocus()) {
        ui->third->clear();
    }
}

void PZST::PreferencesDialog::on_revert_clicked()
{
    QStringList shortcuts = ui->shortcuts->selectedItems()[0]->text(3).split("\n");
    if (ui->first->hasFocus()) {
        ui->first->setText(shortcuts[0]);
    } else if (ui->second->hasFocus()) {
        if (shortcuts.size() > 1) ui->second->setText(shortcuts[1]);
        else ui->second->setText("");
    } else if (ui->third->hasFocus()) {
        if (shortcuts.size() > 2) ui->third->setText(shortcuts[2]);
        else ui->third->setText("");
    }
}

void PreferencesDialog::setShortcut(int n, QString s)
{
    QStringList shortcuts = ui->shortcuts->selectedItems()[0]->text(2).split("\n");
    while (shortcuts.size()<= n) shortcuts << "";
    shortcuts[n] = s;
    while (!shortcuts.isEmpty() && shortcuts[shortcuts.size()-1].isEmpty()) shortcuts.removeAt(shortcuts.size()-1);
    ui->shortcuts->selectedItems()[0]->setText(2, shortcuts.join("\n").trimmed());
}

void PZST::PreferencesDialog::on_first_textChanged(QString txt)
{
    setShortcut(0, txt);
}
void PZST::PreferencesDialog::on_second_textChanged(QString txt)
{
    setShortcut(1, txt);
}
void PZST::PreferencesDialog::on_third_textChanged(QString txt)
{
    setShortcut(2, txt);
}
