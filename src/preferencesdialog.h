#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

namespace PZST {

    namespace Ui {
        class PreferencesDialog;
    }

    class PreferencesDialog : public QDialog {
        Q_OBJECT
    public:
        PreferencesDialog(QWidget *parent = 0);
        ~PreferencesDialog();

    protected:
        void changeEvent(QEvent *e);

    private:
        Ui::PreferencesDialog *ui;

        void setShortcut(int , QString);

    private slots:
        void on_first_textChanged(QString );
        void on_second_textChanged(QString );
        void on_third_textChanged(QString );
        void on_revert_clicked();
        void on_clear_clicked();
        void on_shortcuts_itemSelectionChanged();
        void on_pageSelector_itemSelectionChanged();
        void on_moveDown_clicked();
        void on_moveUp_clicked();
        void on_searchPaths_currentRowChanged(int currentRow);
        void on_pathRemove_clicked();
        void on_pathAdd_clicked();
        void on_buttonBox_accepted();
        void on_fontName_currentIndexChanged(QString );
        void on_fontSize_valueChanged(int );
    signals:
        void shortcutsChanged();
    };
}

#endif // PREFERENCESDIALOG_H
