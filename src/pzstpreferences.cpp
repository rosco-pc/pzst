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
    return instance;
}

void Preferences::connectInstance(const char *sig, const QObject *obj, const char *slot)
{
    connect(&instance(), sig, obj, slot);
}
