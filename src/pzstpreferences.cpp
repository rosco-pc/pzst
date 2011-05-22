#include "pzstpreferences.h"
#include "shortcuts.h"

using namespace PZST;

Preferences::Preferences()
{
}

Preferences::~Preferences()
{
}

QSettings &Preferences::instance()
{
    static QSettings instance;
    return instance;
}

Preferences &Preferences::signaller()
{
    static Preferences instance;
    return instance;
}

void Preferences::connectInstance(const char *sig, const QObject *obj, const char *slot)
{
    connect(&signaller(), sig, obj, slot);
}

void Preferences::emitValueChanged(QString group, QString name, QVariant value)
{
    emit valueChanged(group, name, value);
}

void Preferences::emitShortcutChanged(QString name, QString value)
{
    emit shortcutChanged(name, value);
}

void Preferences::emitShortcutChanged(int cmd, int k1, int k2)
{
    emit shortcutChanged(cmd, k1, k2);
}

QList<QKeySequence> Preferences::getShortcuts(QString name)
{
    QSettings &s = instance();
    s.beginGroup("Shortcuts");
    QString str = instance().value(name, QString()).toString();
    if (str.isNull()) {
        s.endGroup();
        return Shortcuts::defaultSequence(name);
    }
    QList<QKeySequence> ret;
    foreach (QString shortcut, str.split("\n")) {
        ret << QKeySequence(shortcut);
    }
    s.endGroup();
    return ret;
}

void Preferences::setShortcut(QString name, QString value, QKeySequence::SequenceFormat fmt)
{
    QSettings &s = instance();
    s.beginGroup("Shortcuts");
    QStringList orig, portable;
    QList<QKeySequence> seqs;
    orig = value.trimmed().split("\n");
    foreach (QString s, orig) {
        QKeySequence seq = QKeySequence::fromString(s, fmt);
        seqs << seq;
        portable << seq.toString(QKeySequence::PortableText);
    }
    value = portable.join("\n");

    if (s.value(name).toString() != value) {
        s.setValue(name, value);
        int cmd;
        if ((cmd = Shortcuts::qsciCommand(name))) {
            int k1 = 0, k2 = 0;
            if (seqs.size() > 0) k1 = seqs[0][0];
            if (seqs.size() > 1) k2 = seqs[0][1];
            signaller().emitShortcutChanged(cmd, k1, k2);
        } else {
            signaller().emitShortcutChanged(name, value);
        }
    }
    s.endGroup();
}
