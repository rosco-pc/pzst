#include "terminalwidget.h"
#include "ui_terminalwidget.h"

using namespace PZST;

TerminalWidget::TerminalWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TerminalWidget)
{
    ui->setupUi(this);
}

TerminalWidget::~TerminalWidget()
{
    delete ui;
}

void TerminalWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
