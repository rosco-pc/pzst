#include "groupactiondialog.h"
#include "ui_groupactiondialog.h"
#include <QStandardItemModel>
#include <QPushButton>
#include <QCoreApplication>

GroupActionDialog::GroupActionDialog(Action action, QWidget *parent):
    QDialog(parent),
    ui(new Ui::GroupActionDialog)
{
    ui->setupUi(this);
    model = new QStandardItemModel(ui->listView);
    ui->listView->setModel(model);

    if (action != Reload) {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Discard | QDialogButtonBox::Cancel | QDialogButtonBox::SaveAll);
        QPushButton *button = ui->buttonBox->button(QDialogButtonBox::SaveAll);
        button->setIcon(button->style()->standardIcon(QStyle::StandardPixmap(QStyle::SP_DialogSaveButton)));
        button->setText(tr("Save selected"));
        button->setDefault(true);
        button->setFocus();
        QString text;
        if (action == Close) text= tr("Close without Saving");
        else text = tr("Continue without saving");
        ui->buttonBox->button(QDialogButtonBox::Discard)->setText(text);
        setWindowTitle(tr("Unsaved files"));
    } else {
        ui->label->setText(tr("The following files have changed outside PZST.\nDo you want to reload them?\nReloaded files will loose undo history"));
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Yes| QDialogButtonBox::No);
        ui->buttonBox->button(QDialogButtonBox::No)->setText("Do not reload");;
        ui->buttonBox->button(QDialogButtonBox::Yes)->setText("Reload selected");;
        setWindowTitle(tr("Modified files"));
    }

}

GroupActionDialog::~GroupActionDialog()
{
    delete ui;
}

void GroupActionDialog::changeEvent(QEvent *e)
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

void GroupActionDialog::addFile(QString fName)
{
    QStandardItem *item = new QStandardItem(fName);
    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    item->setData(QVariant(Qt::Checked), Qt::CheckStateRole);
    model->appendRow(item);
}

bool GroupActionDialog::run(QStringList &files)
{
    files.clear();
    setWindowModality(Qt::ApplicationModal);
    int result = exec();
    if (result == 1) {
        for (int i = 0; i < model->rowCount(); i++) {
            QModelIndex index = model->index(i, 0);
            if (model->data(index, Qt::CheckStateRole).toInt() == Qt::Checked) {
                files << model->data(index, Qt::DisplayRole).toString();
            }
        }
    }
    return result;
}


void GroupActionDialog::finished(QAbstractButton *b)
{
    int result;
    if (b == ui->buttonBox->button(QDialogButtonBox::Discard)) result = 2;
    else if (b == ui->buttonBox->button(QDialogButtonBox::SaveAll)) result = 1;
    else if (b == ui->buttonBox->button(QDialogButtonBox::Yes)) result = 1;
    else result = 0;
    done(result);
}
