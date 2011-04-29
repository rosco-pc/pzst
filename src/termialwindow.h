#ifndef TERMIALWINDOW_H
#define TERMIALWINDOW_H

#include <QMainWindow>

namespace PZST {
    class TermialWindow : public QMainWindow
    {
    Q_OBJECT
    public:
        explicit TermialWindow(QWidget *parent = 0);

    signals:
        void visibilityChanged(bool visible);

    public slots:
    protected:
        virtual void setVisible(bool visible);

    };
}
#endif // TERMIALWINDOW_H
