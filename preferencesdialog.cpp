#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include <QSettings>
#include <QFileDialog>
#include <QDir>
#include <QList>
#include <QTextCodec>
#include  "pzstpreferences.h"

#ifdef Q_OS_WIN
#include "windows.h"
#endif

using namespace PZST;

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);


    Preferences pref;
    ui->fontName->setCurrentFont(QFont(pref.getFontName()));
    ui->fontSize->setValue(pref.getFontSize());
    ui->tabSize->setValue(pref.getTabSize());
    ui->tabsToSpaces->setChecked(pref.getTabsToSpaces());
    ui->tabsVisible->setChecked(pref.getTabsVisible());
    ui->language->setCurrentIndex(pref.getLanguage());
    ui->UTF16->setChecked(pref.getUTF16());

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


    QFont f;
    f = ui->fontName->currentFont();
    f.setPointSize(ui->fontSize->value());
    ui->fontSample->setFont(f);

#ifdef Q_OS_WIN
    QString template1 = "COM%1:";
    for (int i = 0; i < 30; i++) {
        QString fName = template1.arg(i);
        HANDLE h = CreateFileA(fName.toAscii().data(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
        bool success = false;
        if (h == INVALID_HANDLE_VALUE)
        {
            DWORD dwError = GetLastError();
            if (dwError == ERROR_ACCESS_DENIED || dwError == ERROR_GEN_FAILURE || dwError == ERROR_SHARING_VIOLATION || dwError == ERROR_SEM_TIMEOUT)
              success = true;
        } else {
            success = true;
            ::CloseHandle(h);
        }
        if (success) ui->portName->addItem(fName);
    }
#else
    QString template1 = "/dev/ttyS%1";
    QString template2 = "/dev/ttyUSB%1";
    for (int i = 0; i < 30; i++) {
        QString fName = template1.arg(i);
        if (QFile::exists(fName)) ui->portName->addItem(fName);
    }
    for (int i = 0; i < 30; i++) {
        QString fName = template2.arg(i);
        if (QFile::exists(fName)) ui->portName->addItem(fName);
    }
#endif
    ui->portName->setEditText(pref.getPortName());


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
        ui->searchPaths->takeItem(ui->searchPaths->currentRow());
    }
}
