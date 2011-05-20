#ifndef SHORTCUTEDIT_H
#define SHORTCUTEDIT_H

#include <QLineEdit>

namespace PZST {

class ShortcutEdit : public QLineEdit
{
Q_OBJECT
public:
    explicit ShortcutEdit(QWidget *parent = 0);
    void clear();

signals:

public slots:
protected:
    virtual void keyPressEvent(QKeyEvent *);
private:
    int keys[4];
};

} // namespace PZST

#endif // SHORTCUTEDIT_H
