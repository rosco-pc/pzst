#include "termialwindow.h"
#include "terminalwidget.h"

using namespace PZST;

TermialWindow::TermialWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowTitle(tr("PZST Terminal"));
    setCentralWidget(new TerminalWidget(this));
}

void TermialWindow::setVisible(bool visible)
{
    QMainWindow::setVisible(visible);
    emit visibilityChanged(visible);
}
