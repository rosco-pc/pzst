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
}

void FindDialog::on_findAll_clicked()
{
    if (ui->searchFor->text().isEmpty()) return;
}
