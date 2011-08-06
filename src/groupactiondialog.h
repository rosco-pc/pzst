#ifndef GROUPACTIONDIALOG_H
#define GROUPACTIONDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QAbstractButton>

namespace Ui {
    class GroupActionDialog;
}

class GroupActionDialog : public QDialog {
    Q_OBJECT
public:
    enum Action {
        Other,
        Close,
        Reload,
    };

    explicit GroupActionDialog(Action action, QWidget *parent = 0);
    ~GroupActionDialog();

    void addFile(QString);
    bool run(QStringList &files);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::GroupActionDialog *ui;
    QStandardItemModel *model;
private slots:
    void finished(QAbstractButton*);
};

#endif // GROUPACTIONDIALOG_H
