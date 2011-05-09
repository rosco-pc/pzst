#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QList>
#include "searchscope.h"
#include "searchengine.h"

class Ui_FindDialog;
namespace PZST {

    class FindDialog : public QDialog {
        Q_OBJECT
    public:
        FindDialog(QWidget *parent = 0);
        ~FindDialog();

        void setCurrentFile(SearchScope* s) {currentFile = s;};
        void setOpenFiles(SearchScope* s) {openFiles= s;};
        void search(SearchRequest &);

    public slots:
        void on_findButton_clicked();
        void findNext();
        void on_replaceButton_clicked();

    protected:
        void changeEvent(QEvent *e);
        void showEvent(QShowEvent *);

    private:
        Ui_FindDialog *ui;
        SearchScope* currentFile;
        SearchScope* openFiles;
        void searchClicked(bool all, bool repeat = false, bool allowReplace = false);

    private slots:
        void on_findAll_clicked();
        void on_closeButton_clicked();
        void on_replaceAllButton_clicked();
        void on_re_clicked(bool);
    };
}
#endif // FINDDIALOG_H
