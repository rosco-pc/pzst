#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include "searchengine.h"

class Ui_FindDialog;
namespace PZST {

    class FindDialog : public QDialog {
        Q_OBJECT
    public:
        FindDialog(QWidget *parent = 0);
        ~FindDialog();

    signals:
        void searchRequested(QString text, SearchEngine::SearchOptions options);


    protected:
        void changeEvent(QEvent *e);
        void showEvent(QShowEvent *);

    private:
        Ui_FindDialog *ui;

    private slots:

    private slots:
        void on_findButton_clicked();
        void on_findAll_clicked();
        void on_closeButton_clicked();
    };
}
#endif // FINDDIALOG_H
