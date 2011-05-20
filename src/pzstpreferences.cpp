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
    orig = value.trimmed().split("\n");
    foreach (QString s, orig) {
        portable << QKeySequence::fromString(s, fmt).toString(QKeySequence::PortableText);
    }
    value = portable.join("\n");

    if (s.value(name).toString() != value) {
        s.setValue(name, value);
        signaller().emitShortcutChanged(name, value);
    }
    s.endGroup();
}
