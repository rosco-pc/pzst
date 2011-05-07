#include "finddialog.h"
#include "ui_finddialog.h"
#include "searchengine.h"

using namespace PZST;

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog),
    currentFile(0),
    openFiles(0)
{
    ui->setupUi(this);
    connect(ui->openFiles, SIGNAL(clicked(bool)), ui->replaceButton, SLOT(setDisabled(bool)));
    connect(ui->openFiles, SIGNAL(clicked(bool)), ui->replaceAllButton, SLOT(setDisabled(bool)));
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
    searchClicked(false);
}

void FindDialog::on_findAll_clicked()
{
    searchClicked(true);
    close();
}

void FindDialog::searchClicked(bool all, bool repeat, bool allowReplace)
{
    if (ui->searchFor->text().isEmpty()) return;
    SearchRequest req(ui->searchFor->text());
    QList<SearchScope*> scopes;
    if ((repeat || ui->currentFile->isChecked()) && currentFile) scopes.append(currentFile);
    if (!repeat && !allowReplace && ui->openFiles->isChecked() && openFiles) {
        scopes.append(openFiles);
        all = true;
    }
    if (ui->cs->isChecked()) req.addOptions(SearchRequest::CaseSensitive);
    if (ui->wo->isChecked()) req.addOptions(SearchRequest::WholeWords);
    if (ui->re->isChecked()) req.addOptions(SearchRequest::RegExp);
    if (ui->backward->isChecked()) req.addOptions(SearchRequest::Backwards);
    if (ui->wrapSearch->isChecked()) req.addOptions(SearchRequest::Wrap);
    if (all) req.addOptions(SearchRequest::All);
    if (allowReplace) {
        req.setReplacement(ui->replaceWith->text());
        req.addOptions(SearchRequest::Replace);
    }
    req.setScopes(scopes);
    SearchEngine::search(req);
}

void FindDialog::findNext()
{
    searchClicked(false, true);
}

void FindDialog::on_replaceButton_clicked()
{
    searchClicked(false, false, true);
}

void FindDialog::on_replaceAllButton_clicked()
{
    searchClicked(true, false, true);
    close();
}

void FindDialog::search(SearchRequest & request)
{
    ui->searchFor->setText(request.getQuery());
    ui->replaceWith->setText(request.getReplacement());
    ui->backward->setChecked(request.getOptions() & SearchRequest::Backwards);
    ui->cs->setChecked(request.getOptions() & SearchRequest::CaseSensitive);
    ui->re->setChecked(request.getOptions() & SearchRequest::RegExp);
    ui->wrapSearch->setChecked(request.getOptions() & SearchRequest::Wrap);
    ui->wo->setChecked(request.getOptions() & SearchRequest::WholeWords);
    searchClicked(request.getOptions() & SearchRequest::All, false, request.getOptions() & SearchRequest::Replace);
    ui->wo->setChecked(request.getOptions() & SearchRequest::WholeWords);
    ui->currentFile->setChecked(true);
    ui->openFiles->setChecked(false);
}
