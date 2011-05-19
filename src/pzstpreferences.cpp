#include "pzstpreferences.h"

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
    qDebug("%s %x", instance.allKeys().join("|").toUtf8().data(), &instance);
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
