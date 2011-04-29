#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>
namespace PZST {

    namespace Ui {
        class TerminalWidget;
    }

    class TerminalWidget : public QWidget {
        Q_OBJECT
    public:
        TerminalWidget(QWidget *parent = 0);
        ~TerminalWidget();

    protected:
        void changeEvent(QEvent *e);

    private:
        Ui::TerminalWidget *ui;
    };
}

#endif // TERMINALWIDGET_H
