#include "shortcutedit.h"
#include <QKeyEvent>
#include <QKeySequence>

namespace PZST {

ShortcutEdit::ShortcutEdit(QWidget *parent) :
    QLineEdit(parent)
{
}

void ShortcutEdit::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    if (key <0 || key == Qt::Key_Control || key == Qt::Key_Meta || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_AltGr) {
        e->accept();
        return;
    }

    QString current = text();
    if (current.count(", ") < 3) {
        Qt::KeyboardModifiers mods = e->modifiers();
        mods &= ~Qt::KeypadModifier;
        QKeySequence seq(key | mods);
        if (!current.isEmpty()) current.append(", ");
        current.append(seq.toString());
        setText(current);
    }
}

void ShortcutEdit::clear()
{
    keys[0] = keys[1] = keys[2] = keys[3] = 0;
    setText("");
}

} // namespace PZST
