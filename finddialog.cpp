#include "finddialog.h"
#include "ui_finddialog.h"

using namespace PZST;

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);
    connect(ui->allFiles, SIGNAL(clicked(bool)), ui->replaceButton, SLOT(setDisabled(bool)));
    connect(ui->allFiles, SIGNAL(clicked(bool)), ui->replaceAllButton, SLOT(setDisabled(bool)));
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::changeEvent(QEvent *e)
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

void FindDialog::showEvent(QShowEvent *e)
{
    ui->searchFor->setFocus();
    QDialog::showEvent(e);
}

void FindDialog::on_closeButton_clicked()
{
    reject();
}

void FindDialog::on_findButton_clicked()
{
    if (ui->searchFor->text().isEmpty()) return;
    SearchEngine::SearchOptions options = 0;
    if (ui->re->isChecked()) options |= SearchEngine::RegExp;
    if (ui->cs->isChecked()) options |= SearchEngine::CaseSensitive;
    if (ui->wo->isChecked()) options |= SearchEngine::WholeWord;
    if (ui->wrap->isChecked()) options |= SearchEngine::Wrap;
    if (ui->backward->isChecked()) options |= SearchEngine::Backward;
    if (ui->allFiles->isChecked()) {
        options |= SearchEngine::AllEntries;
        options |= SearchEngine::AllFiles;
    }
    if (options & SearchEngine::AllFiles) accept();
    emit searchRequested(ui->searchFor->text(), options);
}

void FindDialog::on_findAll_clicked()
{
    if (ui->searchFor->text().isEmpty()) return;
    SearchEngine::SearchOptions options = 0;
    if (ui->re->isChecked()) options |= SearchEngine::RegExp;
    if (ui->cs->isChecked()) options |= SearchEngine::CaseSensitive;
    if (ui->wo->isChecked()) options |= SearchEngine::WholeWord;
    if (ui->wrap->isChecked()) options |= SearchEngine::Wrap;
    if (ui->backward->isChecked()) options |= SearchEngine::Backward;
    if (ui->allFiles->isChecked()) options |= SearchEngine::AllFiles;
    options |= SearchEngine::AllEntries;
    if (options & SearchEngine::AllFiles) accept();
    emit searchRequested(ui->searchFor->text(), options);
}
